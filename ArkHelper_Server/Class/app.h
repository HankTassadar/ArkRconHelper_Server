#pragma once
#include"ArkRcon.h"
#include<Windows.h>
#include<iostream>	
#include<thread>
#include<string>
#include<queue>
#include<mutex>
#include<conio.h>
#include"Log.h"
#include"TimeClass.h"
#include"CurlOperate.h"
#include"ArkUpdate.h"

#define COUT(str) std::cout<<MyLog::UTF8toGBK(str)<<std::endl;
//#define COUT(str) std::cout<<str<<std::endl;
#define CIN(str) std::cin>>str;
#define CINUTF8(str) std::cin>>str;str=MyLog::GBKtoUTF8(str);

class ArkHelperServerAPP {
public:
	static ArkHelperServerAPP* create();

	~ArkHelperServerAPP();

public:
	int run(bool*);

private:
	enum class Cmd{
		CHECKARKVERSION,
		HELP
	};
private:
	//程序命令输入线程
	void inputThread();
private:
	ArkHelperServerAPP();

	//程序初始化
	bool init();

	//主要工作线程函数，所有的工作都在此线程上完成
	void mainWork();

	/**
	*后台工作内容
	*/
	void work();

	/**
	* 处理输入命令
	*/
	void solveInput();

	/**
	* 绘制服务器状态UI
	*/
	void drawState();

private:
	//退出flag指针
	bool* _exitFlag;
	bool _inputExit;

	//工作循环计数器，用于粗略记时
	u_int _count;
	//本对象的指针
	static ArkHelperServerAPP* impl;

	//输入线程传来的命令队列
	std::string _cmd;
	std::mutex _cmdQueueMutex;
	//输入线程需要接收的结果
	std::string _cmdResult;
	std::mutex _cmdResultMutex;

	//退出信号锁
	std::mutex _exitMutex;

	//日志
	MyLog::Log* _appLog;

	//RconConfig
	JsonOperate* _rconConfig;

	//RconServer
	ArkRcon _rcon;

	//方舟更新
	ArkUpdate _update;

	//运行帧率
	int _frame;

	bool _inputModeActive;
	bool _workModeActive;
};
