#pragma once

#include<iostream>
#include<thread>
#include<mutex>
#include<Windows.h>
#include<functional>
#include<map>



class CmdLineApplicationBase {
public:
	CmdLineApplicationBase(std::string,unsigned long);
	~CmdLineApplicationBase();

public:
	void run();

public:
	/**
	* 设定主工作线程退出信号，
	*/
	void setMainExitFlag() {
		this->_mainExitMutex.lock();
		*(this->_exitFlag) = true;
		this->_mainExitMutex.unlock();
	}

	/**
	* 向工作线程发送一个工作任务，{time_t,function<void()>}
	*/
	void addWork(time_t,std::function<void()>);
private:
	virtual void inputThread();
	virtual void mainWork();
	virtual void clearCmd();
protected:
	virtual void solveInput(const std::string& cmd) { };	//must be override in child class
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
		this->_inputExit.first = true;
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
	//应用层程序名称
	std::string _appName;

	//帧间隔
	unsigned long _interval;

	//退出flag指针
	bool* _exitFlag;

	std::pair<bool, bool> _inputExit;
	//输入线程退出信号锁
	std::mutex _inputExitMutex;

	std::pair<bool, bool> _mainExit;
	//主工作线程退出信号锁
	std::mutex _mainExitMutex;

	//输入线程传来的命令队列
	std::string _cmd;
	std::mutex _cmdMutex;

	std::multimap<time_t, std::function<void ()>> _workMap;
};



