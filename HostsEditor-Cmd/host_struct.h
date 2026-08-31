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
	std::string start_makeer;
	std::string end_makeer;
	std::vector<host_pair> host_pair;
	std::vector<std::string> data_line;
};