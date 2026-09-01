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

	void add_host_group(hosts_info_group hosts) { if (sizeof(hosts) == 0) { throw EmptyStruct(EMPTY_GROUP); }this->hosts.push_back(hosts); }
	std::vector<hosts_info_group> get_host_group() { return this->hosts; }

	void list_to_pair();

private:
	std::vector<hosts_info_group> hosts;
};

