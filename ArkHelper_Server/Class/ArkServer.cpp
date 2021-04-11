#include"ArkServer.h"

using namespace std;

ArkServer::ArkServer()
	:_id(1)
	, _connected(false)
	,_client(0)
{
}

ArkServer::~ArkServer()
{
	if(this->_connected)
		closesocket(this->_client);
}

bool ArkServer::init(Rcon_addr addr)
{

	this->_rconAddr = addr;
	return this->init();

}

void ArkServer::serAddr(Rcon_addr addr)
{
	this->_rconAddr = addr;
}

bool ArkServer::init()
{
	DEBUGLOGFIN;
	if (this->_connected) {

		DEBUGLOGFRE; return true;

	}

	this->_id = 1;
	this->_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (this->_client == INVALID_SOCKET) {

		DEBUGLOG("create socket error!" + to_string(WSAGetLastError()));
		LOG("create socket error!" + WSAGetLastError());
		DEBUGLOGFRE; return false;

	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(this->_rconAddr.port);
	inet_pton(AF_INET, this->_rconAddr.ip.c_str(), &(serAddr.sin_addr));

	u_long argp = 1;
	ioctlsocket(this->_client, FIONBIO, &argp);//非阻塞

	if (connect(this->_client, (sockaddr*)(&serAddr), sizeof(serAddr)) == SOCKET_ERROR) {

		if (WSAGetLastError() != WSAEWOULDBLOCK) {

			DEBUGLOG(to_string(WSAGetLastError()));
			LOG(WSAGetLastError());
			DEBUGLOGFRE; return false;

		}

	}

	struct timeval timeout {0,500};	//500ms内没有连接上直接返回超时

	fd_set rfd;
	FD_ZERO(&rfd);
	FD_SET(this->_client, &rfd);
	auto ret = select(0, 0, &rfd, 0, &timeout);

	if (ret <= 0) {

		DEBUGLOG("socket connected failed");
		return false;

	}

	this->_connected = true;
	return this->auth();
}

bool ArkServer::auth()
{
	DEBUGLOGFIN;

	this->sendData(this->_rconAddr.password, SERVERDATA_AUTH);

	if (this->waitForAuth()) {

		DEBUGLOG("auth succeed");
		this->_connected = true;
		DEBUGLOGFRE; return true;

	}

	this->_connected = false;
	DEBUGLOGFRE; return false;

}

bool ArkServer::waitForAuth()
{
	DEBUGLOGFIN;

	clock_t start, ends;
	start = clock();

	while (true) {

		Sleep(10);
		auto p = this->recvData();

		if (p.type == SERVERDATA_AUTH_RESPONSE) {

			if (p.id == -1) {

				DEBUGLOG(this->getServerName() + " auth was refused");
				DEBUGLOGFRE; return false;

			}
			else {

				DEBUGLOG(this->getServerName() + " auth was succeed");
				DEBUGLOGFRE; return true;

			}

		}

		ends = clock();

		if (ends - start > 200)break;	//200毫秒无回应自动超时返回false

	}

	DEBUGLOG(this->getServerName() + " auth was timeout in 200 msec");

	DEBUGLOGFRE; return false;
}

bool ArkServer::sendData(const std::string data, const int type)
{

	DEBUGLOGFIN;
	int packet_len = data.length() + RCON_HEADER_SIZE;
	DEBUGLOG("send data length:" + to_string(packet_len));
	unsigned char *packet=new unsigned char[packet_len];

	if (this->_id == 256) {//the massage id max is 255,if id=256,server will return id=0

		this->_id = 1;
		DEBUGLOG("id=256,set id=1");

	}
	this->pack(packet, data, packet_len,this->_id++ , type);

	if (this->_connected) {

		if (send(this->_client, (const char*)packet, packet_len, 0) < 0) {

			DEBUGLOG("send error!" + to_string(WSAGetLastError()));
			LOG("send error! ArkServer.cpp-53-error:" + WSAGetLastError());

		}
		else {

			DEBUGLOG("\nsend id:" + to_string(this->_id - 1) + "/send type:" + to_string(type) + "/send data:" + data + "\n");
			delete[] packet;
			DEBUGLOGFRE;
			return true;

		}

	}

	delete[] packet;
	DEBUGLOGFRE;
	return false;

}

void ArkServer::clearRecv()
{
	DEBUGLOGFIN;

	if (!this->_connected)return;

	while (true) {

		auto i = this->recvData();

		if (i.id == -1 && i.type == -1)break;

		if (i.id == 0 && i.type == 0) {	//keep alive包

			DEBUGLOG("recv a keepalive pack");

		}
		//处理接收到的其他种类的包
	}

	DEBUGLOGFRE;
}

ArkServer::packet ArkServer::recvData()
{
	DEBUGLOGFIN;

	unsigned char* buffer=nullptr;
	size_t size=0;
	auto ret = this->readPacket(&buffer, size);

	if (!ret) {

		DEBUGLOGFRE;
		return packet{ -1,-1,"" };

	}

	int id = this->byte32ToInt(buffer);
	int type = this->byte32ToInt(buffer + 4);
	string data(buffer + 8, buffer + size);
	DEBUGLOG("\nrecv id:" + to_string(id) + "/recv type:" + to_string(type) + "/recv data:" + data + "\n");
	delete[] buffer;
	LOG(data);

	DEBUGLOGFRE;
	return packet{id,type,data};

}

void ArkServer::updatePlayerList()
{
	DEBUGLOGFIN;

	if (!this->_connected) {

		DEBUGLOGFRE; return;

	}

	DEBUGLOG("send listplayers");
	this->sendData("listplayers", SERVERDATA_EXECCOMMAND);
	auto re = this->waitForRecvData();

	if (re.data.find("No Player") != string::npos) {

		this->_player.clear();
		DEBUGLOGFRE; return;

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

	if ((lastPnum == 0 && Pnum == 0) || Pnum==0) {	//no players in server 

		DEBUGLOG(this->getServerName() + "No Player");
		DEBUGLOGFRE; return;

	}

	for (auto &i: player) {

		auto p = this->_player.insert(i);

		if (p.second) {

			DEBUGLOG("Player JOIN:" + p.first->steamName + ";" + p.first->steamId);
			LOG("Player Joined!" + p.first->steamId);

		}

	}

#ifndef _DEBUG

	if (this->_player.size() > lastPnum) {

		this->saveworld();

	}

#endif // _DEBUG

	this->_player = player;

	DEBUGLOGFRE;
}

void ArkServer::updateGameName()
{
	DEBUGLOGFIN;

	if (!this->_connected) {

		DEBUGLOGFRE; return;

	}

	vector<Player> players;

	auto iter = this->_player.begin();

	while (iter != this->_player.end()) {

		if (iter->gameName == "") {

			players.push_back(*iter);
			this->_player.erase(iter++);

		}
		else {

			iter++;

		}

	}

	for (auto &i : players) {

		this->sendData("getplayername " + i.steamId, SERVERDATA_EXECCOMMAND);
		auto re = this->waitForRecvData();
		char* head = (char*)re.data.c_str() + re.data.find("- ") + 2;
		char* tail = (char*)re.data.c_str() + re.data.find("\n");
		i.gameName= string(head, tail);
		this->_player.insert(i);

	}

	DEBUGLOGFRE;
}

void ArkServer::broadcast(const std::string &data)
{
	DEBUGLOGFIN;
	if (!this->_connected) {
		DEBUGLOGFRE;
		return;
	}
	this->sendData("broadcast " + data, SERVERDATA_EXECCOMMAND);
	auto i = this->waitForRecvData();
	DEBUGLOGFRE;
}

bool ArkServer::saveworld()
{
	DEBUGLOGFIN;
	this->sendData("saveworld", SERVERDATA_EXECCOMMAND);
	auto re = this->waitForRecvData();
	LOG(re.data);
	DEBUGLOG(re.data);
	DEBUGLOGFRE;
	return true;
}

ArkServer::packet ArkServer::sendCmdAndWiatForRecv(const std::string& data)
{
	DEBUGLOGFIN;
	if (!this->_connected)return{ -1,-1,"" };
	this->sendData(data, SERVERDATA_EXECCOMMAND);
	auto re = this->waitForRecvData();
#ifdef _DEBUG
	std::cout << re.data;
#endif // _DEBUG
	DEBUGLOG(re.data);
	DEBUGLOGFRE;
	return re;
}

void ArkServer::shutConnect()
{
	DEBUGLOGFIN;
	if (this->_connected) {
		closesocket(this->_client);
	}
	DEBUGLOG(this->getServerName() + " shutConnect");
	this->_id = 1;
	this->_player.clear();
	this->_connected = false;
	DEBUGLOGFRE;
}

ArkServer::packet ArkServer::waitForRecvData()
{
	DEBUGLOGFIN;
	int ID = this->_id - 1;
	DEBUGLOG("ID wait for recv id:" + to_string(ID));
	auto re = this->recvData();
	int count = 0;
	while (re.id != ID) {
		Sleep(20);
		re = this->recvData();
		if (!this->_connected) {
			DEBUGLOG(this->getServerName() + " lost connection");
			LOG(this->getServerName() + "--lost connection!");
			break;
		}
		if (count == 50 * 5) {
			this->shutConnect();
			break;	//5秒还没接收到该包视为失败，关闭连接直接跳出
		}
		count++;
		DEBUGLOG("id recv:" + to_string(re.id));
	}
	if (count == 50 * 5) {
		DEBUGLOG("wait for recv faild in 5 sec;failed id:" + to_string(ID));
	}
	DEBUGLOGFRE;
	return re;
}

void ArkServer::pack(unsigned char packet[], const std::string data, int packet_len, int id, int type)
{
	DEBUGLOGFIN;
	int data_len = packet_len - RCON_HEADER_SIZE;
	memset(packet, 0, packet_len);	
	packet[0] = data_len + 10;
	packet[4] = id;
	packet[8] = type;
	for (int i = 0; i < data_len; i++)
		packet[12 + i] = data.c_str()[i];
	DEBUGLOGFRE;
}

bool ArkServer::readPacket(unsigned char** buffer, size_t& size)
{
	DEBUGLOGFIN;
	auto len = this->readPacketLen();
	if (len == 0) {
		DEBUGLOGFRE;
		return false;
	}
	DEBUGLOG("read a length:" + to_string(len));
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
	DEBUGLOGFRE;
	return true;
}

size_t ArkServer::readPacketLen()
{
	DEBUGLOGFIN;
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
				DEBUGLOG("lost connection");
				this->_connected = false;
				closesocket(this->_client);
				this->_player.clear();
				this->_id = 1;
			}
		}
	}
	size_t len = 0;
	if (this->_connected && i == 4) {
		len = byte32ToInt(buffer);
		DEBUGLOG("len:" + to_string(len));
		LOG(len);
	}
	else {
		DEBUGLOG("read length err,only read " + to_string(i) + " bytes");
	}
	delete[] buffer;
	DEBUGLOGFRE;
	return len;
	
}

size_t ArkServer::byte32ToInt(unsigned char* buffer) const
{
	DEBUGLOGFIN;
	size_t re= static_cast<size_t>(
		static_cast<unsigned char>(buffer[0]) |
		static_cast<unsigned char>(buffer[1]) << 8 |
		static_cast<unsigned char>(buffer[2]) << 16 |
		static_cast<unsigned char>(buffer[3]) << 24);
	DEBUGLOG("byte32 to int:" + to_string(re));
	DEBUGLOGFRE;
	return	re;
}
