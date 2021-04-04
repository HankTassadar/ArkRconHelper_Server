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
	//退出相关的信号

	/**
	* 更新外界来的退出信号
	*/
	void updateMainExit() {
		this->_mainExitMutex.lock();
		this->_mainExit.first = *(this->_exitFlag);
		this->_mainExitMutex.unlock();
	}

	/**
	* 主工作线程退出信号，
	*/
	void setMainExitFlag() {
		this->_mainExitMutex.lock();
		this->_mainExit.first = true;
		this->_mainExitMutex.unlock();
	}

	/**
	* 得到主工作线程退出信号
	*/
	bool getMainExitFlag() {
		this->_mainExitMutex.lock();
		auto re = this->_mainExit.first;
		this->_mainExitMutex.unlock();
		return re;
	}
	/**
	* 设置主工作线程退出状态，线程退出时调用,并向输入线程发出退出信号
	*/
	void setMainExitState() {
		this->_mainExitMutex.lock();
		this->_mainExit.second = true;
		this->_mainExitMutex.unlock();
		this->_inputExitMutex.lock();
		this->_mainExit.first = true;
		this->_inputExitMutex.unlock();
	}

	/**
	* 主工作线程是否退出,如果是返回true
	*/
	bool getMainExitState() {
		this->_mainExitMutex.lock();
		auto re = this->_mainExit.second;
		this->_mainExitMutex.unlock();
		return re;
	}

	/**
	* 输入线程退出信号，调用后输入线程开始退出
	*/
	bool getInputExitFlag() {
		this->_inputExitMutex.lock();
		auto re = this->_inputExit.first;
		this->_inputExitMutex.unlock();
		return re;
	}

	/**
	* 设置输入线程退出状态，输入线程最后退出时调用
	*/
	void setInputExitState() {
		this->_inputExitMutex.lock();
		this->_inputExit.second = true;
		this->_inputExitMutex.unlock();
	}

	/**
	* 得到输入线程退出状态,如果线程已退出，返回true
	*/
	bool getInputExitState() {
		this->_inputExitMutex.lock();
		auto re = this->_inputExit.second;
		this->_inputExitMutex.unlock();
		return re;
	}
private:
	//退出flag指针
	bool* _exitFlag;

	std::pair<bool, bool> _inputExit;
	//输入线程退出信号锁
	std::mutex _inputExitMutex;

	std::pair<bool, bool> _mainExit;
	//主工作线程退出信号锁
	std::mutex _mainExitMutex;


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
	bool _keepWindowOpen;	//保持服务器开启（崩溃自动重启）
	bool _monitorKeep;		//监视器模式开启标志
};
