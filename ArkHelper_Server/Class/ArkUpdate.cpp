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
		server.name = i;
		server.path = root["Servers"][i]["path"].asString();
		server.queryPort = root["Servers"][i]["QueryPort"].asString();
		server.rconPort = root["Servers"][i]["RconPort"].asString();
		server.version = this->readVersion(server.path);
		this->_arkServerWindow.push_back(server);
	}
	this->arkCheckWindows();
}

bool ArkUpdate::checkCrashed()
{
	DEBUGLOG("Function in");
	auto hwnd = FindWindowA(NULL, "The UE4-ShooterGame Game has crashed and will close");
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
		i.version = this->readVersion(i.path);
		if (i.version != this->_netVersion) {
			//close window
			this->_updateLog->logoutUTF8(TimeClass::TimeClass().TimeNow() + "--" + "ShutDown" + "--" + i.name);
			this->closeArkWindow(i.hwnd);

			//make update cmd
			string updateCmd = _arkJson->getRoot()["steamcmdPath"].asString() 
				+ " +login anonymous" + " +force_install_dir " + i.path + " +app_update 376030 validate +quit";

			while(i.version != this->_netVersion){
				//update server
				this->_updateLog->logoutGBK(TimeClass::TimeClass().TimeNow() + "--" + "StartUpdate" + "--" + i.path);
				system(updateCmd.c_str());
				this->_updateLog->logoutGBK(TimeClass::TimeClass().TimeNow() + "--" + "UpdateFinish" + "--" + i.path);

				//update version number
				i.version = this->readVersion(i.path);
			}
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

std::string ArkUpdate::readVersion(const std::string &installpath)
{
	ifstream file(installpath + "/version.txt");
	if (!file.is_open()) {
		this->_updateLog->logoutUTF8(installpath + "/version.txt--open failed!");
		return "";
	}
	string version;
	stringstream buffer;
	buffer << file.rdbuf();
	buffer >> version;
	file.close();
	return version;
};

bool ArkUpdate::checkUpdate()
{
	DEBUGLOGFIN;
	this->updateVersionFromUrl();
	for (auto &i : this->_arkServerWindow) {
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
	//枚举顶层窗口,得到所有方舟窗口的窗口句柄
	this->arkCheckWindows();
	auto root = this->_arkJson->getRoot();
	for (auto &i : this->_arkServerWindow) {
		if (i.hwnd == NULL) {	//start any server which is not started
			string startCmd = "start " + i.path + "/ShooterGame/Binaries/Win64/ShooterGameServer.exe" 
				+ " " + i.map + "?listen?Port=" + i.listenPort + "?QueryPort=" + i.queryPort + "?RconPort=" + i.rconPort + "?" 
				+ this->_arkJson->getRoot()["startCmdAdd"].asString();
			this->_updateLog->logoutGBK(TimeClass::TimeClass().TimeNow() + "--" + "reboot" + "--" + i.name);
			system(startCmd.c_str());
			Sleep(3000);
		}
	}
}

void ArkUpdate::arkCheckWindows()
{
	for (auto &i : this->_arkServerWindow) {
		i.hwnd = NULL;
	}
	//save the ark server window hwnd in this->_arkServerWindow中
	EnumWindows([](HWND hwnd, LPARAM lParam)->BOOL {
		auto ptr = (vector<ArkServer>*)lParam;
		LPSTR a = (LPSTR)new char[256];
		::memset(a, 0, 256);
		GetWindowTextA(hwnd, a, 256);
		std::string name(a);
		if (name.find("ShooterGameServer.exe", 0) != string::npos) {
			string windowtext = name;
			stringstream ss;
			ss << name;
			string version = "";
			string path;
			ss >> path;

			size_t length = path.find("/ShooterGame/Binaries/Win64/ShooterGameServer.exe", 0);
			if (length == -1)
				length = path.find("\\ShooterGame\\Binaries", 0);
			char* b = new char[128];
			char* c = (char*)path.c_str() + 3;
			::memset(b, 0, 128);
			::memcpy(b, c, length - 3);
			string servername(b);
			delete[](b);
			for (auto& i : *ptr) {
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
		}, (LPARAM)(&(this->_arkServerWindow)));
}


