#include "ArkUpdate.h"

using namespace std;

ArkUpdate::ArkUpdate()
	:_updateLog(MyLog::Log::createLog("AppLog/ArkUpdateLog"))
	,_arkJson(new JsonOperate())
{
	this->_arkJson->openFile("Config.json");
	this->updateVersionFromUrl();
	this->init();
}

ArkUpdate::~ArkUpdate()
{ 
	delete(this->_updateLog);
	delete(this->_arkJson);
}

void ArkUpdate::init()
{
	auto root = this->_arkJson->getRoot();
	auto server = root["Servers"].getMemberNames();
	for(auto &i:server) {
		ArkServer server;
		server.hwnd = NULL;
		server.listenPort = root["Servers"][i]["Port"].asString();
		server.map = root["Servers"][i]["map"].asString();
		for (auto& j : root["Servers"][i]["mods"])
			server.mods.insert(j.asString());
		server.name = i;
		server.path = root["Servers"][i]["path"].asString();
		server.queryPort = root["Servers"][i]["QueryPort"].asString();
		server.rconPort = root["Servers"][i]["RconPort"].asString();
		server.version = "";
		this->_arkServerWindow.push_back(server);
	}
	this->arkCheckWindows();
}

bool ArkUpdate::checkCrashed()
{
	DEBUGLOG("Function in");
	auto hwnd = FindWindowA(NULL, "The UE4-ShooterGame Server has crashed and will close");
	if (!hwnd) {
		DEBUGLOG("Function return");
		return false;
	}
	EnumChildWindows(hwnd, [](HWND Hwnd, LPARAM)->int {
		char* str = new char[50] {0};
		GetClassNameA(Hwnd, str, 50);
		if (string(str) == "Button") {
			SendMessageA(Hwnd, WM_LBUTTONDOWN, 0, 0);
			Sleep(50);
			SendMessageA(Hwnd, WM_LBUTTONUP, 0, 0);
		}
		delete[] str;
		return 0;
		}, NULL);
	DEBUGLOG("crashed!");
	this->_updateLog->logoutUTF8(TimeClass().TimeNow() + "--crashed");
	DEBUGLOG("Function return");
	return true;
}

void ArkUpdate::arkUpdate()
{
	DEBUGLOG("Function in");
	for (auto &i : this->_arkServerWindow) {
		i.version = this->readVersion(i.hwnd);
		if (i.version != "" && i.version != this->_netVersion) {
			//close window
			this->_updateLog->logoutUTF8(TimeClass::TimeClass().TimeNow() + "--" + "ShutDown" + "--" + i.name);
			this->closeArkWindow(i.hwnd);

			//make update cmd
			string updateCmd = _arkJson->getRoot()["steamcmdPath"].asString() 
				+ " +login anonymous" + " +force_install_dir " + i.path + " +app_update 376030 validate +quit";

			this->_updateLog->logoutGBK(TimeClass::TimeClass().TimeNow() + "--" + "StartUpdate" + "--" + i.path);
			system(updateCmd.c_str());
			this->_updateLog->logoutGBK(TimeClass::TimeClass().TimeNow() + "--" + "UpdateFinish" + "--" + i.path);

		}
	}
	DEBUGLOG("Function return");
}

void ArkUpdate::closeAll()
{
	this->arkCheckWindows();
	for (auto& i : this->_arkServerWindow) {
		if (i.hwnd) {
			this->closeArkWindow(i.hwnd);
		}
	}
}

void ArkUpdate::updateVersionFromUrl()
{
	DEBUGLOGFIN;
	auto version =CurlOperate::get("http://arkdedicated.com/version");
	if (version.find(".", 0) != string::npos) {
		if (this->_netVersion != version) {
			DEBUGLOG("Version has been changed from " + this->_netVersion + " to " + version);
			this->_netVersion = version;
			this->_updateLog->logoutUTF8(TimeClass::TimeClass().TimeNow()+ "--" + version + "--" + "版本号更新");
		}
	}
#ifdef _DEBUG
	std::cout << TimeClass::TimeClass().TimeNow() + "--" + version << endl;
#endif // _DEBUG
	DEBUGLOGFRE;
}

std::string ArkUpdate::readVersion(const HWND& hwnd )
{
	string version;
	LPSTR winname = (LPSTR)new char[256];
	::memset(winname, 0, 256);
	GetWindowTextA(hwnd, winname, 256);
	string name(winname);
	stringstream ss;
	ss << name;
	ss >> version;
	version = "";
	ss >> version;
	if (version[0] != 'v')return"";
	version = version.substr(1, version.size());
	return version;
};

