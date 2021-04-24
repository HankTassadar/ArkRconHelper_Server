#include"AppArk.h"
#include<algorithm>

AppArk::AppArk()
	: CmdLineApplicationBase("# ark->", 20)
	, _appLog(MyLog::Log::createLog("AppLog/AppLog"))
	, _rconConfig(new JsonOperate())
	, _update(nullptr)
	, _modupdate(nullptr)
	, _remoteModeActive(false)
	, _keepWindowOpen(true)
	, _monitorKeep(false)
{

	if (this->init()) {

		std::cout << "\033[1;32;40mInit Succeed!\033[0m\n" << std::endl;
		this->addWorkFunc();

	}
	else {

		std::cout << "\n\033[1;31;40mInit Failed！See Log File to the detials.\033[0m\n" << std::endl;
		Sleep(2000);
		this->setMainExitFlag();

	}
	
}

AppArk::~AppArk()
{
	delete(this->_appLog);
	delete(this->_rconConfig);
	if (!this->_remoteModeActive) {
		delete(this->_update);
		delete(this->_modupdate);
	}
}

void AppArk::solveInput(const std::string& cmd)
{
	if (cmd == "")return;

	if (cmd == "exit") {

		COUT("EXIT!");
		this->setMainExitFlag();
		return;

	}

	if (cmd == "help") {

		string result;

		for (auto& i : this->_text["help"]) {

			result += i.asString();

		}

		COUT(result);
		return;

	}

	if (cmd == "showonline") {

		string cmdResult;

		COUT(this->_text["showonline"][0].asString());
		string yn = "";

		while (yn != "y" && yn != "n")	CIN(yn);

		if (yn == "n") {

			cmdResult = "-------------------------------\n";

			for (auto& i : this->_rcon._server) {

				auto m = i->getPlayers();

				if (m.size() != 0) {

					cmdResult += i->getServerName() + "\n";
					for (auto& j : m)
						cmdResult += j.steamName + ":" + j.steamId + "\n";

				}

			}

			cmdResult += "-------------------------------\n";

		}
		else {

			this->_rcon.updateGameName();
			cmdResult = "-------------------------------\n";

			for (auto& i : this->_rcon._server) {

				auto m = i->getPlayers();

				if (m.size() != 0) {

					cmdResult += i->getServerName() + "\n";
					for (auto& j : m)
						cmdResult += "SteamName:" + j.steamName + "; "
						+ "GameCharacterName:" + j.gameName + "; SteamId:" + j.steamId + "\n";

				}

			}

			cmdResult += "-------------------------------\n";

		}

		COUT(cmdResult);
		return;

	}

	if (cmd == "broadcast") {

		COUT(this->_text["broadcast"][0].asString());
		string data = "";
		CINUTF8(data);
		this->_rcon.broadcast(data);
		COUT("OK!");
		return;

	}

	if (cmd == "ban") {

		COUT(this->_text["ban"][0].asString());
		string steamid;
		CIN(steamid);
		this->_rcon.kick(steamid);
		this->_rcon.sendCmdAndWiatForItRecv("banplayer " + steamid);
		COUT("OK!");
		return;

	}

	if (cmd == "unban") {

		COUT(this->_text["unban"][0].asString());
		string steamid;
		CIN(steamid);
		this->_rcon.sendCmdAndWiatForItRecv("unbanplayer " + steamid);
		COUT("OK!");
		return;

	}

	if (cmd == "shopreload") {

		this->_rcon.sendCmdAndWiatForItRecv("arkshop.reload");
		COUT("OK!");
		return;

	}

	if (cmd == "state") {

		DEBUGLOG("state");
		string cmdResult;
		auto state = this->_rcon.getState();
		int onlineNum = 0;

		cmdResult += "\n";
		cmdResult += this->_text["state"][0].asString();

		string offline;
		for (auto& i : *state) {

			if (i.second)onlineNum++;

		}

		cmdResult += to_string(onlineNum) + "/" + to_string(state->size()) + "\n\n";

		for (auto& i : *state) {

			if (i.second) {

				cmdResult += "\033[0m " + i.first + " -- ";
				cmdResult += "\033[1;32;40m " + this->_text["state"][1].asString() + " \n";

			}
			else {

				cmdResult += "\033[0m " + i.first + " -- ";
				cmdResult += "\033[1;31;40m " + this->_text["state"][2].asString() + " \n";

			}

		}
		cmdResult += "\033[0m";

		delete(state);
		COUT(cmdResult);
		return;

	}

	if (cmd == "kick") {

		COUT(this->_text["kick"][0].asString());
		string steamid;
		CIN(steamid);
		this->_rcon.kick(steamid);
		COUT("OK!");
		return;

	}

	if (cmd == "reconnect") {

		this->_rcon.reconnect();
		COUT("OK!");
		return;

	}

	if (cmd == "monitor") {

		this->_monitorKeep = true;
		COUT("OK!");
		return;

	}



	if (!this->_remoteModeActive) {

		if (cmd == "modupdate") {

			auto updatetime = this->_modupdate->getUpdateTime();
			string str;
			sort(updatetime.begin(), updatetime.end()
				, [&](const pair<string, time_t>& param1, const pair<string, time_t>& param2)->bool {
					return param1.second > param2.second;
				});
			for (auto& i : updatetime) {
				str += i.first + ": " + TimeClass(i.second).TimeNow() + "\n";
			}
			COUT(str);
			COUT("OK!");
			return;

		}

		if (cmd == "updatemod") {

			COUT(this->_text["updatemod"][0].asString());
			string modid;
			CIN(modid);

			if (this->_modupdate->updateServerRun(modid)) {

				this->addWork(time(NULL) + 60, [&]() {this->modsServerConnect(); });

			}
			else {

				COUT(this->_text["updatemod"][1].asString());

			}

			COUT("OK!");
			return;

		}

		if (cmd == "update") {

			this->_rcon.shutConnect();
			COUT(TimeClass().TimeNow() + this->_text["update"][0].asString());
			this->_update->arkUpdate();
			COUT(TimeClass().TimeNow() + this->_text["update"][1].asString());
			COUT("OK!");
			return;

		}

		if (cmd == "restartall") {

			this->_keepWindowOpen = true;
			this->_update->arkRestart();
			COUT("OK!");
			return;

		}

		if (cmd == "shutdown") {

			this->_keepWindowOpen = false;
			this->_update->closeAll();
			COUT("OK!");
			return;

		}

		if (cmd == "version") {

			string cmdResult;
			bool needupdate = false;

			needupdate = this->_update->checkUpdate();

			if (needupdate) {

				cmdResult += this->_text["version"][0].asString();

			}


			cmdResult += "NetVersion: " + this->_update->getVersion() + "\n";
			for (auto& i : this->_update->getServer()) {
				cmdResult += i.name + "--" + i.version + "\n";
			}

			COUT(cmdResult);
			return;

		}
	}

	this->_monitorKeep = false;//监控模式下退出监控模式

	COUT(this->_text["error"][0].asString());


}

