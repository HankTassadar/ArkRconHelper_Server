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
}

ArkHelperServerAPP::ArkHelperServerAPP()
	:_exitFlag(nullptr)
	, _inputExit(false)
	, _count(0)
	, _cmd("")
	, _cmdResult("")
	,_appLog(MyLog::Log::createLog("AppLog/AppLog"))
	,_rconConfig(new JsonOperate())
{
}

int ArkHelperServerAPP::run(bool *exit)
{
	this->_exitFlag = exit;
	bool exit_flag = false;
	bool input_exit = false;

	std::thread input(&ArkHelperServerAPP::inputThread, this);//命令输入线程
	input.detach();

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
		if (flag)this->_appLog->logoutUTF8(TimeClass().TimeNow() + " " + i.name + "--connected succeed!");
		else {
			this->_appLog->logoutUTF8(TimeClass().TimeNow() + " " + i.name + "--connected faild!");
			COUT(i.name + "--connected failed!");
		}
	}
	return true;
}

void ArkHelperServerAPP::inputThread()
{
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
		while (true) {	//等待命令结果并显示
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
			}
		}
		COUT(cmdResult);
		DEBUGLOG("cmdResult:" + cmdResult);
		//更新退出信号
		this->_exitMutex.lock();
		exit_flag = *(this->_exitFlag);
		this->_exitMutex.unlock();
		cmdStr = "";
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

	this->solveInput();

	if (this->_count % (50 * 1 * 1) == 0) {	//每1秒执行一次

		DEBUGLOG("clearRecv");
		this->_rcon.clearRecv();
		DEBUGLOG("checkCrashed");
		this->_update.checkCrashed();

	}
	if (this->_count % (50 * 10 * 1) == 0) {	//每10秒执行一次

		DEBUGLOG("reconnect");
		this->_rcon.reconnect();

	}
	if (this->_count % (50 * 5 * 1) == 0) {	//每5秒执行一次

		DEBUGLOG("updateplayerlist");
		this->_rcon.updateplayerlist();

	}
	if (this->_count % (50 * 60 * 1) == 0) {	//每分钟执行一次
				
	}
	if (this->_count % (50 * 60 * 10) == 0) {	//每10分钟执行一次

		DEBUGLOG("checkUpdate");
		this->_update.checkUpdate();

	}

	this->_count++;

	if (this->_count % (3600 * 50) == 0)this->_count = 0;

	ends = clock();

	if (ends - start < 20) {//完成时间小于20ms

		Sleep(ends - start - 1);

	}

}

void ArkHelperServerAPP::solveInput()
{
	string cmd = "";
	string cmdResult = "";
	this->_cmdQueueMutex.lock();
	cmd = this->_cmd;
	this->_cmd = "";
	this->_cmdQueueMutex.unlock();   

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
			"exit--ues to exit this progrma\n";

	}
	else if (cmd == "exit") {

		this->_exitMutex.lock();
		*(this->_exitFlag) = true;
		this->_exitMutex.unlock();
		cmdResult += "EXIT!";

	}
	else if (cmd == "showonline") {

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

		cmdResult += this->_update.getVersion();

	}
	else if (cmd == "ban") {

		COUT("Input the steamid of who you want to ban:");
		string steamid;
		CIN(steamid);
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

		this->_update.closeAll();
		cmdResult += "OK!";

	}
	else if (cmd == "restartall") {

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
	else {

		cmdResult = "error CMD! Input \"help\" for more CMD";

	}

	this->_cmdResultMutex.lock();
	this->_cmdResult = cmdResult;
	this->_cmdResultMutex.unlock();
}

