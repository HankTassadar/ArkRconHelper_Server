#pragma once

#include<json.h>	//����jsoncpp���ͷ�ļ�
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
	*ͨ��һ��json�ļ����г�ʼ��
	*/
	bool openFile(const char* jsonFilePath);

	/**
	*ͨ��һ��string���г�ʼ��
	*/
	bool openStr(std::string&);
	/**
	*���ظ��ڵ������
	*/
	Json::Value& getRoot() { return this->root; }



private:
	string jsonFilePath;
	CharReaderBuilder readerBuilder;
	CharReader* jsonReader;
	JSONCPP_STRING err;
	Json::Value root;

};

