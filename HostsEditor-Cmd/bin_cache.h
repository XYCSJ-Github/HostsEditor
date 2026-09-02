#pragma once
#include "hosts_group.h"
#include <string>
#include <vector>

namespace bin_cache
{
	constexpr char MAGIC[4] = { 'H', 'E', '0', '1' };
	constexpr unsigned int VERSION = 1;

	std::string default_path();
	bool save(const std::vector<hosts_group>& groups, const std::string& path);
	bool load(const std::string& path, std::vector<hosts_group>& out);
}