bool AppArk::init()
{
	if (!this->_rcon.init())return false;

	this->_rconConfig->openFile("Config.json");
	auto root = this->_rconConfig->getRoot();
	string language = root["Language"]["Language"].asString();
	this->_text = root["Language"][language];
	this->_remoteModeActive = root["Mode"]["RemoteMode"].asBool();

	if (!this->_remoteModeActive) {
		this->_update = new ArkUpdate();
		this->_modupdate = new ArkModsUpdate();
	}
	struct server {
		string name;
		string ip;
		u_short port = 0;
		string pass;
	};

	vector<server> servers;
	auto allservers = root["Servers"];
	this->_appLog->logoutUTF8("ServerNum=" + to_string(allservers.size()));
	for (auto& i : allservers) {

		server s;
		s.name = i["name"].asString();
		s.ip = root["IP"].asString();
		s.port = i["RconPort"].asInt();
		s.pass = i["AdminPass"].asString();
		this->_appLog->logoutUTF8("Server add" + s.name + s.ip);
		servers.push_back(s);

	}

	for (auto& i : servers) {

		auto flag = this->_rcon.addServer(Rcon_addr{ i.name,i.ip,i.port,i.pass });
		if (flag)
			this->_appLog->logoutUTF8(TimeClass().TimeNow() + " " + i.name + "--connected succeed!");
		else {

			this->_appLog->logoutUTF8(TimeClass().TimeNow() + " " + i.name + "--connected faild!");
			COUT(i.name + "--connected failed!");

		}

	}

	return true;
}

