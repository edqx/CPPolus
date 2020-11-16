#pragma once

#include <string>
#include <winsock2.h>
#include <deque>

struct PendingTransmittedPacket
{
	unsigned short nonce;
	bool received;
};

struct RemoteClient
{
	sockaddr_in* remote;
	unsigned int clientid;

	bool identified;
	bool sent_disconnect;

	std::string username;
	int version;

	short _nonce;
	
	std::deque<PendingTransmittedPacket> sent;
	std::deque<PendingTransmittedPacket> received;

	RemoteClient();
	RemoteClient(sockaddr_in* _remote, unsigned int _clientid);

	unsigned short nonce();
};