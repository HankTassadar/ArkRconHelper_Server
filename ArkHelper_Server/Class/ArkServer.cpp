#include"ArkServer.h"

using namespace std;

ArkServer::ArkServer()
	:_id(1)
	, _connected(false)
	,_client(0)
	,_lastRecvTime(time(nullptr))
{
}

ArkServer::~ArkServer()
{
	closesocket(this->_client);
}

bool ArkServer::init(Rcon_addr addr)
{
	this->_rconAddr = addr;
	return this->init();
}

bool ArkServer::init()
{
	this->_connected = false;
	this->_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->_client == INVALID_SOCKET) {
		LOG("create socket error!" + WSAGetLastError())
			return false;
	}
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(this->_rconAddr.port);
	inet_pton(AF_INET, this->_rconAddr.ip.c_str(), &(serAddr.sin_addr));

	u_long argp = 1;
	ioctlsocket(this->_client, FIONBIO, &argp);//非阻塞

	if (connect(this->_client, (sockaddr*)(&serAddr), sizeof(serAddr)) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			LOG(WSAGetLastError());
			return false;
		}
	}
	struct timeval timeout;	//五秒内没有连接上直接返回超时
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;

	fd_set rfd;
	FD_ZERO(&rfd);
	FD_SET(this->_client, &rfd);
	auto ret = select(0, 0, &rfd, 0, &timeout);
	if (ret <= 0) {
		this->_connected = false;
		return false;
	}

	this->_connected = true;
	return this->auth();
}

bool ArkServer::auth()
{
	this->sendData(this->_rconAddr.password, SERVERDATA_AUTH);
	return this->waitForAuth();
}

bool ArkServer::sendData(const std::string data, const int type)
{
	//LOG("Sending: " + data);

	int packet_len = data.length() + RCON_HEADER_SIZE;
	unsigned char *packet=new unsigned char[packet_len];
	this->pack(packet, data, packet_len,this->_id++ , type);
	if (this->_connected) {
		if (send(this->_client, (const char*)packet, packet_len, 0) < 0) {
			LOG("send error! ArkServer.cpp-53-error:" + WSAGetLastError());
		}
		else {
			delete[] packet;
			return true;
		}
	}
	delete[] packet;
	return false;
}

void ArkServer::clearRecv()
{
	while (true) {
		auto i = this->recvData();
		if (i.id == -1 && i.type == -1)break;

		if (i.id == 0 && i.type == 0) {	//keep alive包
			break;
		}
		
		//处理接收到的其他种类的包
	}
}

ArkServer::packet ArkServer::recvData()
{
	unsigned char* buffer=nullptr;
	size_t size=0;
	auto ret = this->readPacket(&buffer, size);
	if (!ret)return packet{ -1,-1,"" };
	int id = this->byte32ToInt(buffer);
	int type = this->byte32ToInt(buffer + 4);
	string data(buffer + 8, buffer + size);
	delete[] buffer;
	LOG(data);
	time(&(this->_lastRecvTime));
	return packet{id,type,data};
}

void ArkServer::updatePlayerList()
{
	this->sendData("listplayers", SERVERDATA_EXECCOMMAND);
	auto re = this->waitForRecvData();
	if (re.data.find("No Player") != string::npos) {
		this->_player.clear();
		return;
	}
	LOG(this->getServerName()+re.data);
	//检查在线玩家数量
	int playernum = -1;
	int pos = re.data.find("\n", 0);
	do {
		playernum++;
		pos = re.data.find("\n", pos+1);
	} while (pos != string::npos);
	//获取在线玩家信息
	pos = 0;
	char* ptr = (char*)re.data.c_str();
	set<Player> player;
	for (auto i = 0; i < playernum; i++) {	//将所有玩家信息加入一个set中
		Player p;
		auto head = re.data.find('\n', pos);
		auto namehead = re.data.find(". ", pos);
		auto nametail = re.data.find(", ", pos);
		p.steamName = string(ptr + namehead + 2, ptr + nametail);
		auto temp_ptr = ptr;
		temp_ptr += nametail;
		while (!isdigit(*temp_ptr++));
		auto idhead = temp_ptr - 1;
		while (isdigit(*temp_ptr++));
		auto idtail = temp_ptr - 1;
		p.steamId = string(idhead, idtail);
		player.insert(p);
		pos = re.data.find('\n', pos + 1);
	}
	auto lastPnum = this->_player.size();
	auto Pnum = player.size();
	if (lastPnum == 0 && Pnum == 0) return;
	if (lastPnum == 0 && Pnum != 0) {	//第一个进服的玩家或者程序第一次启动时
#ifndef _DEBUG
		this->saveworld();
#endif // !_DEBUG


		this->_player = player;	
		return;
	}
	for (auto &i: player) {
		auto p = this->_player.insert(i);
		if (p.second) {
			LOG("Player Joined!" + p.first->steamId);
		}
	}
#ifndef _DEBUG
	if (this->_player.size() > lastPnum) {
		this->saveworld();
	}
#endif // _DEBUG
	this->_player = player;
}

