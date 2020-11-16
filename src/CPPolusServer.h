#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include <map>
#include <iostream>
#include <sstream>
#include <winsock2.h>
#include <algorithm>
#include <iterator>
#include <thread>

#include "utils/BinaryReader.h"
#include "utils/BinaryWriter.h"
#include "utils/Codes.h"
#include "utils/Singleton.h"
#include "utils/Versions.h"

#include "constants/Enums.h"

#include "RemoteClient.h"
#include "Room.h"

#pragma comment(lib, "ws2_32.lib")

std::string getaddr(sockaddr_in remote);

inline bool operator==(const sockaddr_in& first, const sockaddr_in& second)
{
	return first.sin_addr.S_un.S_addr == second.sin_addr.S_un.S_addr &&
		first.sin_port == second.sin_port;
}

inline bool operator<(const sockaddr_in& first, const sockaddr_in& second)
{
	return first.sin_addr.S_un.S_addr < second.sin_addr.S_un.S_addr&&
		first.sin_port < second.sin_port;
}

class CPPolusServer : public Singleton<CPPolusServer>
{
	bool pinging;

	bool IdentifyClient(RemoteClient& client, std::string username, int version);

	bool OnMessageReceived(sockaddr_in& remote, char* bytes);
	bool ParsePayload(RemoteClient& remote, BinaryReader& reader);
	bool ParseMessage(RemoteClient& remote, BinaryReader& reader);
	bool GetOrCreateClient(sockaddr_in& remote, RemoteClient* client);

	void Recv(int& size, char);
public:
	char* host;
	unsigned short port;
	std::map<sockaddr_in, RemoteClient> remotes{};
	std::map<int, Room> rooms{};

	bool connected;
	int clientid_incr;
	SOCKET sock;

	CPPolusServer();

	bool Bind(unsigned short _port = 22023, char _host[4] = { 0 });
	bool Listen();
	bool BeginPing();
	bool StopPing();

	bool SendTo(RemoteClient& client, const char* buf, unsigned int size);
	bool SendTo(RemoteClient& client, BinaryWriter& writer);
	/* bool SendAndWait(RemoteClient client, const char* buf, unsigned int size, unsigned short nonce);
	   bool SendAndWait(RemoteClient client, BinaryWriter writer, unsigned short nonce); */
	bool Acknowledge(RemoteClient& client, unsigned short nonce);
	bool SetAcknowledged(RemoteClient& client, unsigned short nonce);
	bool AppendSentReliable(RemoteClient& client, unsigned short nonce);
	bool AppendReceivedReliable(RemoteClient& client, unsigned short nonce);
	unsigned char CalculateMissing(RemoteClient& client);
	bool Disconnect(RemoteClient& client, char reason = 0, std::string message = "");
};