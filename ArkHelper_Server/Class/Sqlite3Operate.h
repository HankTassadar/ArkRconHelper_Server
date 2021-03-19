#pragma once

#include"sqlite3.h"
#include<cstring>
#include<string>
#include<vector>
#include<iconv.h>

#ifdef _WIN32
	//windowsϵͳ���Լ�׼��iconv.h��iconv.lib
	#ifdef _WIN64
	#pragma comment(lib,"x64/sqlite3.lib")
	#pragma comment(lib,"x64/iconv.lib")
	#else
	#pragma comment(lib,"sqlite3lib.lib")
	#pragma comment(lib,"iconv.lib")
	#endif // _WIN64
#endif // _WIN32





class Sqlite3Operate{

public:
	/**
	*sqlite3���������ͣ�NULL_TYPEΪ�գ�INTEGERΪ���ͣ�REALΪ�����ͣ�TEXTΪ�ַ�����UTF-8����BLOBΪ���ݿ飨���ȥʲô����ʲô��
	*/
	enum class DataType
	{
		NULL_TYPE,
		INTEGER,
		REAL,
		TEXT,
		BLOB
	};

public:
	/**
	*����ʱ��ʹ��shared_ptr,��ֹ�����˳�ʱ��������
	*/
	Sqlite3Operate(const char*);
	~Sqlite3Operate();
	Sqlite3Operate(const Sqlite3Operate&) = delete;
	Sqlite3Operate operator&=(const Sqlite3Operate&) = delete;

public:
	/**
	*�����ݿ����һ����Ϊname�ı�
	*/
	bool addTable(const char*);

	/**
	*�õ����ݿ������б�����ֵ�һ��vector
	*ʹ�����ǵ�delete����ֹ�ڴ�й©
	*/
	std::vector<std::string>* getAllTableName();

	/**
	*ɾ��һ����Ϊname�ı�
	*/
	bool dropTable(const char*);

	/**
	*�����ݿ�����Ϊtable�ı�������һ��Ϊname��Column���У�
	*/
	bool addColumn(const char*, const char*);

	/**
	*�����ݿ��в�ѯ���ݣ�ֻ����һ�����ݣ�
	*��һ����������ѯ�ı���
	*��һ����������ѯ������
	*�ڶ���������ѯ����������
	*����������������ֵ
	*/
	std::string* findOneData(const std::string&, const std::string&, const std::string&, const std::string&);

	/**
	*ִ��һ������һ��ֵ��sql���
	*/
	std::string getOneData(const std::string&);

	/**
	*ִ��һ��������ֵ�����
	*/
	bool doOneSqlStr(const std::string&);

	/**
	*�����ݿ����һ���¼�¼
	*��һ����������
	*��һ�����������飬��"id,uid,data"
	*�ڶ�������ֵ����"1,asdqwe,daiero"��ע�⣬text��Ҫ�ӵ�����
	*/
	bool insertNew(const std::string&, const std::string&, const std::string&);

private:
	std::string GBKtoUTF8(std::string gbkStr);
private:
	sqlite3* dataBasePtr;
	std::string dbPath;
};



