#include"ArkModsUpdate.h"
#include"JsonOperate.h"
#include"CurlOperate.h"
#include<algorithm>
#include"TimeClass.h"
using namespace std;

ArkModsUpdate::ArkModsUpdate()
	: _log(MyLog::Log::createLog("AppLog/ModUpdateLog"))
{
	this->init();
}

ArkModsUpdate::~ArkModsUpdate()
{
	delete(this->_log);
}

bool ArkModsUpdate::checkUpdate()
{
	vector<pair<string, string>> param;
	param.push_back({ "itemcount", to_string(this->_server.mods.size()) });

	int num = 0;
	for (auto& i : this->_server.mods)
		param.push_back({ "publishedfileids[" + to_string(num++) + "]",i });

	auto response = CurlOperate::post(param, "http://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/v1/?");
	auto rejson = new JsonOperate();
	rejson->openStr(response);
	auto modsinfo = rejson->getRoot()["response"]["publishedfiledetails"];
	
	for (auto& i : modsinfo) {
		auto key = i["publishedfileid"].asString();
		if (this->_mods[key].second == 0) {
			this->_mods[key].first = i["time_updated"].asLargestUInt();
			this->_mods[key].second = i["time_updated"].asLargestUInt();
		}
		else {
			this->_mods[key].second = this->_mods[key].first;
			this->_mods[key].first = i["time_updated"].asLargestUInt();
		}
	}
	
	bool re = false;
	for (auto& i : this->_mods) {
		if (i.second.first != i.second.second) {
			this->_log->logoutUTF8(TimeClass().TimeNow() + i.first + "update");
			re = true;
			break;
		}		
	}

	return re;
}

bool ArkModsUpdate::updateServerRun(std::string modid)
{
	this->_log->logoutUTF8(TimeClass().TimeNow() + "start update server");
	if (modid != "" && 
		count_if(this->_mods.begin(), this->_mods.end(), [=](decltype(*(this->_mods.begin())) mod)->bool {return mod.first == modid; }) != 1)
		return false;
	
	//write the mods which is needed update to Game.ini
	ofstream gameinifile(this->_server.path + "/ShooterGame/Saved/Config/WindowsServer/Game.ini", std::ios::app);
	if (!gameinifile.good()) {
		this->_log->logoutUTF8("Open Game.ini Failed!");
		return false;
	}

	gameinifile << "\n[ModInstaller]" << endl;
	
	if (modid == "") {
		for (auto& i : this->_mods) {
			if (i.second.first != i.second.second) {
				this->_log->logoutUTF8(i.first + "add in gameini");
				gameinifile << "ModIDS=" + i.first << endl;
			}
		}
	}
	else {
		gameinifile << "ModIDS=" + modid << endl;
	}

	gameinifile.close();

	system(this->_startCmd.c_str());
	while (this->_server.hwnd == NULL) {
		Sleep(10);
		EnumWindows([](HWND hwnd, LPARAM lParam)->BOOL {
			auto ptr = (ArkModsUpdate*)lParam;
			LPSTR a = (LPSTR)new char[256];
			::memset(a, 0, 256);
			GetWindowTextA(hwnd, a, 256);
			std::string name(a);
			if (name.find("ShooterGameServer.exe", 0) != string::npos) {
				if (name.find(ptr->_server.name) != string::npos) {
					ptr->setHwnd(hwnd);	
				}
			}
			delete[](a);
			return TRUE;
			}, (LPARAM)this);
	}
	return true;
}

bool ArkModsUpdate::connectServer()
{
	return this->_connection.init();
}

std::vector<std::string> ArkModsUpdate::shutdownUpdateServer()
{
	this->_connection.shutConnect();

	EnumWindows([](HWND hwnd, LPARAM lParam)->BOOL {
		auto ptr = (ArkModsUpdate*)lParam;
		LPSTR a = (LPSTR)new char[256];
		::memset(a, 0, 256);
		GetWindowTextA(hwnd, a, 256);
		std::string name(a);
		if (name.find("ShooterGameServer.exe", 0) != string::npos) {
			if (name.find(ptr->_server.name) != string::npos) {
				ptr->setHwnd(hwnd);
			}
		}
		delete[](a);
		return TRUE;
		}, (LPARAM)this);

	SendNotifyMessage(this->_server.hwnd, WM_CLOSE, 0, 0);
	while (true) {	//wait for close finish
		LPSTR winname = (LPSTR)new char[256];
		::memset(winname, 0, 256);
		GetWindowTextA(this->_server.hwnd, winname, 256);
		string name(winname);
		this->_log->logoutUTF8(TimeClass().TimeNow() + "-close window find-" + name);
		delete[](winname);
		Sleep(20);
		if (name == "")
			break;
	}

	fstream readfile(this->_server.path + "/ShooterGame/Saved/Config/WindowsServer/Game.ini");
	stringstream buffer;
	string dataStr;
	buffer << readfile.rdbuf();
	dataStr = buffer.str();
	readfile.close();

	dataStr = dataStr.substr(0, dataStr.find("\n[ModInstaller]"));
	std::ofstream writefile(this->_server.path + "/ShooterGame/Saved/Config/WindowsServer/Game.ini"
		, std::ios::out | std::ios::trunc);

	writefile << dataStr;
	writefile.close();

	std::vector<std::string> updatemodlist;

	for (auto& i : this->_mods) {
		if (i.second.first != i.second.second)
			updatemodlist.push_back(i.first);
	}

	return updatemodlist;
}

void ArkModsUpdate::setUpdateDown()
{
	for (auto& i : this->_mods) {
		i.second.second = i.second.first;
	}
}

std::vector<std::pair<std::string, time_t>> ArkModsUpdate::getUpdateTime()
{

	vector<pair<string, time_t>> re;
	for (auto& i : this->_mods) {

		re.push_back({ i.first,i.second.second });

	}

	return re;
}

void ArkModsUpdate::setHwnd(HWND hwnd)
{
	this->_server.hwnd = hwnd;
	this->_log->logoutUTF8(TimeClass().TimeNow() + "-mod server window find-" + to_string((int)hwnd));
}

bool ArkModsUpdate::init()
{
	auto config = new JsonOperate();
	config->openFile("Config.json");
	auto modsupdateserver = config->getRoot()["ModsUpdateServer"];

	for (auto& i : modsupdateserver["Mods"])
		this->_server.mods.push_back(i.asString());

	this->_server.hwnd = NULL;
	this->_server.name = modsupdateserver["ServerName"].asString();
	this->_server.pass = modsupdateserver["AdminPass"].asString();
	this->_server.path = modsupdateserver["ServerPath"].asString();
	this->_server.port = modsupdateserver["Port"].asString();
	this->_server.queryport = modsupdateserver["QueryPort"].asString();
	this->_server.rconport = modsupdateserver["RconPort"].asString();
	
	this->_startCmd = "start " + this->_server.path + "/ShooterGame/Binaries/Win64/ShooterGameServer.exe"
		+ " " + "ScorchedEarth_P" + "?listen?Port=" + this->_server.port + "?QueryPort=" + this->_server.queryport + "?RconPort=" + this->_server.rconport + "?"
		+ config->getRoot()["startCmdAdd"].asString()+" -automanagedmods";

	Rcon_addr addr{ this->_server.name,config->getRoot()["IP"].asString()
		,stoi(this->_server.rconport),this->_server.pass };
	this->_connection.serAddr(addr);

	for (auto& i : this->_server.mods)
		this->_mods[i] = { 0,0 };

	this->checkUpdate();

	delete(config);
	return true;
}

