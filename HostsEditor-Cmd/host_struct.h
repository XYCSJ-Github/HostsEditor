#pragma once
#include <string>
#include <vector>
struct host_pair
{
	bool enable;
	std::string host_name;
	std::string ip;
};

struct hosts_info_group
{
	std::string start_marker;
	std::string end_marker;
	std::vector<host_pair> host_pair;
	std::vector<std::string> data_line;
	bool is_default_group = false;
};