#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include <map>
#include <vector>
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
#include "PacketWriter.h"
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
	char* host;
	unsigned short port;

	bool _connected;
	int _clientid_incr;
	SOCKET sock;

	std::map<sockaddr_in, RemoteClient> remotes{};
	std::map<int, Room> rooms{};

	std::vector<std::pair<RemoteClient, PendingTransmittedPacket>> pending;

	bool IdentifyClient(RemoteClient* client, std::string username, int version);

	bool OnMessageReceived(sockaddr_in& remote, char* bytes, int bytes_received);
	bool ParsePayload(RemoteClient* remote, unsigned short payloadlen, unsigned short tag, BinaryReader& reader);
	bool ParseMessage(RemoteClient* remote, BinaryReader& reader);
	bool GetOrCreateClient(sockaddr_in& remote, RemoteClient* client);
	bool BeginPing();

	bool Acknowledge(RemoteClient* client, unsigned short nonce);
	bool SetAcknowledged(RemoteClient* client, unsigned short nonce);
	PendingTransmittedPacket AppendSentReliable(RemoteClient* client, unsigned short nonce);
	PendingTransmittedPacket AppendReceivedReliable(RemoteClient* client, unsigned short nonce);

	unsigned char CalculateMissing(RemoteClient* client);
public:
	CPPolusServer();

	bool Bind(unsigned short _port = 22023, char _host[4] = { 0 });
	bool Listen();

	bool SendTo(RemoteClient* client, unsigned char* buf, size_t size);
	bool SendTo(RemoteClient* client);
	bool Broadcast(std::map<int, ClientData> clients, unsigned char* buf, size_t size, std::vector<ClientData> exclude);
	bool Broadcast(std::map<int, ClientData> clients, PacketWriter& packet, std::vector<ClientData> exclude);
	bool Broadcast(std::map<int, ClientData> clients, std::vector<ClientData> exclude);
	bool SendRepeat(RemoteClient* client, unsigned char* buf, size_t size, unsigned short nonce);
	bool SendRepeat(RemoteClient* client, unsigned short nonce);
	bool Disconnect(RemoteClient* client, char reason = 0, std::string message = "");
	bool JoinError(RemoteClient* client, char reason = 0);
};