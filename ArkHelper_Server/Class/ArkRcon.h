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
	*update online players list
	*/
	void updateplayerlist();

	/**
	*clear all socket recv(using for keep alive pack)
	*/
	void clearRecv();

	/**
	*send a broadcast to all servers
	*/
	void broadcast(std::string&);

	/**
	*get all online player's game character name
	*/
	void updateGameName();

	/**
	*sene a cmd to all servers and wait for recv 
	*/
	void sendCmdAndWiatForItRecv(const std::string&);

	void shutConnect();

	void reconnect();
public:
	std::vector<ArkServer*> _server;
};


