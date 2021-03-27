﻿#include"Class/app.h"
#include"Class/Sqlite3Operate.h"
bool g_bExit;
HANDLE g_hEvent = INVALID_HANDLE_VALUE;

BOOL CALLBACK CosonleHandler(DWORD ev)
{
	BOOL bRet = FALSE;
	switch (ev)
	{
		// the user wants to exit. 
	case CTRL_CLOSE_EVENT:
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_LOGOFF_EVENT:
		g_bExit = true;
		// 停住，让主线程执行清理任务
		WaitForSingleObject(g_hEvent, -1);
		// 注意事件在这里清理
		CloseHandle(g_hEvent);
		bRet = TRUE;
		break;
	default:
		break;
	}
	return bRet;
}



int main()
{
	//system("mode con cp select=65001");//默认utf-8显示
	//system("cls");

	SetConsoleCtrlHandler(CosonleHandler, TRUE);
	g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//程序运行
    auto app = ArkHelperServerAPP::create();
	if (app) {
		std::cout << "Init Succeed!" << std::endl;
		app->run(&g_bExit);
		delete(app);
	}
	else {
		std::cout << "Init Failed！See Log File to the detials." << std::endl;
		Sleep(2000);
	}
	//std::cout << CurlOperate::get("http://2021.ip138.com");
	
	SetEvent(g_hEvent);
	return 1;
}

/*
struct player {
	string steamid;
	int VIP;
	int points;
};

void getPointsWithVips() {
	Sqlite3Operate vipdata("C:/Users/gameserver/Desktop/ArkDB.db");
	Sqlite3Operate shopdata("C:/Users/gameserver/Desktop/ArkShop.db");
	std::vector<player> players;
	for (int i = 1; i < 1996; i++) {
		player p;
		auto k = vipdata.findOneData("Players", "SteamId", "Id", to_string(i));
		p.steamid = *k;
		delete(k);
		auto j = vipdata.findOneData("Players", "Groups", "Id", to_string(i));
		if (*j == "Default,")
			p.VIP = 0;
		else {
			for (int l = 1; l < 10; l++) {
				if (j->find(to_string(l), 0) != string::npos) {
					p.VIP = l;
					break;
				}
			}
		}
		players.push_back(p);
	}
	shopdata.doOneSqlStr("BEGIN TRANSACTION");
	for (auto& i : players) {
		auto k = shopdata.findOneData("Players", "Points", "SteamId", i.steamid);
		if (*k != "") {
			i.points = stoi(*k);
			int added = 2000;
			added += i.VIP * 1000;
			i.points += added;
			string sql = "update Players set Points = " + to_string(i.points) + " where SteamId = '" + i.steamid + "'";
			shopdata.doOneSqlStr(sql);
		}
		delete(k);
	};
	shopdata.doOneSqlStr("COMMIT");
	vipdata.~Sqlite3Operate();
}
*/
