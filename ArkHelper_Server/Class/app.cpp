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

	auto root = this->_rconConfig->getRoot();
	string language = root["Language"]["Language"].asString();
	auto text = root["Language"][language];
	if (cmd == "") {

		return;

	}
	else if (cmd == "help") {

		for (auto& i : text["help"]) {

			cmdResult += i.asString();

		}

	}
	else if (cmd == "exit") {

		this->_exitMutex.lock();
		*(this->_exitFlag) = true;
		this->_exitMutex.unlock();
		cmdResult += "EXIT!";

	}
	else if (cmd == "showonline") {

		if (!this->_workModeActive)this->_rcon.updateplayerlist();

		COUT(text["showonline"][0].asString());
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

		COUT(text["broadcast"][0].asString());
		string data = "";
 		CINUTF8(data); 
		this->_rcon.broadcast(data); 
		cmdResult += "Send OK!";

	}
	else if (cmd == "version") {

		bool needupdate = false;

		if (!this->_workModeActive) {

			needupdate = this->_update.checkUpdate();
		
		}

		if (needupdate) {

			cmdResult += text["version"][0].asString();

		}
		
		cmdResult += this->_update.getVersion();

	}
	else if (cmd == "ban") {

		COUT(text["ban"][0].asString());
		string steamid;
		CIN(steamid);
		this->_rcon.kick(steamid);
		this->_rcon.sendCmdAndWiatForItRecv("banplayer " + steamid);
		cmdResult = "OK!";

	}
	else if (cmd == "unban") {

		COUT(text["unban"][0].asString());
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
		COUT(TimeClass().TimeNow() + text["update"][0].asString());
		this->_update.arkUpdate();
		COUT(TimeClass().TimeNow() + text["update"][1].asString());
		cmdResult += "OK!";

	}
	else if (cmd == "shopreload") {

		this->_rcon.sendCmdAndWiatForItRecv("arkshop.reload");
		cmdResult += "OK!";

	}
	else if (cmd == "state") {

		auto state = this->_rcon.getState();
		int onlineNum = 0;

		cmdResult += "\n";
		cmdResult += text["state"][0].asString();

		string offline;
		for (auto& i : *state) {

			if (i.second)onlineNum++;

		}

		cmdResult += to_string(onlineNum) + "/" + to_string(state->size()) + "\n\n";

		for (auto& i : *state) {

			if (i.second) {

				cmdResult += i.first + " -- ";
				cmdResult += "\033[1;32;40m" + string(text["state"][1].asString().c_str()) + "\033[0m\n";

			}
			else {

				cmdResult += i.first + " -- ";
				cmdResult += "\033[1;31;40m" + string(text["state"][2].asString().c_str()) + "\033[0m\n";

			}

		}

		delete(state);
	}
	else if (cmd == "kick") {

		COUT(text["kick"][0].asString());
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

		cmdResult += text["error"][0].asString();

	}

	this->_cmdResultMutex.lock();
	this->_cmdResult = cmdResult;
	this->_cmdResultMutex.unlock();
}

void ArkHelperServerAPP::drawState()
{
	DEBUGLOGFIN;

	if (this->_inputModeActive && (!this->_workModeActive))return;

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

	for (auto& i: this->_rcon._server){

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

