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
	while (true) {
		this->_exitMutex.lock();
		auto flag = this->_inputExit;
		this->_exitMutex.unlock();
		if (!flag)break;
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
		input_exit = this->_inputExit;
		this->_exitMutex.unlock();
	} while (!exit_flag&&!input_exit);

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
	auto namelist = root["Server"].getMemberNames();
	for (auto &i : namelist) {
		server s;
		s.name = i;
		s.ip = root["IP"].asString();
		s.port = root["Server"][i]["Port"].asInt();
		s.pass= root["Server"][i]["Pass"].asString();
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
}

void ArkHelperServerAPP::mainWork()
{
	
	this->solveInput();
	if (this->_count % (50 * 10 * 1) == 0) {	//每10秒执行一次
		
	}
	if (this->_count % (50 * 5 * 1) == 0) {	//每5秒执行一次
		this->_rcon.update();
	}
	if (this->_count % (50 * 1 * 1) == 0) {	//每1秒执行一次

	}
	if (this->_count % (50 * 60 * 1) == 0) {	//每分钟执行一次
		
	}
	this->_count++;
	Sleep(20);
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
			"exit--ues to exit this progrma\n"
			"version--show the newest version of ARK Server\n"
			"ban--ban player\n"
			"unban--unban player\n";
	}
	else if (cmd == "exit") {
		this->_exitMutex.lock();
		*(this->_exitFlag) = true;
		this->_exitMutex.unlock();
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
		string data1 = "这是一个测试公告";
		string data2 = MyLog::GBKtoUTF8(data1);
		CINUTF8(data);
		string data4 = MyLog::GBKtoUTF8(data);
		this->_rcon.broadcast(data); 
		cmdResult = "Send OK!";
	}
	else if (cmd == "version") {
		this->_update.updateVersionFromUrl();
		cmdResult = this->_update.getVersion();
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
	else {
		cmdResult = "error CMD! Input \"help\" for more CMD";
	}

	this->_cmdResultMutex.lock();
	this->_cmdResult = cmdResult;
	this->_cmdResultMutex.unlock();
}

