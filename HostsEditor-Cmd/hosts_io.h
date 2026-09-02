#pragma once
#include "MyException.h"
#include "hosts_group.h"
class hosts_io
{
public:
	inline void setpath(std::string path) { if (path.empty()) { throw EmptyString(NULL_HOSTS_PATH_STRING); } this->hosts_path = path; }
	std::vector<hosts_group>& get_group() { return this->groups; }

	void loadfile();
	void Analyse();
	void apply();

private:
	std::string trim(const std::string& str);
	hosts_group find_block(const std::string& data);

	std::vector<hosts_group> groups;
	std::string hosts_path;
	std::string data;
};
