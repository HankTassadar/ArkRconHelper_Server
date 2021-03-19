#pragma once

#include"sqlite3.h"
#include<cstring>
#include<string>
#include<vector>
#include<iconv.h>

#ifdef _WIN32
	//windows系统下自己准备iconv.h和iconv.lib
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
	*sqlite3的数据类型，NULL_TYPE为空，INTEGER为整型，REAL为浮点型，TEXT为字符串（UTF-8），BLOB为数据块（存进去什么就是什么）
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
	*构造时请使用shared_ptr,防止程序退出时忘记析构
	*/
	Sqlite3Operate(const char*);
	~Sqlite3Operate();
	Sqlite3Operate(const Sqlite3Operate&) = delete;
	Sqlite3Operate operator&=(const Sqlite3Operate&) = delete;

public:
	/**
	*向数据库添加一个名为name的表
	*/
	bool addTable(const char*);

	/**
	*得到数据库中所有表的名字的一个vector
	*使用完后记得delete掉防止内存泄漏
	*/
	std::vector<std::string>* getAllTableName();

	/**
	*删除一个名为name的表
	*/
	bool dropTable(const char*);

	/**
	*向数据库中名为table的表中增加一个为name的Column（列）
	*/
	bool addColumn(const char*, const char*);

	/**
	*从数据库中查询数据（只返回一个数据）
	*第一参数：所查询的表名
	*第一参数：所查询的列名
	*第二参数：查询索引的列名
	*第三参数：索引的值
	*/
	std::string* findOneData(const std::string&, const std::string&, const std::string&, const std::string&);

	/**
	*执行一个返回一个值的sql语句
	*/
	std::string getOneData(const std::string&);

	/**
	*执行一个不反回值的语句
	*/
	bool doOneSqlStr(const std::string&);

	/**
	*向数据库添加一行新记录
	*第一参数：表名
	*第一参数：列名组，如"id,uid,data"
	*第二参数：值，如"1,asdqwe,daiero"，注意，text型要加单引号
	*/
	bool insertNew(const std::string&, const std::string&, const std::string&);

private:
	std::string GBKtoUTF8(std::string gbkStr);
private:
	sqlite3* dataBasePtr;
	std::string dbPath;
};



