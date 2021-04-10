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

#ifdef _WIN32
#pragma comment(lib,"iconv.lib")
#endif // _WIN32



namespace MyLog {

	void log(std::string);

	std::string GBKtoUTF8(std::string);

	std::string UTF8toGBK(std::string);
	class Log {

	public:
		static Log* createLog(std::string);

		void setLogName(const std::string&);

		void logoutGBK(const std::string&);

		void logoutUTF8(const std::string&);

		void clear();
		~Log();

	private:
		Log() {};



	private:
		std::string logname;
		std::ofstream file;
		std::mutex _myMutex;
	};

};


#ifdef _DEBUG
static MyLog::Log* debuglog = MyLog::Log::createLog("AppLog/DebugLog");
static std::string debuglogfilepos = "";
#define DEBUGLOG(str)																\
	debuglogfilepos=__FILE__;														\
	debuglogfilepos+="--";															\
	debuglogfilepos += std::to_string(__LINE__);									\
	debuglogfilepos += "--";														\
	debuglogfilepos += __FUNCTION__;												\
	debuglog->logoutUTF8(TimeClass().TimeNow() + "--" + str + "--" + debuglogfilepos);
#define RELEASELOG(str)	

#endif // _DEBUG

#ifndef _DEBUG
#define DEBUGLOG(str)
static MyLog::Log* releaselog = MyLog::Log::createLog("AppLog/ReleaseLog");
static std::string releaselogpos = "";
static uint32_t releaselogcount = 0;
#define RELEASELOG(str)															\
	if(releaselogcount++==500){releaselog->clear();releaselogcount=0;}			\
	releaselogpos=__FILE__;														\
	releaselogpos+="--";														\
	releaselogpos += std::to_string(__LINE__);									\
	releaselogpos += "--";														\
	releaselogpos += __FUNCTION__;												\
	releaselog->logoutUTF8(TimeClass().TimeNow() + "--" + str + "--" + releaselogpos);
#endif // !_DEBUG

#define DEBUGLOGFIN DEBUGLOG("Function in")
#define DEBUGLOGFRE DEBUGLOG("Function return")