void ArkUpdate::shutdownAndModsUpdate(const std::vector<std::string>& modid)
{
	map<string, HWND> modupdate;

	for (auto& i : this->_arkServerWindow) {

		for (auto& j : modid) {

			auto iter = i.mods.find(j);

			if (iter != i.mods.end()) {

				modupdate[i.name] = i.hwnd;
				break;

			}

		}

	}

	for (auto& i : modupdate)
		this->closeArkWindow(i.second);
}

bool ArkUpdate::checkUpdate()
{
	DEBUGLOGFIN;
	this->updateVersionFromUrl();
	this->arkCheckWindows();
	for (auto &i : this->_arkServerWindow) {
		i.version = this->readVersion(i.hwnd);
		if (i.version != this->_netVersion) {
			DEBUGLOGFRE;
			return true;
		}
	}
	DEBUGLOGFRE;
	return false;
}

bool ArkUpdate::closeArkWindow(HWND hwnd)
{
	//close server window
	SendNotifyMessage(hwnd, WM_CLOSE, 0, 0);
	while (true) {	//wait for close finish
		LPSTR winname = (LPSTR)new char[256];
		::memset(winname, 0, 256);
		GetWindowTextA(hwnd, winname, 256);
		string name(winname);
		delete[](winname);
		Sleep(2);
		if (name == "")
			break;
	}
	return true;
}

void ArkUpdate::arkRestart()
{
	DEBUGLOGFIN;
	//枚举顶层窗口,得到所有方舟窗口的窗口句柄
	this->arkCheckWindows();
	DEBUGLOG("getRoot");
	auto root = this->_arkJson->getRoot();
	DEBUGLOG("begin start server");
	for (auto &i : this->_arkServerWindow) {

		if (i.hwnd == NULL) {	//start any server which is not started

			string startCmd = "start " + i.path + "/ShooterGame/Binaries/Win64/ShooterGameServer.exe" 
				+ " " + i.map + "?listen?Port=" + i.listenPort + "?QueryPort=" + i.queryPort + "?RconPort=" + i.rconPort + "?" 
				+ this->_arkJson->getRoot()["startCmdAdd"].asString();
			DEBUGLOG(startCmd);

			this->_updateLog->logoutGBK(TimeClass::TimeClass().TimeNow() + "--" + "reboot" + "--" + i.name);
			
			DEBUGLOG("updateLog");
			::system(startCmd.c_str());
			DEBUGLOG("start over");
			Sleep(3000);

		}

	}
	DEBUGLOGFRE;
}

void ArkUpdate::arkCheckWindows()
{
	DEBUGLOGFIN;
	bool reflag = true;

	for (auto& i : this->_arkServerWindow) {

		LPSTR winname = (LPSTR)new char[256];
		::memset(winname, 0, 256);
		GetWindowTextA(i.hwnd, winname, 256);
		string name(winname);
		delete[](winname);

		if (name == "") {

			reflag = false;
			break;

		}

	}

	//如果所有窗口句柄都存在窗口，直接返回
	if (reflag)return;

	for (auto &i : this->_arkServerWindow) {

		i.hwnd = NULL;

	}

	//save the ark server window hwnd in this->_arkServerWindow
	EnumWindows([](HWND hwnd, LPARAM lParam)->BOOL {
		auto ptr = (ArkUpdate*)lParam;
		LPSTR a = (LPSTR)new char[256];
		::memset(a, 0, 256);
		GetWindowTextA(hwnd, a, 256);
		std::string name(a);
		if (name.find("ShooterGameServer.exe", 0) != string::npos) {
			string windowtext = name;
			stringstream ss;
			ss << name;
			string path;
			ss >> path;

			size_t tail = path.find("/ShooterGame/Binaries/Win64/ShooterGameServer.exe", 0);
			if (tail == -1)
				tail = path.find("\\ShooterGame\\Binaries", 0);
			size_t head = path.rfind("/", tail-1);
			if (head == string::npos)
				head = path.rfind("\\", tail-1);
			char* b = new char[128];
			char* c = (char*)path.c_str() + head + 1;
			::memset(b, 0, 128);
			::memcpy(b, c, tail - head - 1);
			string servername(b);

			//ptr->log(servername);
			
			delete[](b);
			for (auto& i : ptr->getServer()) {
				if (i.name == servername) {
			#ifdef _DEBUG
					std::cout << hwnd << endl;
			#endif // _DEBUG
					i.hwnd = hwnd; break;
				}
			}
		}
		delete[](a);
		return TRUE; 
		}, (LPARAM)(this));

	for (auto& i : this->_arkServerWindow) {
		if (i.version == "")i.version = this->readVersion(i.hwnd);
	}
	DEBUGLOGFRE;
}


