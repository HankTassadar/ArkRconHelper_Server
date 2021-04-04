#pragma once
#include<iostream>
#include<thread>
#include<mutex>
#include<Windows.h>
#include<functional>
#include<map>

//windows下接收程序的退出信号，如Ctrl+C等信号
#ifdef _WIN32
	bool g_bExit;

	HANDLE g_hEvent = INVALID_HANDLE_VALUE;

	BOOL CALLBACK CosonleHandler(DWORD ev)
	{
		BOOL bRet = FALSE;
		switch (ev)
		{
		// the user wants to exit. 

		case CTRL_CLOSE_EVENT:
		// Handle the CTRL-C signal. 
		case CTRL_C_EVENT:
		case CTRL_SHUTDOWN_EVENT:
		case CTRL_LOGOFF_EVENT:
		//MessageBox(NULL, L"CTRL+BREAK received!", L"CEvent", MB_OK);
			g_bExit = true;
			WaitForSingleObject(g_hEvent, INFINITY);
			bRet = TRUE;
			break;
		default:
			break;
	}
		return bRet;
}
#endif // _WIN32

//#define COUT(str) std::cout<<MyLog::UTF8toGBK(str)<<std::endl;
#define COUT(str) std::cout<<str<<std::endl;
#define CIN(str) std::cin>>str;


class CmdLineApplicationBase {
public:
	CmdLineApplicationBase(std::string);
	~CmdLineApplicationBase();

public:
	void run();

public:
	/**
	* 设定主工作线程退出信号，
	*/
	void setMainExitFlag() {
		this->_mainExitMutex.lock();
		this->_mainExit.first = true;
		this->_mainExitMutex.unlock();
	}

protected:
	virtual void inputThread();
	virtual void mainWork();
	virtual std::string solveInput(const std::string&) = 0;
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

	std::multimap<time_t, std::function<void()>> _workMap;
};