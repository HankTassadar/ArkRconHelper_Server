#pragma once
#include<iostream>
#include<string>
#include<string.h>
#include<fstream>
#include<ctime>
#include<mutex>
#include<memory>
#include<iconv.h>
#include<cstdlib>

#pragma comment(lib,"iconv.lib")

namespace MyLog{

	void log(std::string);

	std::string GBKtoUTF8(std::string);

	std::string UTF8toGBK(std::string);
	class Log {
	public:
		static Log* createLog(std::string);

		void logoutGBK(const std::string&);

		void logoutUTF8(const std::string&);
		~Log();

	private:
		Log() {};

		

	private:
		std::ofstream file;
		std::mutex _myMutex;
};

}


