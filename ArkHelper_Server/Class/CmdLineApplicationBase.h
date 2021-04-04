#pragma once
#include<iostream>
#include<thread>
#include<mutex>
#include<Windows.h>
#include<functional>
#include<map>

//windows�½��ճ�����˳��źţ���Ctrl+C���ź�
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
	* �趨�������߳��˳��źţ�
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
	//�˳���ص��ź�

	/**
	* ������������˳��ź�
	*/
	void updateMainExit() {
		this->_mainExitMutex.lock();
		this->_mainExit.first = *(this->_exitFlag);
		this->_mainExitMutex.unlock();
	}

	/**
	* �õ��������߳��˳��ź�
	*/
	bool getMainExitFlag() {
		this->_mainExitMutex.lock();
		auto re = this->_mainExit.first;
		this->_mainExitMutex.unlock();
		return re;
	}

	/**
	* �����������߳��˳�״̬���߳��˳�ʱ����,���������̷߳����˳��ź�
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
	* �������߳��Ƿ��˳�,����Ƿ���true
	*/
	bool getMainExitState() {
		this->_mainExitMutex.lock();
		auto re = this->_mainExit.second;
		this->_mainExitMutex.unlock();
		return re;
	}

	/**
	* �����߳��˳��źţ����ú������߳̿�ʼ�˳�
	*/
	bool getInputExitFlag() {
		this->_inputExitMutex.lock();
		auto re = this->_inputExit.first;
		this->_inputExitMutex.unlock();
		return re;
	}

	/**
	* ���������߳��˳�״̬�������߳�����˳�ʱ����
	*/
	void setInputExitState() {
		this->_inputExitMutex.lock();
		this->_inputExit.second = true;
		this->_inputExitMutex.unlock();
	}

	/**
	* �õ������߳��˳�״̬,����߳����˳�������true
	*/
	bool getInputExitState() {
		this->_inputExitMutex.lock();
		auto re = this->_inputExit.second;
		this->_inputExitMutex.unlock();
		return re;
	}


private:
	//Ӧ�ò��������
	std::string _appName;

	//֡���
	unsigned long _interval;

	//�˳�flagָ��
	bool* _exitFlag;

	std::pair<bool, bool> _inputExit;
	//�����߳��˳��ź���
	std::mutex _inputExitMutex;

	std::pair<bool, bool> _mainExit;
	//�������߳��˳��ź���
	std::mutex _mainExitMutex;

	//�����̴߳������������
	std::string _cmd;
	std::mutex _cmdMutex;

	std::multimap<time_t, std::function<void()>> _workMap;
};