void ArkServer::updateGameName()
{
	vector<Player> players;
	for (auto &i : this->_player)
		players.push_back(i);
	this->_player.clear();
	for (auto &i : players) {
		this->sendData("getplayername " + i.steamId, SERVERDATA_EXECCOMMAND);
		auto re = this->waitForRecvData();
		char* head = (char*)re.data.c_str() + re.data.find("- ") + 2;
		char* tail = (char*)re.data.c_str() + re.data.find("\n");
		i.gameName= string(head, tail);
		this->_player.insert(i);
	}
}

void ArkServer::broadcast(std::string &data)
{
	this->sendData("broadcast " + data, SERVERDATA_EXECCOMMAND);
	auto i = this->waitForRecvData();
}

bool ArkServer::saveworld()
{
	this->sendData("saveworld", SERVERDATA_EXECCOMMAND);
	auto re = this->waitForRecvData();
	LOG(re.data);
	return true;
}

ArkServer::packet ArkServer::sendCmdAndWiatForRecv(const std::string& data)
{
	this->sendData(data, SERVERDATA_EXECCOMMAND);
	return this->waitForRecvData();
}

ArkServer::packet ArkServer::waitForRecvData()
{
	int ID = this->_id - 1;
	auto re = this->recvData();
	while (re.id != ID) {
		Sleep(20);
		re = this->recvData();
		if (!this->_connected) {
			LOG(this->getServerName() + "--lost connection!");
			break;
		}
	}
	return re;
}

bool ArkServer::waitForAuth()
{
	clock_t start, ends;
	start = clock();
	while (true) {
		Sleep(10);
		auto p = this->recvData();
		if (p.type == SERVERDATA_AUTH_RESPONSE) {
			if (p.id == -1)
				return false;
			else
				return true;
		}
		ends = clock();
		if (ends - start > 500)break;	//500毫秒无回应自动超时返回false
	}
	return false;
}

void ArkServer::pack(unsigned char packet[], const std::string data, int packet_len, int id, int type)
{
	int data_len = packet_len - RCON_HEADER_SIZE;
	memset(packet, 0, packet_len);	
	packet[0] = data_len + 10;
	packet[4] = id;
	packet[8] = type;
	for (int i = 0; i < data_len; i++)
		packet[12 + i] = data.c_str()[i];

}

bool ArkServer::readPacket(unsigned char** buffer, size_t& size)
{
	auto len = this->readPacketLen();
	if (len == 0)return false;
	size = len;
	auto p = new unsigned char[len] {0};
	*buffer = p;
	if (len > RCON_SLEEP_THRESHOLD) {
		Sleep(RCON_SLEEP_MILLISECONDS);
	}
	auto ptr = p;
	unsigned int bytes = 0;
	do bytes += recv(this->_client, (char*)(ptr+bytes), len - bytes, 0);
	while (bytes < len);
	return true;
}

size_t ArkServer::readPacketLen()
{
	/*
	unsigned char* buffer = new unsigned char[4]{ 0 };
	auto i = recv(this->_client, (char*)buffer, 4, 0);
	if (i == -1) {
		delete[]buffer;
		return 0;
	}
	if (i == 0) {
		delete[]buffer;
		this->_connected = false;
		LOG("Lost connection!");
		return 0;
	}
	const size_t len = byte32ToInt(buffer);
	LOG(len);
	delete[] buffer;
	return len;
	*/

	unsigned char* buffer = new unsigned char[4]{ 0 };

	timeval timeout{ 0,500 };
	fd_set rfd;
	FD_ZERO(&rfd);
	FD_SET(this->_client, &rfd);
	int i=-1;
	auto ret = select(0, &rfd, 0, 0, &timeout);
	if (ret == 1) {
		if (FD_ISSET(this->_client, &rfd)) {
			i = recv(this->_client, (char*)buffer, 4, 0);
			if (i == 0 || i==-1) {
				this->_connected = false;
			}
		}
	}
	size_t len = 0;
	if (this->_connected && i == 4) {
		len = byte32ToInt(buffer);
		LOG(len);
	}
	delete[] buffer;
	return len;
	
}

size_t ArkServer::byte32ToInt(unsigned char* buffer) const
{
	return	static_cast<size_t>(
		static_cast<unsigned char>(buffer[0]) |
		static_cast<unsigned char>(buffer[1]) << 8 |
		static_cast<unsigned char>(buffer[2]) << 16 |
		static_cast<unsigned char>(buffer[3]) << 24);
}
