#pragma once

#include"ArkServer.h"
#include<map>

class ArkModsUpdate {
public:

	ArkModsUpdate();
	~ArkModsUpdate();
public:
	/**
	* return true if any mods is need to update
	*/
	bool checkUpdate();

	/**
	* run the update server
	*/
	bool updateServerRun();

	/**
	* connect the update server
	* return true if server rcon connected
	*/
	bool connectServer();

	/**
	* shutdown mods update server
	* retrun the updated mods id
	*/
	std::vector<std::string> shutdownUpdateServer();
	
	/**
	* call this function if just update finished and the server which mods updated reboot 
	*/
	void setUpdateDown();

	/**
	* get all mods last update time
	*/
	std::vector<std::pair<std::string, time_t>> getUpdateTime();

	/**
	* set server window hwnd,do not use this function out of class
	*/
	void setHwnd(HWND hwnd) {
		this->_server.hwnd = hwnd;
	}
private:
	bool init();
private:
	struct UpdateServer {
		HWND hwnd;
		std::string name;
		std::string path;
		std::string port;
		std::string queryport;
		std::string rconport;
		std::string pass;
		std::vector<std::string> mods;
	};

private:
	UpdateServer _server;

	ArkServer _connection;

	std::map<std::string, std::pair<time_t, time_t>> _mods;

	std::string _startCmd;
};