#include "bin_cache.h"
#include "Logout/Logout.h"
#include <cstdint>
#include <fstream>
#include <Windows.h>

namespace bin_cache
{
	namespace
	{
		bool write_str(std::ofstream& out, const std::string& s)
		{
			uint32_t len = (uint32_t)s.size();
			out.write(reinterpret_cast<const char*>(&len), sizeof(len));
			if (len > 0) out.write(s.data(), len);
			return out.good();
		}

		bool write_u32(std::ofstream& out, uint32_t v)
		{
			out.write(reinterpret_cast<const char*>(&v), sizeof(v));
			return out.good();
		}

		bool write_u8(std::ofstream& out, uint8_t v)
		{
			out.write(reinterpret_cast<const char*>(&v), sizeof(v));
			return out.good();
		}

		bool read_exact(std::ifstream& in, char* buf, std::streamsize n)
		{
			in.read(buf, n);
			return in.gcount() == n;
		}

		bool read_u32(std::ifstream& in, uint32_t& v)
		{
			if (!read_exact(in, reinterpret_cast<char*>(&v), sizeof(v))) return false;
			return true;
		}

		bool read_u8(std::ifstream& in, uint8_t& v)
		{
			if (!read_exact(in, reinterpret_cast<char*>(&v), sizeof(v))) return false;
			return true;
		}

		bool read_str(std::ifstream& in, std::string& s)
		{
			uint32_t len = 0;
			if (!read_u32(in, len)) return false;
			if (len > (1u << 24)) return false;
			s.resize(len);
			if (len > 0 && !read_exact(in, &s[0], len)) return false;
			return true;
		}
	}

	std::string default_path()
	{
		char buf[MAX_PATH] = {};
		DWORD len = GetEnvironmentVariableA("LOCALAPPDATA", buf, MAX_PATH);
		std::string path;
		if (len > 0 && len < MAX_PATH)
		{
			path = buf;
		}
		if (path.empty())
		{
			path = ".";
		}
		return path + "\\Temp\\HostsEditor-cache.bin";
	}

	bool save(const std::vector<hosts_group>& groups, const std::string& path)
	{
		std::ofstream out(path, std::ios::binary | std::ios::out | std::ios::trunc);
		if (!out.is_open()) return false;

		out.write(MAGIC, sizeof(MAGIC));
		if (!write_u32(out, VERSION)) return false;
		if (!write_u32(out, (uint32_t)groups.size())) return false;

		for (const auto& g : groups)
		{
			if (!write_u32(out, (uint32_t)g.get_host_group().size())) return false;

			for (const auto& ig : g.get_host_group())
			{
				if (!write_str(out, ig.start_marker)) return false;
				if (!write_str(out, ig.end_marker)) return false;

				if (!write_u32(out, (uint32_t)ig.host_pair.size())) return false;
				for (const auto& p : ig.host_pair)
				{
					if (!write_u8(out, p.enable ? 1 : 0)) return false;
					if (!write_str(out, p.host_name)) return false;
					if (!write_str(out, p.ip)) return false;
				}

				if (!write_u32(out, (uint32_t)ig.data_line.size())) return false;
				for (const auto& line : ig.data_line)
				{
					if (!write_str(out, line)) return false;
				}

				if (!write_u8(out, ig.is_default_group ? 1 : 0)) return false;
			}
		}

		out.close();
		return out.good();
	}

	bool load(const std::string& path, std::vector<hosts_group>& out)
	{
		std::ifstream in(path, std::ios::binary | std::ios::in);
		if (!in.is_open()) return false;

		char magic[4] = {};
		if (!read_exact(in, magic, sizeof(magic))) return false;
		if (memcmp(magic, MAGIC, sizeof(MAGIC)) != 0) return false;

		uint32_t version = 0;
		if (!read_u32(in, version)) return false;
		if (version != VERSION) return false;

		uint32_t group_count = 0;
		if (!read_u32(in, group_count)) return false;

		out.clear();
		out.reserve(group_count);

		for (uint32_t gi = 0; gi < group_count; ++gi)
		{
			hosts_group g;

			uint32_t hg_count = 0;
			if (!read_u32(in, hg_count)) return false;

			for (uint32_t hi = 0; hi < hg_count; ++hi)
			{
				hosts_info_group ig;

				if (!read_str(in, ig.start_marker)) return false;
				if (!read_str(in, ig.end_marker)) return false;

				uint32_t pair_count = 0;
				if (!read_u32(in, pair_count)) return false;
				ig.host_pair.reserve(pair_count);
				for (uint32_t pi = 0; pi < pair_count; ++pi)
				{
					host_pair p{};
					uint8_t enable = 0;
					if (!read_u8(in, enable)) return false;
					p.enable = enable != 0;
					if (!read_str(in, p.host_name)) return false;
					if (!read_str(in, p.ip)) return false;
					ig.host_pair.push_back(p);
				}

				uint32_t line_count = 0;
				if (!read_u32(in, line_count)) return false;
				ig.data_line.reserve(line_count);
				for (uint32_t li = 0; li < line_count; ++li)
				{
					std::string line;
					if (!read_str(in, line)) return false;
					ig.data_line.push_back(line);
				}

				uint8_t is_default = 0;
				if (!read_u8(in, is_default)) return false;
				ig.is_default_group = is_default != 0;

				g.add_host_group(ig);
			}

			out.push_back(g);
		}

		return true;
	}
}
