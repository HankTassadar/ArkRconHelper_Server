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
		exit(0);
	}
}

void ArkRcon::update()
{
	for (auto& i : this->_server) {
		i->clearRecv();
		i->updatePlayerList();
	}
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

bool ArkRcon::addServer(Rcon_addr addr)
{
	auto server = new ArkServer();
	auto flag = server->init(addr);
	if (flag) {
		LOG(server->getServerName() + "连接成功");
		server->updatePlayerList();
		this->_server.push_back(server);
		return true;
	}
	LOG(server->getServerName() + "连接失败");
	delete(server);
	return false;
}


