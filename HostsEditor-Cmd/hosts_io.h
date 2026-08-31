#pragma once
#include "hosts_group.h"
#include <fstream>
class hosts_io
{
public:
	hosts_io() = default;
	~hosts_io();

	inline void setpath(std::string path) { if (path.empty()) { throw EmptyString("未指定hosts文件路径"); } this->hosts_path = path; }

	void loadfile();
	void Analyse();

	std::string trim(const std::string& str);
	std::vector<hosts_group> find_block(const std::string& data);

	void reflush();
	void apply();

private:
	std::vector<hosts_group> groups;
	std::string hosts_path;
	std::string* data;
};

