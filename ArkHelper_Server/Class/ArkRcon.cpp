#include"ArkRcon.h"

using namespace std;

ArkRcon::ArkRcon()
{
}

ArkRcon::~ArkRcon()
{
	WSACleanup();
	for (auto& i : this->_server)delete(i);
}

bool ArkRcon::init()
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		LOG("WSA init Error");
		return false;
	}
	return true;
}

void ArkRcon::updateplayerlist()
{
	DEBUGLOG("Function in");
	for (auto& i : this->_server) {
		DEBUGLOG("Server name:" + i->getServerName());
		i->updatePlayerList();
	}
	DEBUGLOG("Function return");
}

void ArkRcon::clearRecv()
{
	DEBUGLOG("Function in");
		for (auto& i : this->_server) {
			DEBUGLOG("Server name:" + i->getServerName());
			i->clearRecv();
		}
	DEBUGLOG("Function return");
}

void ArkRcon::broadcast(std::string& data)
{
	DEBUGLOG("Function in");
		for (auto& i : this->_server) {
			DEBUGLOG("Server name:" + i->getServerName());
				i->broadcast(data);
		}
	DEBUGLOG("Function return");
}

void ArkRcon::updateGameName()
{
	DEBUGLOG("Function in");
	for (auto& i : this->_server) {
		DEBUGLOG("Server name:" + i->getServerName());
		i->updateGameName();
	}
	DEBUGLOG("Function return");
}

void ArkRcon::sendCmdAndWiatForItRecv(const std::string& data)
{
	DEBUGLOG("Function in");
	for (auto& i : this->_server) {
		DEBUGLOG("Server name:" + i->getServerName());
		i->sendCmdAndWiatForRecv(data);
	}
	DEBUGLOG("Function return");
}

void ArkRcon::shutConnect()
{
	DEBUGLOG("Function in");
	for (auto& i : this->_server) {
		DEBUGLOG("Server name:" + i->getServerName());
		i->shutConnect();
	}
	DEBUGLOG("Function return");
}

void ArkRcon::reconnect()
{
	DEBUGLOG("Function in");
	for (auto& i : this->_server) {
		DEBUGLOG("Server name:" + i->getServerName());
		i->init();
	}
	DEBUGLOG("Function return");
}

bool ArkRcon::addServer(Rcon_addr addr)
{
	DEBUGLOG("Function in");
	bool re = false;
	auto server = new ArkServer();
	auto flag = server->init(addr);
	if (flag) {
		DEBUGLOG(server->getServerName() + " connected succeed");
		LOG(server->getServerName() + " connect succeed!");
		server->updatePlayerList();
		re = true;
	}
	else {
		DEBUGLOG(server->getServerName()+" connected failed");
		LOG(server->getServerName() + " connect failed!");
	}
	this->_server.push_back(server);
	DEBUGLOG("Function return");
	return re;
}


