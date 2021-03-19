#pragma once
#include<curl/curl.h>
#include<string>
#include<sstream>
#ifdef _DEBUG
	#pragma comment(lib,"libcurld.lib")
#else
	#pragma comment(lib,"libcurl.lib")
#endif // _DEBUG


class CurlOperate {
public:
	static std::string get(const std::string& url);
	static size_t recive_data_fun(void* ptr, size_t size, size_t nmemb, void* stream) {
		if (size * nmemb == 0)return nmemb;
		size_t strSize = size * nmemb + 1;
		char* str = new char[strSize]; 
		char* tempStr = new char[strSize];
		memset(str, 0, strSize);
		memcpy(str, ptr, size * nmemb);

		//剔除所有的'\r'
		char* temp = str;
		char* head = str;
		size_t length = 0;
		while (temp < str + strSize) {
			if (*temp != '\r')
				length++;
			else {
				memset(tempStr, 0, strSize);
				memcpy(tempStr, head, length);
				(*(std::stringstream*)stream) << tempStr;
				head = temp + 1;
				length = 0;
			}
			temp++;
		}
		memset(tempStr, 0, strSize);
		memcpy(tempStr, head, length);
		(*(std::stringstream*)stream) << tempStr;
		//释放空间
		delete[](tempStr);
		delete[](str);
		return nmemb;
	}

};