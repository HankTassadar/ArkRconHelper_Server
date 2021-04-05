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
	* �趨�������߳��˳��źţ�
	*/
	void setMainExitFlag() {
		this->_mainExitMutex.lock();
		*(this->_exitFlag) = true;
		this->_mainExitMutex.unlock();
	}

	/**
	* �����̷߳���һ����������{time_t,function<void()>}
	*/
	void addWork(time_t,std::function<void()>);
private:
	virtual void inputThread();
	virtual void mainWork();
	virtual void clearCmd();
protected:
	virtual void solveInput(const std::string& cmd) { };	//must be override in child class
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
		this->_inputExit.first = true;
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

	std::multimap<time_t, std::function<void ()>> _workMap;
};



