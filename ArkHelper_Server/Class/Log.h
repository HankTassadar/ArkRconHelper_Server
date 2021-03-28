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
#include"TimeClass.h"
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

static MyLog::Log* debuglog = MyLog::Log::createLog("AppLog/DebugLog");
static std::string debuglogfilepos = "";
#ifdef _DEBUG
#define DEBUGLOG(str)																\
	debuglogfilepos=__FILE__;														\
	debuglogfilepos+="--";															\
	debuglogfilepos += std::to_string(__LINE__);									\
	debuglogfilepos += "--";														\
	debuglogfilepos += __FUNCTION__;												\
	debuglog->logoutUTF8(TimeClass().TimeNow() + "--" + str + "--" + debuglogfilepos);


#endif // _DEBUG

#ifndef _DEBUG
#define DEBUGLOG(str)
#endif // !_DEBUG

#define DEBUGLOGFIN DEBUGLOG("Function in")
#define DEBUGLOGFRE DEBUGLOG("Function in")