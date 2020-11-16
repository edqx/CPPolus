#include "CPPolusServer.h"

#define BUFLEN 512

std::string getaddr(sockaddr_in remote)
{
	const char* host = inet_ntoa(remote.sin_addr);
	unsigned short port = ntohs(remote.sin_port);

	std::stringstream strm;
	strm << host << ":" << port;

	std::string str(strm.str());

	return str;
}

CPPolusServer::CPPolusServer()
{
	pinging = false;
	connected = false;
	host = 0;
	port = 22023;
	clientid_incr = 0;
	sock = 0;
}

bool CPPolusServer::Bind(unsigned short _port, char* _host)
{
	printf("[INF] Binding socket to port %i..\n", _port);
	struct sockaddr_in server;
	WSADATA wsa;

	if (WSAStartup(0x0202, &wsa) != 0)
	{
		return false;
	}

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Invalid socket.\n");
		return false;
	}

	host = _host;
	port = _port;

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = (unsigned long)host;
	server.sin_port = htons(port);

	if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Could not bind socket.\n");
		return false;
	}

	printf("[WIN] Binded socket to port %i.\n", _port);
	connected = true;

	return true;
}

bool CPPolusServer::Listen()
{
	printf("[INF] Listening for messages..\n");

	BeginPing();

	char buf[BUFLEN];
	struct sockaddr_in remote;
	int slen = sizeof(remote);
	while (true)
	{
		memset(buf, 0, BUFLEN);
		if (recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr*)&remote, &slen) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			return false;
		}

		OnMessageReceived(remote, buf);
	}
}

bool CPPolusServer::BeginPing()
{
	if (!sock || !connected || pinging)
		return false;

	pinging = true;
	std::thread([this]()
	{
		while (pinging)
		{
			auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(1500);

			std::map<sockaddr_in, RemoteClient>::iterator it = remotes.begin();
			while (it != remotes.end())
			{
				BinaryWriter writer(3);
				writer.Write((char)Opcode_Ping);
				writer.Write(_byteswap_ushort(it->second.nonce()));

				SendTo(it->second, writer);
				it++;
			}

			std::this_thread::sleep_until(x);
		}

		std::terminate();
	}).detach();

	return true;
}

bool CPPolusServer::StopPing()
{
	if (!sock || !connected || !pinging)
		return false;

	pinging = false;

	return true;
}

bool CPPolusServer::IdentifyClient(RemoteClient& client, std::string username, int version)
{
	clientid_incr++;

	client.identified = true;
	client.username = username;
	client.version = version;

	printf("Identified %s as %s (Version %s)\n", getaddr(*client.remote).c_str(), username.c_str(), FormatVersion(version).c_str());

	return true;
}