void AppArk::addWorkFunc()
{
	this->addWork(time(NULL), [&]() {this->every1sec(); });
	this->addWork(time(NULL), [&]() {this->every5sec(); });
	this->addWork(time(NULL), [&]() {this->every10sec(); });
	this->addWork(time(NULL), [&]() {this->every1min(); });
	this->addWork(time(NULL), [&]() {this->every5min(); });
	this->addWork(time(NULL), [&]() {this->every10min(); });
	if (this->_rconConfig->getRoot()["Mode"]["AutoUpdateServer"].asBool() == true) {
		this->addWork(time(NULL), [&]() {this->checkServerUpdate(); });
	}

	if (this->_rconConfig->getRoot()["Mode"]["AutoUpdateMods"].asBool() == true) {
		this->addWork(time(NULL) + 600, [&]() {this->checkModsUpdate(); });
	}
}

void AppArk::drawState()
{
	DEBUGLOGFIN;

	if (!this->_monitorKeep)return;

	DEBUGLOG("start draw");
	string ui;
	size_t namelen = 0;

	for (auto& i : this->_rcon._server) {

		if (i->getServerName().size() > namelen)
			namelen = i->getServerName().size();

	}

	DEBUGLOG("namelen = " + to_string(namelen));

	namelen = (namelen / 10 + 1) * 10;

	DEBUGLOG("namelen = " + to_string(namelen));

	for (auto& i : this->_rcon._server) {

		string line = "";

		if (i->connectedState()) {


			line += "------" + i->getServerName();

			DEBUGLOG("line.size() = " + to_string(line.size()));
			auto len = namelen + 10 - line.size();

			DEBUGLOG("len = " + to_string(len));

			for (size_t j = 0; j < len; j++) {

				line += "-";

			}

			line += "Online";
			len = namelen + 30 - line.size();

			DEBUGLOG("len = " + to_string(len));

			for (size_t j = 0; j < len; j++) {

				line += "-";

			}

			line = "\033[1;32;40m" + line;
			line += "\n\033[0m";
			ui += line;

			for (auto& j : i->getPlayers()) {

				ui += j.steamId + "-----" + j.steamName + "\n";

			}

			ui += "\033[1;32;40m---------------------------------------------\033[0m\n\n";

		}
		else {


			line += "------" + i->getServerName();
			auto len = namelen + 10 - line.size();

			DEBUGLOG("len = " + to_string(len));

			for (size_t j = 0; j < len; j++) {

				line += "-";

			}

			line += "Offline";
			len = namelen + 30 - line.size();

			DEBUGLOG("len = " + to_string(len));

			for (size_t j = 0; j < len; j++) {

				line += "-";

			}

			line += "\033[0m\n\n";
			line = "\033[1;31;40m" + line;
			ui += line;
		}

	}

	::system("cls");
	COUT(ui);
	DEBUGLOGFRE;
}

