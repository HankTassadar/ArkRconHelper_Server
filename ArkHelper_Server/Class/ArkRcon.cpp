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
	for (auto& i : this->_server) {
		i->updatePlayerList();
	}
}

void ArkRcon::clearRecv()
{
	for (auto& i : this->_server)
		i->clearRecv();
}

void ArkRcon::broadcast(std::string& data)
{
	for (auto& i : this->_server)
		i->broadcast(data);
}

void ArkRcon::updateGameName()
{
	for (auto& i : this->_server)
		i->updateGameName();
}

void ArkRcon::sendCmdAndWiatForItRecv(const std::string& data)
{
	for (auto& i : this->_server)
		i->sendCmdAndWiatForRecv(data);
}

void ArkRcon::shutConnect()
{
	for (auto& i : this->_server)
		i->shutConnect();
}

void ArkRcon::reconnect()
{
	for (auto& i : this->_server)
		i->init();
}

bool ArkRcon::addServer(Rcon_addr addr)
{
	auto server = new ArkServer();
	auto flag = server->init(addr);
	if (flag) {
		LOG(server->getServerName() + " connect succeed!");
		server->updatePlayerList();
	}
	else {
		LOG(server->getServerName() + " connect faild!");
	}
	this->_server.push_back(server);
	return true;
}