bool CPPolusServer::OnMessageReceived(sockaddr_in& remote, char* bytes)
{
	BinaryReader reader(BUFLEN, bytes);

	char op;
	reader.Read(&op);

	RemoteClient client;
	if (!GetOrCreateClient(remote, &client))
		return false;

	switch (op)
	{
	case Opcode_Reliable: {
		unsigned short nonce;
		reader.Read(&nonce, true);
		Acknowledge(client, nonce);
	case Opcode_Unreliable:
		while (reader.left())
		{
			ParsePayload(client, reader);
		}
		break;
	}
	case Opcode_Hello: {
		printf("Rece1ived hello from %s:%d\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
		if (!client.identified)
		{
			unsigned short nonce;
			std::string username;
			int version;

			reader.Jump(0x01);
			reader.Read(&nonce);
			reader.Read(&version);
			reader.ReadString(&username);

			if (IdentifyClient(client, username, version))
			{
				Acknowledge(client, nonce);
			}
		}
		break;
	}
	case Opcode_Disconnect: {
		if (!client.sent_disconnect)
		{
			Disconnect(client, Disconnect_NoReason);
		}

		remotes.erase(remotes.find(remote));
		break;
	}
	case Opcode_Acknowledge: {
		unsigned short nonce;
		unsigned char missing;

		reader.Read(&nonce);
		reader.Read(&missing);

		SetAcknowledged(client, _byteswap_ushort(nonce));

		for (int i = 0; i < 8; i++)
		{
			if (((1 << i) & missing) == 0)
			{
				Acknowledge(client, nonce);
			}
		}
		break;
	}
	case Opcode_Ping: {
		unsigned short nonce;
		reader.Read(&nonce, true);

		Acknowledge(client, nonce);
		break;
	}
	}

	return true;
}

bool CPPolusServer::ParsePayload(RemoteClient& client, BinaryReader& reader)
{
	unsigned short payloadlen;
	unsigned char tag;

	reader.Read(&payloadlen);
	reader.Read(&tag);

	switch (tag)
	{
	case Tag_HostGame:
		GameSettings settings;
		if (ReadGameSettings(reader, &settings))
		{
			int code = V2GenerateCode();
			while (rooms.find(code) != rooms.end())
				code = V2GenerateCode();

			Room room{ code, settings, false };
			rooms.insert(std::make_pair(code, room));
		}
		break;
	case Tag_JoinGame:
		break;
	case Tag_StartGame:
		break;
	case Tag_RemoveGame:
		break;
	case Tag_RemovePlayer:
		break;
	case Tag_GameData:
	case Tag_GameDataTo:
		int code;
		int recipient;

		reader.Read(&code);
		if (tag == Tag_GameDataTo)
		{
			reader.ReadPackedInt32(&recipient);
		}

		break;
	case Tag_JoinedGame:
		break;
	case Tag_EndGame:
		break;
	case Tag_GetGameList:
		break;
	case Tag_AlterGame:
		break;
	case Tag_KickPlayer:
		break;
	case Tag_Redirect:
		break;
	case Tag_ReselectServer:
		break;
	case Tag_GetGameListV2:
		break;
	}

	return true;
}

bool CPPolusServer::SendTo(RemoteClient& client, const char* buf, unsigned int size)
{
	if (sendto(sock, buf, size, 0, (struct sockaddr*)&client.remote, sizeof(client.remote)) == SOCKET_ERROR)
		return false;

	return true;
}

bool CPPolusServer::SendTo(RemoteClient& client, BinaryWriter& writer)
{
	if (!SendTo(client, writer.data(), writer.size()))
		return false;

	return true;
}

/* bool CPPolusServer::SendAndWait(RemoteClient client, const char* buf, unsigned int size, unsigned short nonce)
{
	if (sendto(sock, buf, size, 0, (struct sockaddr*)&client.remote, sizeof(client.remote)) == SOCKET_ERROR)
		return false;

	return true;
}

bool CPPolusServer::SendAndWait(RemoteClient client, BinaryWriter writer, unsigned short nonce)
{
	if (!SendAndWait(client, writer.data(), writer.size(), nonce))
		return false;

	return true;
} */

bool CPPolusServer::Acknowledge(RemoteClient& client, unsigned short nonce)
{
	BinaryWriter writer(4);
	writer.Write((char)Opcode_Acknowledge);
	writer.Write(_byteswap_ushort(nonce));
	writer.Write(CalculateMissing(client));

	return SendTo(client, writer);
}

bool CPPolusServer::SetAcknowledged(RemoteClient& client, unsigned short nonce)
{
	for (unsigned int i = 0; i < min(client.sent.size(), 8); i++)
	{
		if (client.sent[i].nonce == nonce)
		{
			client.sent[i].received = true;
			return true;
		}
	}
	
	return false;
}

bool CPPolusServer::AppendSentReliable(RemoteClient& client, unsigned short nonce)
{
	client.sent.push_front(PendingTransmittedPacket{ nonce, false });

	if (client.sent.size() > 8)
	{
		client.sent.pop_back();
	}

	return true;
}

bool CPPolusServer::AppendReceivedReliable(RemoteClient& client, unsigned short nonce)
{
	client.received.push_front(PendingTransmittedPacket{ nonce, false });

	if (client.received.size() > 8)
	{
		client.received.pop_back();
	}

	return true;
}

unsigned char CPPolusServer::CalculateMissing(RemoteClient& client)
{
	unsigned char missing = 0;

	for (size_t i = 0; i < 8; i++)
	{
		if (i >= client.sent.size())
		{
			missing |= 1 << i;
		}
		else if (client.sent[i].received)
		{
			missing |= 1 << i;
		}
	}

	return missing;
}

bool CPPolusServer::Disconnect(RemoteClient& client, char reason, std::string message)
{
	BinaryWriter writer(1, true);
	writer.Write((char)0x09);

	if (reason != Disconnect_NoReason)
	{
		writer.Write((char)reason);

		if (reason == Disconnect_Custom)
		{
			writer.WriteString(message);
		}
	}

	return true;
}

bool CPPolusServer::GetOrCreateClient(sockaddr_in& remote, RemoteClient* client)
{
	if (remotes.count(remote) == 0)
	{
		clientid_incr++;

		if (clientid_incr < 0)
			clientid_incr = 1;

		*client = RemoteClient(&remote, clientid_incr);
		remotes.insert(std::make_pair(remote, *client));

		return true;
	}

	std::map<sockaddr_in,RemoteClient>::const_iterator pos = remotes.find(remote);

	if (pos == remotes.end())
		return false;

	*client = pos->second;

	return true;
}