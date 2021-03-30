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
	//�������������߳�
	void inputThread();
private:
	ArkHelperServerAPP();

	//�����ʼ��
	bool init();

	//��Ҫ�����̺߳��������еĹ������ڴ��߳������
	void mainWork();

	/**
	*��̨��������
	*/
	void work();

	/**
	* ������������
	*/
	void solveInput();

	/**
	* ���Ʒ�����״̬UI
	*/
	void drawState();

private:
	//�˳�flagָ��
	bool* _exitFlag;
	bool _inputExit;

	//����ѭ�������������ڴ��Լ�ʱ
	u_int _count;
	//�������ָ��
	static ArkHelperServerAPP* impl;

	//�����̴߳������������
	std::string _cmd;
	std::mutex _cmdQueueMutex;
	//�����߳���Ҫ���յĽ��
	std::string _cmdResult;
	std::mutex _cmdResultMutex;

	//�˳��ź���
	std::mutex _exitMutex;

	//��־
	MyLog::Log* _appLog;

	//RconConfig
	JsonOperate* _rconConfig;

	//RconServer
	ArkRcon _rcon;

	//���۸���
	ArkUpdate _update;

	//����֡��
	int _frame;

	bool _inputModeActive;
	bool _workModeActive;
};