void AppArk::every1sec()
{
	this->drawState();

	if (this->_keepWindowOpen) {

		DEBUGLOG("clearRecv");
		RELEASELOG("clearRecv");
		this->_rcon.clearRecv();

		if (!this->_remoteModeActive) {

			DEBUGLOG("restartAll");
			RELEASELOG("restartAll");
			this->_update->arkRestart();

			RELEASELOG("cheakCrashed");
			DEBUGLOG("checkCrashed");
			this->_update->checkCrashed();

		}

	}

	RELEASELOG("every1sec-over");
	this->addWork(time(NULL) + 1, [&]() {this->every1sec(); });
}

void AppArk::every5sec()
{
	RELEASELOG("updateplayerslist");
	DEBUGLOG("updateplayerlist");
	this->_rcon.updateplayerlist();
	RELEASELOG("every5sec-over");
	this->addWork(time(NULL) + 5, [&]() {this->every5sec(); });
}

void AppArk::every10sec()
{
	if (this->_keepWindowOpen) {
		RELEASELOG("reconnect");
		DEBUGLOG("reconnect");
		this->_rcon.reconnect();
	}
	RELEASELOG("every10sec-over");
	this->addWork(time(NULL) + 10, [&]() {this->every10sec(); });
}

void AppArk::every1min()
{
	this->addWork(time(NULL) + 60, [&]() {this->every1min(); });
}

void AppArk::every5min()
{
	this->addWork(time(NULL) + 300, [&]() {this->every5min(); });
}

void AppArk::every10min()
{
	this->addWork(time(NULL) + 600, [&]() {this->every10min(); });
}

void AppArk::checkServerUpdate()
{
	if (this->_remoteModeActive)return;

	RELEASELOG("checkUpdate");
	
	DEBUGLOG("checkUpdate");
	if (this->_update->checkUpdate()) {

		for (unsigned long i = 0; i < 5; i++) {

			this->addWork(time(NULL) + i * 60, [=]() {
				auto str = this->_text["update"][2].asString() + to_string(5 - i) + this->_text["update"][3].asString();
				this->_appLog->logoutUTF8(TimeClass().TimeNow() + "broadcast: " + str);
				this->_rcon.broadcast(str);
				});


		}

		this->addWork(time(NULL) + 300, [=]() {
			this->_rcon.shutConnect();
			this->_appLog->logoutUTF8(TimeClass().TimeNow() + "start auto update");
			this->_update->arkUpdate();
			this->addWork(time(NULL) + 600, [=]() {this->checkServerUpdate(); });
			});
	}
	else {
		this->addWork(time(NULL) + 600, [=]() {this->checkServerUpdate(); });
	}
	RELEASELOG("every10min-over");
}

void AppArk::checkModsUpdate()
{
	if (this->_remoteModeActive)return;

	RELEASELOG("checkModeUpdate");
	DEBUGLOG("checkModsUpdate");
	if (this->_rconConfig->getRoot()["Mode"]["AutoUpdateMods"].asBool() 
		&& this->_modupdate->checkUpdate()) {
		this->_appLog->logoutUTF8(TimeClass().TimeNow() + "--mods start update");
		this->_modupdate->updateServerRun();
		this->_appLog->logoutUTF8(TimeClass().TimeNow() + "--mods start update over,start to connect");
		this->addWork(time(NULL) + 60, [&]() {this->modsServerConnect(); });
	}
	else {
		this->addWork(time(NULL) + 600, [&]() {this->checkModsUpdate(); });
	}
	RELEASELOG("every10min_1-over");
}

void AppArk::modsServerConnect()
{
	RELEASELOG("modsServerConnect");
	if (this->_modupdate->connectServer()) {
		this->_appLog->logoutUTF8(TimeClass().TimeNow() + "--mods update finished");
		auto modid = this->_modupdate->shutdownUpdateServer();
		this->_update->shutdownAndModsUpdate(modid);
		this->addWork(time(NULL) + 600, [&]() {this->checkModsUpdate(); });
	}
	else {
		this->addWork(time(NULL) + 10, [&]() {this->modsServerConnect(); });
	}
	RELEASELOG("modsServerConnect-over");
}
