#pragma once
#include <string>

#define NULL_HOSTS_PATH_STRING 0x11
#define NULL_HOSTS_DATA 0x12

class EmptyString : public std::exception
{
private:
	int type = 0x0;
public:
	explicit EmptyString(int type) { this->type = type; }
	const char* what() const noexcept override
	{
		switch (this->type)
		{
		case NULL_HOSTS_PATH_STRING:
		{
			return "未指定hosts文件路径";
		}
		case NULL_HOSTS_DATA:
		{
			return "未读取到hosts文件内容";
		}
		default:
		{
			return "未知 EmptyString";
		}
		}
	}
};

class FileWriteError : public std::exception
{
public:
	const char* what() const noexcept override
	{
		return "写入hosts文件失败";
	}
};