#include "app.h"

ArkHelperServerAPP* ArkHelperServerAPP::impl = nullptr;

ArkHelperServerAPP* ArkHelperServerAPP::create()
{
	if (!impl) {
		auto ptr = new ArkHelperServerAPP();
		if(ptr&&ptr->init())
			ptr->impl = ptr;
	}
	return impl;
}

ArkHelperServerAPP::~ArkHelperServerAPP()
{
	//wait for input thread exit 
	while (true) {
		this->_exitMutex.lock();
		auto flag = this->_inputExit;
		this->_exitMutex.unlock();
		if (flag)break;
		Sleep(10);
	}
	this->impl = nullptr;
	delete(this->_appLog);
	DEBUGLOG("app destructure");
}

ArkHelperServerAPP::ArkHelperServerAPP()
	:_exitFlag(nullptr)
	, _inputExit(true)
	, _count(0)
	, _cmd("")
	, _cmdResult("")
	, _appLog(MyLog::Log::createLog("AppLog/AppLog"))
	, _rconConfig(new JsonOperate())
	, _frame(50)
	, _inputModeActive(false)
	, _workModeActive(false)
	, _keepWindowOpen(true)
	, _monitorKeep(true)
{
}

int ArkHelperServerAPP::run(bool *exit)
{

	this->_exitFlag = exit;
	bool exit_flag = false;

	if (this->_inputModeActive) {	//active input mode 开启输入模式

		this->_monitorKeep = false;
		std::thread input(&ArkHelperServerAPP::inputThread, this);//命令输入线程
		input.detach();

	}

	do {
		this->mainWork();
		
		//更新退出信号
		this->_exitMutex.lock();
		exit_flag = *(this->_exitFlag);
		this->_exitMutex.unlock();
	} while (!exit_flag);

	return 0;
}

