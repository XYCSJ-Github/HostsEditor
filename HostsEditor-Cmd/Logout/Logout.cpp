#include <iomanip>
#include "Logout.h"

bool is_debug = false;

namespace LO {

	void Logout(const std::string msg, const std::string model, const int out_mod, const bool join_time)
	{
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		switch (out_mod)
		{
		case LOG_MOD_INFO:
		{
			SetConsoleTextAttribute(console, ConsoleColor::White);
			if (join_time)
			{
				std::tm* time = GetTime();
				long long millis = GetMSTime();
				if (time == nullptr)
				{
					std::cout << "[Error getting time]";
					break;
				}
				std::cout << "[" << std::setfill('0') << std::setw(2) << time->tm_hour << ":" << std::setfill('0') << std::setw(2) << time->tm_min << ":" << std::setfill('0') << std::setw(2) << time->tm_sec << "." << std::setfill('0') << std::setw(3) << millis << "]";
			}
			std::cout << "[Info]" << "[" << model << "]: " << msg << std::endl;
			SetConsoleTextAttribute(console, ConsoleColor::White);
			break;
		}

		case LOG_MOD_WARNING:
		{
			SetConsoleTextAttribute(console, ConsoleColor::Yellow);
			if (join_time)
			{
				std::tm* time = GetTime();
				long long millis = GetMSTime();
				if (time == nullptr)
				{
					std::cout << "[Error getting time]";
					break;
				}
				std::cout << "[" << std::setfill('0') << std::setw(2) << time->tm_hour << ":" << std::setfill('0') << std::setw(2) << time->tm_min << ":" << std::setfill('0') << std::setw(2) << time->tm_sec << "." << std::setfill('0') << std::setw(3) << millis << "]";
			}
			std::cout << "[Warning]" << "[" << model << "]: " << msg << std::endl;
			SetConsoleTextAttribute(console, ConsoleColor::White);
			break;
		}

		case LOG_MOD_ERROR:
		{
			SetConsoleTextAttribute(console, ConsoleColor::Red);
			if (join_time)
			{
				std::tm* time = GetTime();
				long long millis = GetMSTime();
				if (time == nullptr)
				{
					std::cerr << "[Error getting time]";
					break;
				}
				std::cerr << "[" << std::setfill('0') << std::setw(2) << time->tm_hour << ":" << std::setfill('0') << std::setw(2) << time->tm_min << ":" << std::setfill('0') << std::setw(2) << time->tm_sec << "." << std::setfill('0') << std::setw(3) << millis << "]";
			}
			std::cerr << "[Error]" << "[" << model << "]: " << msg << std::endl;
			SetConsoleTextAttribute(console, ConsoleColor::White);
			break;
		}

		case LOG_MOD_DEBUG:
		{
			if (is_debug == false) break;

			SetConsoleTextAttribute(console, ConsoleColor::BrightYellow);
			if (join_time)
			{
				std::tm* time = GetTime();
				long long millis = GetMSTime();
				if (time == nullptr)
				{
					std::cout << "[Error getting time]";
					break;
				}
				std::cout << "[" << std::setfill('0') << std::setw(2) << time->tm_hour << ":" << std::setfill('0') << std::setw(2) << time->tm_min << ":" << std::setfill('0') << std::setw(2) << time->tm_sec << "." << std::setfill('0') << std::setw(3) << millis << "]";
			}
			std::cout << "[Debug]" << "[" << model << "]: " << msg << std::endl;
			SetConsoleTextAttribute(console, ConsoleColor::White);
			break;
		}

		default:
			break;
		}
	}

	std::tm* GetTime()
	{
		auto now = std::chrono::system_clock::now();

		std::time_t now_time = std::chrono::system_clock::to_time_t(now);

		static std::tm timeinfo;
		if (localtime_s(&timeinfo, &now_time) != 0) {
			return nullptr;
		}
		return &timeinfo;
	}
	long long GetMSTime()
	{
		auto now = std::chrono::system_clock::now();
		auto duration_since_epoch = now.time_since_epoch();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration_since_epoch).count() % 1000;

		return millis;
	}
	std::string GetTimestamp()
	{
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
		std::tm bt;
		localtime_s(&bt, &in_time_t);
		std::ostringstream oss;
		oss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S") << '.'
			<< std::setfill('0') << std::setw(3) << ms.count();
		return oss.str();
	}

	void ClearScreen()
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hConsole, &csbi);
		DWORD written;
		SHORT w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		SHORT h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		COORD topLeft = { csbi.srWindow.Left, csbi.srWindow.Top };
		FillConsoleOutputCharacterA(hConsole, ' ', w * h, topLeft, &written);
		FillConsoleOutputAttribute(hConsole, csbi.wAttributes, w * h, topLeft, &written);
		SetConsoleCursorPosition(hConsole, topLeft);
	}

	void SetDebugLogOut()
	{
		is_debug = true;
	}
	bool GetIsDebug()
	{
		return is_debug;
	}
}