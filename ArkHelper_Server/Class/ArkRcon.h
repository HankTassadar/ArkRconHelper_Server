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

	/**
	* shutdown all server's rcon connecting
	*/
	void shutConnect();

	/**
	* reconnect all server's rcon
	*/
	void reconnect();

	/**
	* kick player out of server
	*/
	void kick(const std::string&);

	/**
	* get add server state
	* if server online,return pair<string servername,bool ifonline>
	*/
	std::vector<std::pair<std::string, bool>>* getState();

public:
	std::vector<ArkServer*> _server;
};


