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
	*��socket�������е����ݲ�����ֱ��socket��û���µ����ݺ�����
	*/
	void clearRecv();

	/**
	*��socket����һ���������socket���ް����򷵻�һ��(-1,-1,"")��
	*/
	packet recvData();

	/**
	*������������б�
	*/
	void updatePlayerList();

	/**
	*����������ҵ���Ϸ����������
	*/
	void updateGameName();

	/**
	*���͹���,�ַ�������ΪUTF-8
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
	*�ȴ�����һ���ض�ID�İ�
	*/
	packet waitForRecvData();

	/**
	*�ȴ���֤��ͨ��
	*/
	bool waitForAuth();

	/**
	*��װһ����
	*/
	void pack(unsigned char packet[], const std::string data, int packet_len, int id, int type);
	
	/**
	*��ȡһ�������İ�
	*/
	bool readPacket(unsigned char **buffer,size_t& size);

	/**
	*��ȡ����
	*/
	size_t readPacketLen() const;

	/**
	*�ֽ���תint
	*/
	size_t byte32ToInt(unsigned char* buffer) const;
private:
	Rcon_addr _rconAddr;
	std::set<Player> _player;
	SOCKET _client;
	bool _connected;	//����״̬
	unsigned int _id;	//cmd id
	time_t _lastRecvTime;	//last time recv a packet
};