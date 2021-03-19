#pragma once

#include<json.h>	//包含jsoncpp库的头文件
#include<fstream>
#include<string>
#include<iostream>
#ifdef _WIN32
	#pragma comment(lib,"jsoncpp_static.lib")
#endif // _WIN32

using namespace Json;
using namespace std;

class JsonOperate
{
public:
	JsonOperate();
	//JsonOperate(const char*);
	~JsonOperate();
public:

	/**
	*通过一个json文件进行初始化
	*/
	bool openFile(const char* jsonFilePath);

	/**
	*通过一个string进行初始化
	*/
	bool openStr(std::string&);
	/**
	*返回根节点的引用
	*/
	Json::Value& getRoot() { return this->root; }



private:
	string jsonFilePath;
	CharReaderBuilder readerBuilder;
	CharReader* jsonReader;
	JSONCPP_STRING err;
	Json::Value root;

};

