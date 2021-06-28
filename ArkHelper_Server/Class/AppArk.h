#pragma once
#include"ArkRcon.h"
#include"ArkUpdate.h"
#include"ArkModsUpdate.h"
#include"CmdLineApplicationBase.h"


#define COUT(str) std::cout<<MyLog::UTF8toGBK(str)<<std::endl;
//#define COUT(str) std::cout<<str<<std::endl;
#define CIN(str) std::cin>>str;
#define CINUTF8(str) std::cin>>str;str=MyLog::GBKtoUTF8(str);

class AppArk :public CmdLineApplicationBase {
public:
	AppArk();

	~AppArk();
public:
protected:
	virtual void solveInput(const std::string& cmd) override;

private:
	bool init();
	void addWorkFunc();
	void drawState();
private:
	void every1sec();
	void every5sec();
	void every10sec();
	void every1min();
	void every5min();
	void every10min();
	void checkServerUpdate();
	void checkModsUpdate();
	void modsServerConnect();

	/*
	* һ��ʱ����͹��棬timeΪ����
	*/
	void broadcastInTime(std::string msg, int time);
private:
	//��־
	MyLog::Log* _appLog;

	//RconConfig
	JsonOperate* _rconConfig;

	//language pack
	Json::Value _text;

	//RconServer
	ArkRcon _rcon;

	//���۸���
	ArkUpdate* _update;

	//mods����
	ArkModsUpdate* _modupdate;

	bool _remoteModeActive;
	bool _keepWindowOpen;	//���ַ����������������Զ�������
	bool _monitorKeep;		//������ģʽ������־
};