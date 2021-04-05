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
	DEBUGLOGFIN;
	for (auto& i : this->_server) {
		DEBUGLOG("Server name:" + i->getServerName());
		i->updatePlayerList();
	}
	DEBUGLOGFRE;
}

void ArkRcon::clearRecv()
{
	DEBUGLOGFIN;
		for (auto& i : this->_server) {
			DEBUGLOG("Server name:" + i->getServerName());
			i->clearRecv();
		}
		DEBUGLOGFRE;
}

void ArkRcon::broadcast(const std::string& data)
{
	DEBUGLOGFIN;
		for (auto& i : this->_server) {
			DEBUGLOG("Server name:" + i->getServerName());
				i->broadcast(data);
		}
	DEBUGLOGFRE;
}

void ArkRcon::updateGameName()
{
	DEBUGLOGFIN;
	for (auto& i : this->_server) {
		DEBUGLOG("Server name:" + i->getServerName());
		i->updateGameName();
	}
	DEBUGLOGFRE;
}

void ArkRcon::sendCmdAndWiatForItRecv(const std::string& data)
{
	DEBUGLOGFIN;
	for (auto& i : this->_server) {
		DEBUGLOG("Server name:" + i->getServerName());
		i->sendCmdAndWiatForRecv(data);
	}
	DEBUGLOGFRE;
}

void ArkRcon::shutConnect()
{
	DEBUGLOGFIN;
	for (auto& i : this->_server) {
		DEBUGLOG("Server name:" + i->getServerName());
		i->shutConnect();
	}
	DEBUGLOGFRE;
}

void ArkRcon::reconnect()
{
	DEBUGLOGFIN;
	for (auto& i : this->_server) {
		DEBUGLOG("Server name:" + i->getServerName());
		i->init();
	}
	DEBUGLOGFRE;
}

void ArkRcon::kick(const std::string& steamid)
{
	DEBUGLOGFIN;

	for (auto& i : this->_server) {

		auto players = i->getPlayers();

		for (auto& j : players) {

			if (j.steamId == steamid) {

				i->sendCmdAndWiatForRecv("kickplayer " + steamid);
				DEBUGLOG("steamid:" + steamid + "--name:" + j.steamName + "--kicked!");
				break;
			}

		}

	}

	DEBUGLOGFRE;
}

std::vector<std::pair<std::string, bool>>* ArkRcon::getState()
{
	DEBUGLOGFIN
	auto re = new vector<pair<string, bool>>();

	for (auto& i : this->_server) {

		DEBUGLOG("Server name:" + i->getServerName());
		(*re).push_back(make_pair(i->getServerName(), i->connectedState()));

	}

	DEBUGLOGFRE;
	return re;

}

bool ArkRcon::addServer(Rcon_addr addr)
{
	DEBUGLOGFIN;
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
	DEBUGLOGFRE;
	return re;
}


