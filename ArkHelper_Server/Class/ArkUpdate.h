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
		HWND hwnd;	//���ھ��
		std::string version;	//���ڵķ������汾��
		std::string name;	//����������
		std::string path;	//��������װ·��
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
	* check for if any server crashed with UE4��if there is a server crashed, auto close it
	* �鿴�Ƿ���crashed����,����У���رոô���ʹ���Զ����� 
	*/
	bool checkCrashed();

	/**
	* update all servers if the server is needed
	* �������з�����
	*/
	void arkUpdate();

	/**
	* close all ark window ;
	* �ر����з��۴���
	*/
	void closeAll();

	/**
	* restart all ark server
	* ��������,����Ϊ��������װ·��
	*/
	void arkRestart();

	/**
	* get version number
	* ���ص�ǰ������汾��
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
	* ��������·������°汾��
	*/
	void updateVersionFromUrl();

	/**
	* read ark server version from installpath/version.txt
	* ��ȡ���۷������İ汾�ţ�����Ϊconfig�е�path(��������װ·��)
	*/
	std::string readVersion(const HWND& hwnd);

	/**
	* use handle to close an ark server window
	* ͨ�����ھ���رշ��۴��ڣ��ڴ��ڳ��׹ر�ǰ����
	*/
	bool closeArkWindow(HWND);

	/**
	* check ang update all ark server window hwnd
	* ���������ļ�������д���
	*/
	void arkCheckWindows();


private:
	//������־
	MyLog::Log* _updateLog;

	//�������°汾
	std::string _netVersion;

	//���µ��������json
	JsonOperate* _arkJson;

	//���۷���˴������Ƽ�����
	std::vector<ArkServer> _arkServerWindow;


};

