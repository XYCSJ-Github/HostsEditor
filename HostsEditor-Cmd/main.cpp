#include "hosts_io.h"
#include "Logout/Logout.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

static const char* DEFAULT_HOSTS_PATH = "C:\\Windows\\System32\\drivers\\etc\\hosts";

std::string showhosts(std::vector<hosts_group> hg)
{
	std::string outstr;
	for (hosts_group& c : hg)
	{
		for (hosts_info_group& a : c.get_host_group())
		{
			std::string sstr = a.start_marker;
			size_t spos = sstr.find('#');

			size_t start = sstr.find_first_not_of(" \t", spos + 1);
			size_t end = sstr.find(' ', start);

			if (end == std::string::npos) end = sstr.length();

			outstr += sstr.substr(start, end - start) + ":\n";
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
		<< "  enable <host>          启用匹配的条目并写回\n"
		<< "  disable <host>         禁用匹配的条目并写回\n"
		<< "  add <ip> <host>        新增启用条目到 Default 组并写回\n"
		<< "  remove <host>          删除匹配的条目并写回\n"
		<< "  help                   显示本帮助\n"
		<< "\n"
		<< "选项:\n"
		<< "  --file <路径>            指定 hosts 文件路径 (默认: " << DEFAULT_HOSTS_PATH << ")\n";
}

static void set_enable(std::vector<hosts_group>& groups, const std::string& host, bool enable, int& changed)
{
	for (auto& g : groups)
	{
		for (auto& ig : g.get_host_group())
		{
			for (auto& p : ig.host_pair)
			{
				if (p.host_name == host)
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

static void add_entry(std::vector<hosts_group>& groups, const std::string& ip, const std::string& host)
{
	for (auto& g : groups)
	{
		for (auto& ig : g.get_host_group())
		{
			if (ig.is_default_group)
			{
				ig.host_pair.push_back(host_pair{ true, host, ip });
				return;
			}
		}
	}

	if (!groups.empty())
	{
		hosts_info_group ig;
		ig.is_default_group = true;
		ig.start_marker = "#Default Start";
		ig.end_marker = "#Default End";
		ig.host_pair.push_back(host_pair{ true, host, ip });
		groups[0].add_host_group(ig);
	}
}

int main(int argc, char** argv)
{
	LOG_CREATE_MODEL_NAME("main");

	std::string path = DEFAULT_HOSTS_PATH;
	std::string command;
	std::vector<std::string> args;

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
	try
	{
		hi.setpath(path);
		hi.loadfile();
		hi.Analyse();

		if (command == "show")
		{
			std::cout << showhosts(hi.get_group());
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
			add_entry(hi.get_group(), args[0], args[1]);
			changed = 1;
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
	}
	catch (const std::exception& e)
	{
		LOG_WARNING(e.what());
		return 1;
	}

	return 0;
}
