#pragma once

#include<string>
#include<vector>
#include<iostream>
#include<mutex>
#include<thread>
#include"ArkServer.h"

#pragma comment(lib,"ws2_32.lib")

class ArkRcon
{
public:
	ArkRcon();
	~ArkRcon();
public:
	bool addServer(Rcon_addr addr);

	bool init();

	/**
	*��������socket�������е����ݣ���������������б�
	*/
	void update();

	/**
	*�����з��������͹���
	*/
	void broadcast(std::string&);

	/**
	*�õ�����������ҵ���Ϸ�ڵĽ�ɫ����
	*/
	void updateGameName();

public:
	std::vector<ArkServer*> _server;
};


