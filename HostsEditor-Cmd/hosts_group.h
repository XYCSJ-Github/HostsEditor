#pragma once
#include "MyException.h"
#include "host_struct.h"
#include <sstream>
#include <regex>
class hosts_group
{
public:
	hosts_group() = default;
	~hosts_group() = default;

	void add_host_pair(hosts_info_group hosts) { if (sizeof(hosts) == 0) { throw EmptyStruct("空的组"); }this->hosts.push_back(hosts); }

private:
	std::vector<hosts_info_group> hosts;
};