bool ArkHelperServerAPP::init()
{

	if (!this->_rcon.init())return false;

	this->_rconConfig->openFile("Config.json");
	auto root = this->_rconConfig->getRoot();
	this->_inputModeActive = root["Mode"]["InputMode"].asBool();
	this->_workModeActive = root["Mode"]["WorkMode"].asBool();

	struct server{
		string name;
		string ip;
		u_short port = 0;
		string pass;
	};

	vector<server> servers;
	auto allservers = root["Servers"];

	for (auto &i : allservers) {

		server s;
		s.name = i["name"].asString();
		s.ip = root["IP"].asString();
		s.port = i["RconPort"].asInt();
		s.pass= i["AdminPass"].asString();
		servers.push_back(s);

	}

	for (auto &i : servers) {

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

void ArkHelperServerAPP::inputThread()
{
	this->_inputExit = false;
	bool exit_flag = false;
	std::string cmdStr = "";
	std::string cmdResult = "";

	while (!exit_flag)
	{

		cmdStr.clear();
		std::cout << "# ark->";
		CIN(cmdStr);
		this->_cmdQueueMutex.lock();
		this->_cmd = cmdStr;
		this->_cmdQueueMutex.unlock();
		DEBUGLOG("cmdStr:" + cmdStr);

		while (!exit_flag) {	//等待命令结果并显示

			this->_cmdResultMutex.lock();

			if (this->_cmdResult != "") {

				cmdResult = this->_cmdResult;
				this->_cmdResult = "";
				this->_cmdResultMutex.unlock();
				break;

			}
			else {

				this->_cmdResultMutex.unlock();
				Sleep(20);
				this->_exitMutex.lock();
				exit_flag = *(this->_exitFlag);
				this->_exitMutex.unlock();

			}

		}

		COUT(cmdResult);
		DEBUGLOG("cmdResult:" + cmdResult);
		cmdStr = "";

		//更新退出信号
		this->_exitMutex.lock();
		exit_flag = *(this->_exitFlag);
		this->_exitMutex.unlock();

	}

	//发送输入线程退出信号
	this->_exitMutex.lock();
	this->_inputExit = true;
	this->_exitMutex.unlock();
	DEBUGLOG("inputThread Exit");
}

void ArkHelperServerAPP::mainWork()
{
	clock_t start, ends;
	start = clock();

	if (this->_inputModeActive) this->solveInput();

	if (this->_workModeActive)this->work();

	this->_count++;

	if (this->_count % (3600 * 24 * this->_frame) == 0)this->_count = 0;

	ends = clock();

	unsigned long interval = 1000.0 / this->_frame;
	DEBUGLOG("Used " + to_string(ends - start) + "ms");

	if (ends - start < interval) {//完成时间小于20ms

		DEBUGLOG("Sleep for " + to_string(interval - (ends - start) - 1) + "ms");
		Sleep(interval - (ends - start) - 1);

	}

}

void ArkHelperServerAPP::work()
{
	DEBUGLOGFIN;

	if (this->_count % (this->_frame * 1 * 1) == 0) {	//每1秒执行一次


		this->drawState();

		if (this->_keepWindowOpen) {

			DEBUGLOG("restartAll");
			this->_update.arkRestart();

		}

		DEBUGLOG("clearRecv");
		this->_rcon.clearRecv();
		DEBUGLOG("checkCrashed");
		this->_update.checkCrashed();

	}
	if (this->_count % (this->_frame * 5 * 1) == 10) {	//每5秒执行一次

		DEBUGLOG("updateplayerlist");
		this->_rcon.updateplayerlist();

	}
	if (this->_count % (this->_frame * 10 * 1) == 20) {	//每10秒执行一次

		DEBUGLOG("reconnect");
		this->_rcon.reconnect();

	}
	if (this->_count % (this->_frame * 60 * 1) == 30) {	//每分钟执行一次

	}
	if (this->_count % (this->_frame * 60 * 10) == 40) {	//每10分钟执行一次

		DEBUGLOG("checkUpdate");
		this->_update.checkUpdate();

	}

	DEBUGLOGFRE;
}

void ArkHelperServerAPP::solveInput()
{
	string cmd = "";
	string cmdResult = "";
	this->_cmdQueueMutex.lock();
	cmd = this->_cmd;
	this->_cmd = "";
	this->_cmdQueueMutex.unlock();   

	if (!this->_workModeActive)this->_rcon.clearRecv();

	if (cmd == "") {

		return;

	}
	else if (cmd == "help") {

		cmdResult += "showonline--this cmd show you all online players\n"
			"broadcast--send a broadcast mseeage to all servers\n"
			"version--show the newest version of ARK Server\n"
			"ban--ban player\n"
			"unban--unban player\n"
			"shutdown--shutdown all server which is in Config.json\n"
			"restartall--restart all server(if not started,start server)\n"
			"update--update all server\n"
			"shopreload--reload all server's shop config(don't use if you have no shop plugin)\n"
			"state--show if server is connected with rcon\n"
			"kick--kick player from server\n"
			"reconnect--reconnect the server which is offline\n"
			"monitor--start monitor mode,input any error cmd will exit this mode\n"
			"exit--ues to exit this progrma\n";

	}
	else if (cmd == "exit") {

		this->_exitMutex.lock();
		*(this->_exitFlag) = true;
		this->_exitMutex.unlock();
		cmdResult += "EXIT!";

	}
	else if (cmd == "showonline") {

		if (!this->_workModeActive)this->_rcon.updateplayerlist();

		COUT("Whether you would like to know player's game character name? y/n");
		string yn = "";

		while (yn != "y"&&yn != "n")	CIN(yn);

		if (yn == "n") {

			cmdResult = "-------------------------------\n";

			for (auto &i : this->_rcon._server) {

				auto m = i->getPlayers();

				if (m.size() != 0) {

					cmdResult += i->getServerName() + "\n";
					for (auto &j : m)
						cmdResult += j.steamName + ":" + j.steamId + "\n";

				}

			}

			cmdResult += "-------------------------------\n";

		}
		else {

			this->_rcon.updateGameName();
			cmdResult = "-------------------------------\n";

			for (auto &i : this->_rcon._server) {

				auto m = i->getPlayers();

				if (m.size() != 0) {

					cmdResult += i->getServerName() + "\n";
					for (auto &j : m)
						cmdResult += "SteamName:" + j.steamName + "; " 
						+ "GameCharacterName:" + j.gameName + "; SteamId:" + j.steamId + "\n";
				
				}

			}

			cmdResult += "-------------------------------\n";

		}

	}
	else if (cmd == "broadcast") {

		COUT("Input what you want to send: ");
		string data = "";
 		CINUTF8(data); 
		this->_rcon.broadcast(data); 
		cmdResult += "Send OK!";

	}
	else if (cmd == "version") {

		if (!this->_workModeActive) {

			if (this->_update.checkUpdate()) {

				cmdResult += "Need Update!\n";

			}

			cmdResult += this->_update.getVersion();
		
		}
	}
	else if (cmd == "ban") {

		COUT("Input the steamid of who you want to ban:");
		string steamid;
		CIN(steamid);
		this->_rcon.kick(steamid);
		this->_rcon.sendCmdAndWiatForItRecv("banplayer " + steamid);
		cmdResult = "OK!";

	}
	else if (cmd == "unban") {

		COUT("Input the steamid of who you want to unban:");
		string steamid;
		CIN(steamid);
		this->_rcon.sendCmdAndWiatForItRecv("unbanplayer " + steamid);
		cmdResult = "OK!";

	}
	else if (cmd == "shutdown") {

		this->_keepWindowOpen = false;
		this->_update.closeAll();
		cmdResult += "OK!";

	}
	else if (cmd == "restartall") {

		this->_keepWindowOpen = true;
		this->_update.arkRestart();
		cmdResult += "OK!";

	}
	else if (cmd == "update") {

		this->_rcon.shutConnect();
		COUT(TimeClass().TimeNow() + "--Rcon connections have beed all shutdown!");
		this->_update.arkUpdate();
		COUT(TimeClass().TimeNow() + "--Update Finished!");
		cmdResult += "OK!";

	}
	else if (cmd == "shopreload") {

		this->_rcon.sendCmdAndWiatForItRecv("arkshop.reload");
		cmdResult += "OK!";

	}
	else if (cmd == "state") {

		auto state = this->_rcon.getState();
		int onlineNum = 0;

		cmdResult += "\nServers Online ";

		string offline;
		for (auto& i : *state) {

			if (i.second)onlineNum++;

		}

		cmdResult += to_string(onlineNum) + "/" + to_string(state->size()) + "\n\n";

		for (auto& i : *state) {

			if (i.second) {

				cmdResult += i.first + "--";
				cmdResult += "connected\n";

			}
			else {

				offline += i.first + "--";
				offline += "disconnected\n";

			}

		}

		if(offline!="")
			cmdResult += "OffLine\n" + offline;

		delete(state);
	}
	else if (cmd == "kick") {

		COUT("Input the player's steamid you want to kick:");
		string steamid;
		CIN(steamid);
		this->_rcon.kick(steamid);
		cmdResult += "OK!";

	}
	else if (cmd == "reconnect") {

		this->_rcon.reconnect();
		cmdResult += "OK!";

	}
	else if (cmd == "monitor") {

		this->_monitorKeep = true;
		cmdResult += "OK!";

	}
	else {

			this->_monitorKeep = false;

		cmdResult += "Error CMD! Input \"help\" for more CMD";

	}

	this->_cmdResultMutex.lock();
	this->_cmdResult = cmdResult;
	this->_cmdResultMutex.unlock();
}

void ArkHelperServerAPP::drawState()
{
	if (this->_inputModeActive && (!this->_workModeActive))return;

	if (!this->_monitorKeep)return;

	string ui;
	for (auto& i: this->_rcon._server){

		if (i->connectedState()) {


			ui += "------" + i->getServerName();
			auto len = 30 - ui.size();
			for (size_t j = 0; j < len; j++) {
				ui += "-";
			}
			ui += "Online";
			len = 50 - ui.size();
			for (size_t j = 0; j < len; j++) {
				ui += "-";
			}
			ui = "\033[1;32;40m" + ui;
			ui += "\n\033[0m";

			for (auto& j : i->getPlayers()) {

				ui += j.steamId + "-----" + j.steamName + "\n";

			}

			ui += "\033[1;32;40m------------------------------\033[0m\n\n";

		}
		else {


			ui += "------" + i->getServerName();
			auto len = 30 - ui.size();
			for (size_t j = 0; j < len; j++) {
				ui += "-";
			}
			ui += "Offline";
			len = 50 - ui.size();
			for (size_t j = 0; j < len; j++) {
				ui += "-";
			}
			ui += "\033[0m\n\n";
			ui = "\033[1;31;40m" + ui;

		}
		
	}

	::system("cls");
	COUT(ui);

}

