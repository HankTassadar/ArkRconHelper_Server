#pragma once
#include<Windows.h>
#include<string>
#include<vector>
#include<set>
#include<thread>
#include"CurlOperate.h"
#include"Log.h"
#include"JsonOperate.h"
#include"TimeClass.h"
class ArkUpdate {
public:
	ArkUpdate();
	~ArkUpdate();
	void init();
private:
	struct ArkServer
	{
		HWND hwnd;	//窗口句柄
		std::string version;	//窗口的服务器版本号
		std::string name;	//服务器名称
		std::string path;	//服务器安装路径
		std::set<std::string> mods;	
		std::string map;
		std::string listenPort;	
		std::string queryPort;
		std::string rconPort;
	};

public:

	/**
	* read all server version from window text
	*/
	void readVersion();

	/**
	* shutdown the servers which mods has just been update
	*/
	void shutdownAndModsUpdate(const std::vector<std::string>&);

	/**
	*check if any server need to update
	*return true if server need update
	*/
	bool checkUpdate();

	/**
	* check for if any server crashed with UE4，if there is a server crashed, auto close it
	* 查看是否有crashed窗口,如果有，则关闭该窗口使其自动重启 
	*/
	bool checkCrashed();

	/**
	* update all servers if the server is needed
	* 更新所有服务器
	*/
	void arkUpdate();

	/**
	* close all ark window ;
	* 关闭所有方舟窗口
	*/
	void closeAll();

	/**
	* restart all ark server
	* 重启方舟,参数为服务器安装路径
	*/
	void arkRestart();

	/**
	* get version number
	* 返回当前的网络版本号
	*/
	std::string getVersion() {
		return this->_netVersion;
	}

	std::vector<ArkServer>& getServer() {
		return this->_arkServerWindow;
	}

	void log(std::string str) {
		this->_updateLog->logoutUTF8(str);
	}
private:
	/**
	* update the newest ark server version number from http://arkdedicated.com/version 
	* 从网络更新方舟最新版本号
	*/
	void updateVersionFromUrl();

	/**
	* read ark server version from installpath/version.txt
	* 读取方舟服务器的版本号，参数为config中的path(服务器安装路径)
	*/
	std::string readVersion(const HWND& hwnd);

	/**
	* use handle to close an ark server window
	* 通过窗口句柄关闭方舟窗口，在窗口彻底关闭前阻塞
	*/
	bool closeArkWindow(HWND);

	/**
	* check ang update all ark server window hwnd
	* 根据配置文件检查所有窗口
	*/
	void arkCheckWindows();


private:
	//更新日志
	MyLog::Log* _updateLog;

	//网络最新版本
	std::string _netVersion;

	//更新的相关配置json
	JsonOperate* _arkJson;

	//方舟服务端窗口名称及其句柄
	std::vector<ArkServer> _arkServerWindow;


};

