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
		HWND hwnd;	//���ھ��
		std::string version;	//���ڵķ������汾��
		std::string name;	//����������
		std::string path;	//��������װ·��
		std::string map;
		std::string listenPort;	
		std::string queryPort;
		std::string rconPort;
		bool isonline;	//�������Ƿ�����
	};

public:
	/**
	*check if server need to update
	*return true if server need update
	*/
	bool needForUpdate();

	/**
	*ö�ٴ����ҵ����з��������ں�õ��䴰�ھ��
	*/
	static BOOL CALLBACK EnumWindowsCallBack(HWND hwnd, LPARAM lParam);

	/**
	*�鿴�Ƿ���crashed���� 
	*/
	bool checkCrashed();
	/**
	*update all servers if the server is needed
	*/
	void arkUpdate();

	/**
	*ͨ�����ھ���رշ��۷���������
	*/
	bool closeArkWindow(HWND);

	/**
	*��������,����Ϊ��������װ·��
	*/
	void arkRestart();

	/**
	*���������ļ�������д���
	*/
	void arkCheckWindows();

	/**
	*��������·������°汾��
	*/
	void updateVersionFromUrl();

	/**
	*���ص�ǰ������汾��
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
	//������־
	MyLog::Log* _updateLog;

	//�������°汾
	std::string _netVersion;

	//���µ��������json
	JsonOperate* _arkJson;

	//���۷���˴������Ƽ�����
	std::vector<ArkServer> _arkServerWindow;

};

