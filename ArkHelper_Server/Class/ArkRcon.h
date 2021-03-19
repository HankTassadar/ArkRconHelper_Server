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
	*接收所有socket缓冲区中的数据，并更新在线玩家列表
	*/
	void update();

	/**
	*向所有服务器发送公告
	*/
	void broadcast(std::string&);

	/**
	*得到所有在线玩家的游戏内的角色名字
	*/
	void updateGameName();

public:
	std::vector<ArkServer*> _server;
};


