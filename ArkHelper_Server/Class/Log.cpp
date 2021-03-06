#include"Log.h"

using namespace MyLog;

Log::~Log() {
	file.close();
	//std::cout << MyLog::GBKtoUTF8("Log析构调用") << std::endl;
}

void Log::logoutUTF8(const std::string& loginfo)
{
	this->_myMutex.lock();
	file << loginfo << std::endl;
	file.flush();
	this->_myMutex.unlock();
}


Log* Log::createLog(std::string logname) {
	Log* re = nullptr;
	re = new Log();
	if (re)
		re->file.open(logname+".txt", std::ios::app);
	re->setLogName(logname + ".txt");
	return re;
}

void MyLog::Log::setLogName(const std::string& logname)
{
	this->logname = logname;
}

void Log::logoutGBK(const std::string &loginfo) {
		this->logoutUTF8(GBKtoUTF8(loginfo));
}

void MyLog::Log::clear()
{
	file.close();
	file.clear();
	file.open(this->logname, std::ios::out | std::ios::trunc);
	file << "";
	file.close();
	file.clear();
	file.open(this->logname, std::ios::app);
}


std::string MyLog::UTF8toGBK(std::string utf8)
{
	iconv_t iconvH = iconv_open("gb18030", "UTF-8");
	const char* strChar = utf8.c_str();
	const char** pin = &strChar;
	size_t strLength = utf8.length();
	char* outbuf = (char*)malloc(strLength * 4);
	char* pBuff = outbuf;
	memset(outbuf, 0, strLength * 4);

	size_t outLength = strLength * 4;
	if (-1 == iconv(iconvH, (const char**)pin, &strLength, &outbuf, &outLength)) {
		free(pBuff);
		iconv_close(iconvH);
		return "错误";
	}
	std::string re = pBuff;
	iconv_close(iconvH);

	return re;
}

void MyLog::log(std::string loginfo) {
	time_t t;
	time(&t);
	char buffer[100];
	ctime_s(buffer, 100, &t);
	std::string tm(buffer);
	std::cout << "日志信息：" << tm + loginfo << std::endl;
}

std::string MyLog::GBKtoUTF8(std::string gbkStr) {
	iconv_t iconvH = iconv_open("UTF-8", "gb18030");
	const char* strChar = gbkStr.c_str();
	const char** pin = &strChar;
	size_t strLength = gbkStr.length();
	char* outbuf = (char*)malloc(strLength * 4);
	char* pBuff = outbuf;
	::memset(outbuf, 0, strLength * 4);

	size_t outLength = strLength * 4;
	if (-1 == iconv(iconvH, (const char**)pin, &strLength, &outbuf, &outLength)) {
		free(pBuff);
		iconv_close(iconvH);
		return "failed";
	}
	gbkStr = pBuff;
	iconv_close(iconvH);

	return gbkStr;
}