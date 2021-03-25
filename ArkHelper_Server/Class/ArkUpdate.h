#pragma once
#include<Windows.h>
#include<string>
#include<vector>
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
		std::string map;
		std::string listenPort;	
		std::string queryPort;
		std::string rconPort;
		bool isonline;	//服务器是否在线
	};

public:
	/**
	*check if server need to update
	*return true if server need update
	*/
	bool needForUpdate();

	/**
	*枚举窗口找到所有服务器窗口后得到其窗口句柄
	*/
	static BOOL CALLBACK EnumWindowsCallBack(HWND hwnd, LPARAM lParam);

	/**
	*查看是否有crashed窗口 
	*/
	bool checkCrashed();
	/**
	*update all servers if the server is needed
	*/
	void arkUpdate();

	/**
	*通过窗口句柄关闭方舟服务器窗口
	*/
	bool closeArkWindow(HWND);

	/**
	*重启方舟,参数为服务器安装路径
	*/
	void arkRestart();

	/**
	*根据配置文件检查所有窗口
	*/
	void arkCheckWindows();

	/**
	*从网络更新方舟最新版本号
	*/
	void updateVersionFromUrl();

	/**
	*返回当前的网络版本号
	*/
	std::string getVersion() {
		return this->_netVersion;
	}

private:
	/**
	*read ark server version from installpath/version.txt
	*/
	std::string readVersion(const std::string &installpath);

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

