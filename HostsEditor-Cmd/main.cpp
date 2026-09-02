#include "hosts_io.h"
#include "bin_cache.h"
#include "Logout/Logout.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

static const char* DEFAULT_HOSTS_PATH = "C:\\Windows\\System32\\drivers\\etc\\hosts";

static std::string get_group_name(const std::string& marker)
{
	size_t spos = marker.find('#');
	if (spos == std::string::npos) return "";
	size_t start = marker.find_first_not_of(" \t", spos + 1);
	if (start == std::string::npos) return "";
	size_t end = marker.find(' ', start);
	if (end == std::string::npos) end = marker.length();
	return marker.substr(start, end - start);
}

std::string showhosts(std::vector<hosts_group> hg)
{
	std::string outstr;
	for (hosts_group& c : hg)
	{
		for (hosts_info_group& a : c.get_host_group())
		{
			outstr += get_group_name(a.start_marker) + ":\n";
			for (host_pair& b : a.host_pair)
			{
				outstr += "IP:" + b.ip + "|Host:" + b.host_name + "|Enable:";
				if (b.enable == true)
				{
					outstr += "Yes\n";
				}
				else
				{
					outstr += "No\n";
				}
			}
			outstr += "\n";
		}
	}
	return outstr;
}

static void print_usage()
{
	std::cout
		<< "用法: HostsEditor-Cmd [--file <路径>] <命令> [参数...]\n"
		<< "\n"
		<< "命令:\n"
		<< "  show                   解析并打印 hosts 分组内容 (默认)\n"
		<< "  apply                  序列化并写回 hosts 文件\n"
		<< "  enable <host|group>    启用匹配的主机或整个分组并写回\n"
		<< "  disable <host|group>   禁用匹配的主机或整个分组并写回\n"
		<< "  add <ip> <host> [group]  新增启用条目到指定分组(默认Default)并写回\n"
		<< "  moveto <host> <group>  移动条目到指定分组并写回\n"
		<< "  remove <host>          删除匹配的条目并写回\n"
		<< "  help                   显示本帮助\n"
		<< "\n"
		<< "选项:\n"
		<< "  --file <路径>            指定 hosts 文件路径 (默认: " << DEFAULT_HOSTS_PATH << ")\n"
		<< "  --cache                  命令成功后写入bin缓存 (默认不写)\n";
}

static void set_enable(std::vector<hosts_group>& groups, const std::string& target, bool enable, int& changed)
{
	bool group_matched = false;
	for (auto& g : groups)
	{
		for (auto& ig : g.get_host_group())
		{
			if (get_group_name(ig.start_marker) == target)
			{
				group_matched = true;
				for (auto& p : ig.host_pair)
				{
					if (p.enable != enable)
					{
						p.enable = enable;
						++changed;
					}
				}
			}
		}
	}

	if (group_matched) return;

	for (auto& g : groups)
	{
		for (auto& ig : g.get_host_group())
		{
			for (auto& p : ig.host_pair)
			{
				if (p.host_name == target && p.enable != enable)
				{
					p.enable = enable;
					++changed;
				}
			}
		}
	}
}

static void remove_entry(std::vector<hosts_group>& groups, const std::string& host, int& changed)
{
	for (auto& g : groups)
	{
		for (auto& ig : g.get_host_group())
		{
			auto& pairs = ig.host_pair;
			size_t before = pairs.size();
			pairs.erase(std::remove_if(pairs.begin(), pairs.end(),
				[&host](const host_pair& p) { return p.host_name == host; }), pairs.end());
			changed += (int)(before - pairs.size());
		}
	}
}

static bool add_entry(std::vector<hosts_group>& groups, const std::string& ip, const std::string& host, const std::string& group_name)
{
	for (auto& g : groups)
	{
		for (auto& ig : g.get_host_group())
		{
			if (group_name.empty())
			{
				if (ig.is_default_group)
				{
					ig.host_pair.push_back(host_pair{ true, host, ip });
					return false;
				}
			}
			else if (get_group_name(ig.start_marker) == group_name)
			{
				ig.host_pair.push_back(host_pair{ true, host, ip });
				return false;
			}
		}
	}

	hosts_info_group ig;
	if (group_name.empty())
	{
		ig.is_default_group = true;
		ig.start_marker = "# Default Start";
		ig.end_marker = "# Default End";
	}
	else
	{
		ig.start_marker = "# " + group_name + " Start";
		ig.end_marker = "# " + group_name + " End";
	}
	ig.host_pair.push_back(host_pair{ true, host, ip });

	if (!groups.empty())
	{
		groups[0].add_host_group(ig);
	}
	else
	{
		hosts_group hg;
		hg.add_host_group(ig);
		groups.push_back(hg);
	}
	return true;
}

