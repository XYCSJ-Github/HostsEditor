//Logout.h 日志输出
#pragma warning(disable: 4005) //禁用警告C4005: 宏重定义
#pragma once

#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <Windows.h>

#define LOG_MOD_INFO     0//消息输出
#define LOG_MOD_WARNING  1//警告输出
#define LOG_MOD_ERROR    2//错误输出
#define LOG_MOD_DEBUG    3//调试输出

#define LOGOUT(msg, model, out_mod, join_time) LO::Logout(msg, model, out_mod, join_time)//全自定义模式调用

//自定义模块名称调用
#define LOG_INFO_M(msg, model)    LO::Logout(msg, model, LOG_MOD_INFO)
#define LOG_WARNING_M(msg, model) LO::Logout(msg, model, LOG_MOD_WARNING)
#define LOG_ERROR_M(msg, model)   LO::Logout(msg, model, LOG_MOD_ERROR)
#define LOG_DEBUG_M(msg, model)   LO::Logout(msg, model, LOG_MOD_DEBUG)

//仅文字调用
#define LOG_INFO(msg)    LO::Logout(msg, model_name, LOG_MOD_INFO)
#define LOG_WARNING(msg) LO::Logout(msg, model_name, LOG_MOD_WARNING)
#define LOG_ERROR(msg)   LO::Logout(msg, model_name, LOG_MOD_ERROR)
#define LOG_DEBUG(msg)   LO::Logout(msg, model_name, LOG_MOD_DEBUG)

#define LOG_DEBUG_OUT   LO::SetDebugLogOut();//启用调试输出

#define LOG_CREATE_MODEL_NAME(name) std::string model_name = name//创建模块名
#define LOG_CREATE_MODEL_NAME_VAR(name, model_name_var) std::string model_name_var = name//自定义变量模块名

namespace LO {
	//定义颜色枚举
	enum ConsoleColor {
		Black = 0,
		Blue = 1,
		Green = 2,
		Cyan = 3,
		Red = 4,
		Magenta = 5,
		Yellow = 6,
		White = 7,
		Gray = 8,
		BrightBlue = 9,
		BrightGreen = 10,
		BrightCyan = 11,
		BrightRed = 12,
		BrightMagenta = 13,
		BrightYellow = 14,
		BrightWhite = 15
	};

	void Logout(const std::string msg, const std::string model, const int out_mod = 0, const bool join_time = true);//输出日志
	inline void Logout(const std::ostringstream& oss, const std::string model, const int out_mod = 0, const bool join_time = true) {
		Logout(oss.str(), model, out_mod, join_time);
	}
	std::tm* GetTime();//获取时间
	long long GetMSTime();//获取毫秒（3位）
	std::string GetTimestamp();//获取时间戳 (YYYY-MM-DD HH:MM:SS.mmm)
	void ClearScreen();//清屏
	void SetDebugLogOut();//启用调试
}
