#pragma once
#include "hosts_group.h"
class hotos_io
{
public:
	hotos_io() = default;
	~hotos_io() = default;

	inline void setpath(std::string path) { if (!path.empty()) { throw EmptyString("未指定hosts文件路径"); } }

	void reflush();
	void apply();

private:
	hosts_group* hosts_group;
	std::string hosts_path;
};

