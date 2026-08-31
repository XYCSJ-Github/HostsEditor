#pragma once
#include "MyException.h"
#include "host_struct.h"
class hosts_group
{
public:
	hosts_group() = default;
	~hosts_group() = default;

	void add(int index);
	void add(std::string host_name);
	void del(int index);
	void del(std::string host_name);
	void enable(int index);
	void enable(std::string host_name);
	void disenable(int index);
	void disenable(std::string host_name);

private:
	host_struct *hosts;
};

