#pragma once
#include "host_struct.h"
class hosts_group
{
public:
	void add_host_group(hosts_info_group group) { this->hosts.push_back(group); }
	std::vector<hosts_info_group>& get_host_group() { return this->hosts; }

	void list_to_pair();
	std::string to_string() const;

private:
	std::vector<hosts_info_group> hosts;
};