static bool move_entry(std::vector<hosts_group>& groups, const std::string& host, const std::string& group_name, int& moved)
{
	std::vector<host_pair> to_move;
	for (auto& g : groups)
	{
		for (auto& ig : g.get_host_group())
		{
			auto& pairs = ig.host_pair;
			auto it = pairs.begin();
			while (it != pairs.end())
			{
				if (it->host_name == host)
				{
					to_move.push_back(*it);
					it = pairs.erase(it);
					++moved;
				}
				else
				{
					++it;
				}
			}
		}
	}

	if (moved == 0) return false;

	hosts_info_group* target = nullptr;
	for (auto& g : groups)
	{
		for (auto& ig : g.get_host_group())
		{
			if (get_group_name(ig.start_marker) == group_name)
			{
				target = &ig;
				break;
			}
		}
		if (target) break;
	}

	if (target == nullptr)
	{
		hosts_info_group ig;
		ig.start_marker = "# " + group_name + " Start";
		ig.end_marker = "# " + group_name + " End";
		if (!groups.empty())
		{
			groups[0].add_host_group(ig);
			target = &groups[0].get_host_group().back();
		}
		else
		{
			hosts_group hg;
			hg.add_host_group(ig);
			groups.push_back(hg);
			target = &groups[0].get_host_group().back();
		}
	}

	target->host_pair.insert(target->host_pair.end(), to_move.begin(), to_move.end());
	return true;
}

int main(int argc, char** argv)
{
	LOG_CREATE_MODEL_NAME("main");

	std::string path = DEFAULT_HOSTS_PATH;
	std::string command;
	std::vector<std::string> args;
	bool save_cache = false;

	for (int i = 1; i < argc; ++i)
	{
		std::string a = argv[i];
		if (a == "--file" || a == "-f")
		{
			if (i + 1 >= argc)
			{
				LOG_ERROR("缺少 --file 参数值");
				print_usage();
				return 1;
			}
			path = argv[++i];
		}
		else if (a == "--cache" || a == "-c")
		{
			save_cache = true;
		}
		else if (a == "--help" || a == "-h" || a == "help")
		{
			print_usage();
			return 0;
		}
		else if (!a.empty() && a[0] == '-')
		{
			LOG_ERROR("未知选项: " + a);
			print_usage();
			return 1;
		}
		else if (command.empty())
		{
			command = a;
		}
		else
		{
			args.push_back(a);
		}
	}

	if (command.empty()) command = "show";

	hosts_io hi;
	auto refresh_cache = [&path, &model_name]()
	{
		try
		{
			hosts_io fresh;
			fresh.setpath(path);
			fresh.loadfile();
			fresh.Analyse();
			if (!bin_cache::save(fresh.get_group(), bin_cache::default_path()))
			{
				LOG_WARNING("写入bin缓存失败");
			}
		}
		catch (const std::exception& e)
		{
			LOG_WARNING("刷新bin缓存失败: " + std::string(e.what()));
		}
	};
	try
	{
		hi.setpath(path);
		hi.loadfile();
		hi.Analyse();

		if (command == "show")
		{
			std::cout << showhosts(hi.get_group());
			if (save_cache) refresh_cache();
			return 0;
		}

		int changed = 0;

		if (command == "enable")
		{
			if (args.empty()) { print_usage(); return 1; }
			set_enable(hi.get_group(), args[0], true, changed);
		}
		else if (command == "disable")
		{
			if (args.empty()) { print_usage(); return 1; }
			set_enable(hi.get_group(), args[0], false, changed);
		}
		else if (command == "add")
		{
			if (args.size() < 2) { print_usage(); return 1; }
			std::string group_name = args.size() > 2 ? args[2] : "";
			add_entry(hi.get_group(), args[0], args[1], group_name);
			changed = 1;
		}
		else if (command == "moveto")
		{
			if (args.size() < 2) { print_usage(); return 1; }
			move_entry(hi.get_group(), args[0], args[1], changed);
		}
		else if (command == "remove")
		{
			if (args.empty()) { print_usage(); return 1; }
			remove_entry(hi.get_group(), args[0], changed);
		}
		else if (command == "apply")
		{
			changed = -1;
		}
		else
		{
			LOG_ERROR("未知命令: " + command);
			print_usage();
			return 1;
		}

		hi.apply();

		if (changed == 0)
		{
			std::cout << "未找到匹配的条目，文件未改变\n";
		}
		else if (changed > 0)
		{
			std::cout << "已更新 " << changed << " 条并写回: " << path << "\n";
		}
		else
		{
			std::cout << "已写回: " << path << "\n";
		}

		if (save_cache) refresh_cache();
	}
	catch (const std::exception& e)
	{
		LOG_WARNING(e.what());
		return 1;
	}

	return 0;
}
