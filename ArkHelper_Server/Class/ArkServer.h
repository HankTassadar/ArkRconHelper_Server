#pragma once
#include<WS2tcpip.h>
#include<WinSock2.h>
#include<string>
#include<vector>
#include<set>
#include<iostream>
#include<time.h>
#include"Log.h"
#pragma comment(lib,"ws2_32.lib")

#ifdef _DEBUG
#define LOG(str) std::cout << str  << std::endl;
#else
#define LOG(str)
#endif

#define SERVERDATA_AUTH 3
#define SERVERDATA_EXECCOMMAND 2
#define SERVERDATA_AUTH_RESPONSE 2
#define SERVERDATA_RESPONSE_VALUE 0

#define RCON_DEFAULT_TIMEOUT 4
#define RCON_HEADER_SIZE 14
#define RCON_SLEEP_THRESHOLD 1024
#define RCON_SLEEP_MILLISECONDS 200

struct Rcon_addr {
	std::string serverName;
	std::string	ip;
	u_short port = 0;
	std::string password;
};

struct Player {
	std::string steamName;
	std::string gameName;
	std::string steamId;
	bool operator<(const struct Player &right) const {
		if (this->steamId == right.steamId)return false;
		return this->steamId < right.steamId;
	}
	bool operator==(const struct Player &right) const {
		return this->steamId == right.steamId;
	}
};

class ArkServer {
public:
	struct packet {
		int id;
		int type;
		std::string data;
	};
public:
	ArkServer();
	~ArkServer();

public:
	bool init(Rcon_addr addr);

	bool auth();

	bool sendData(const std::string data, const int type);

	/**
	*从socket接收所有的数据并处理，直到socket中没有新的数据后跳出
	*/
	void clearRecv();

	/**
	*从socket接收一个包，如果socket中无包，则返回一个(-1,-1,"")包
	*/
	packet recvData();

	/**
	*更新在线玩家列表
	*/
	void updatePlayerList();

	/**
	*更新在线玩家的游戏内人物名字
	*/
	void updateGameName();

	/**
	*发送公告,字符串编码为UTF-8
	*/
	void broadcast(std::string&);

	/**
	*server save
	*/
	bool saveworld();

	/**
	*send a cmd and wiat for recive it's back
	*/
	packet sendCmdAndWiatForRecv(const std::string&);

	std::string getServerName() {
		return this->_rconAddr.serverName;
	}

	std::set<Player>& getPlayers() {
		return this->_player;
	}
private:
	/**
	*等待接收一个特定ID的包
	*/
	packet waitForRecvData();

	/**
	*等待验证的通过
	*/
	bool waitForAuth();

	/**
	*封装一个包
	*/
	void pack(unsigned char packet[], const std::string data, int packet_len, int id, int type);
	
	/**
	*读取一个完整的包
	*/
	bool readPacket(unsigned char **buffer,size_t& size);

	/**
	*读取包长
	*/
	size_t readPacketLen() const;

	/**
	*字节流转int
	*/
	size_t byte32ToInt(unsigned char* buffer) const;
private:
	Rcon_addr _rconAddr;
	std::set<Player> _player;
	SOCKET _client;
	bool _connected;	//在线状态
	unsigned int _id;	//cmd id
	time_t _lastRecvTime;	//last time recv a packet
};