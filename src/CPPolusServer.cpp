#include "CPPolusServer.h"

#define BUFLEN 512
#define PING_INTERVAL 1000

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
	_connected = false;
	host = 0;
	port = 22023;
	_clientid_incr = 0;
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
	_connected = true;

	return true;
}

bool CPPolusServer::Listen()
{
	printf("[INF] Listening for messages..\n");

	BeginPing();


	std::thread([this]()
	{
		while (_connected)
		{

		}
	}).detach();

	char buf[BUFLEN];
	struct sockaddr_in remote;
	int slen = sizeof(remote);
	while (true)
	{
		int blen = recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr*)&remote, &slen);

		if (blen == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			if (err == 10054)
			{
				RemoteClient client;
				GetOrCreateClient(remote, &client);
				free(client.writer.data);
				remotes.erase(remotes.find(remote));
				printf("Received socket close from %s\n", getaddr(remote).c_str());
			}
			else
			{
				printf("recvfrom() failed with error code : %d\n", WSAGetLastError());
			}
			continue;
		}

		OnMessageReceived(remote, buf, blen);
	}
}

bool CPPolusServer::BeginPing()
{
	if (!sock || !_connected)
		return false;

	std::thread([this]()
	{
		while (_connected)
		{
			auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(PING_INTERVAL);

			std::map<sockaddr_in, RemoteClient>::iterator it = remotes.begin();
			while (it != remotes.end())
			{
				it->second.writer.BeginPacket(Opcode_Ping);
				it->second.writer.Write(it->second.nonce(), true);

				SendTo(&it->second);
				it++;
			}

			std::this_thread::sleep_until(x);
		}
	}).detach();

	return true;
}

bool CPPolusServer::IdentifyClient(RemoteClient* client, std::string username, int version)
{
	_clientid_incr++;

	client->identified = true;
	client->username = username;
	client->version = version;

	printf("Identified %s as %s (Version %s)\n", getaddr(*client->remote).c_str(), username.c_str(), FormatVersion(version).c_str());

	return true;
}

bool CPPolusServer::OnMessageReceived(sockaddr_in& remote, char* bytes, int bytes_received)
{
	BinaryReader reader(bytes_received, bytes);

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
		Acknowledge(&client, nonce);
	case Opcode_Unreliable:
		while (reader.left())
		{
			unsigned short payloadlen;
			unsigned char tag;

			reader.Read(&payloadlen);
			reader.Read(&tag);

			char* message = (char*)malloc(payloadlen);

			if (message)
			{
				size_t payloadstart = reader.cursor;

				if (reader.ReadMessage(payloadlen, message))
				{
					BinaryReader msgreader(payloadlen, message);

					ParsePayload(&client, payloadlen, tag, msgreader);
				}

				reader.Goto(payloadstart + payloadlen);
				free(message);
			}
		}
		break;
	}
	case Opcode_Hello: {
		printf("Received hello from %s:%d\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
		if (!client.identified)
		{
			unsigned short nonce;
			std::string username;
			int version;

			reader.Jump(0x01);
			reader.Read(&nonce);
			reader.Read(&version);
			reader.ReadString(&username);

			if (IdentifyClient(&client, username, version))
			{
				Acknowledge(&client, nonce);
			}
		}
		break;
	}
	case Opcode_Disconnect: {
		if (!client.sent_disconnect)
		{
			Disconnect(&client, Disconnect_NoReason);
		}

		free(client.writer.data);
		remotes.erase(remotes.find(remote));
		break;
	}
	case Opcode_Acknowledge: {
		unsigned short nonce;
		unsigned char missing;

		reader.Read(&nonce, true);
		reader.Read(&missing);

		SetAcknowledged(&client, nonce);

		for (size_t i = 0; i < 8; i++)
		{
			if (i < client.sent.size() &&
				((1 << i) & missing) == 0)
			{
				PendingTransmittedPacket sent = client.sent[i];

				Acknowledge(&client, sent.nonce);
			}
		}
		break;
	}
	case Opcode_Ping: {
		unsigned short nonce;
		reader.Read(&nonce, true);

		Acknowledge(&client, nonce);
		break;
	}
	}

	return true;
}

bool CPPolusServer::ParsePayload(RemoteClient* client, unsigned short payloadlen, unsigned short tag, BinaryReader& reader)
{
	switch (tag)
	{
	case Tag_HostGame:
	{
		GameSettings settings;
		if (ReadGameSettings(reader, &settings))
		{
			int code = V2GenerateCode();
			while (rooms.find(code) != rooms.end())
				code = V2GenerateCode();

			Room room{ code, settings, false };
			rooms.insert(std::make_pair(code, room));

			client->writer.BeginPacket(Opcode_Reliable);
			client->writer.Write(client->nonce(), true);
			client->writer.BeginPayload(Tag_HostGame);
			client->writer.Write(code);
			client->writer.End();

			SendTo(client);
		}
		break;
	}
	case Tag_JoinGame:
	{
		int code;
		char mapOwnership;

		reader.Read(&code);
		reader.Read(&mapOwnership);

		std::map<int, Room>::iterator found_room_it = rooms.find(code);

		if (found_room_it == rooms.end())
			return JoinError(client, Disconnect_GameNotFound);

		Room& found_room = found_room_it->second;

		if (found_room.clients.size() >= found_room.settings.maxPlayers)
			return JoinError(client, Disconnect_GameFull);

		if (found_room.started)
			return JoinError(client, Disconnect_GameStarted);

		found_room.clients.insert(std::make_pair(client->clientid, ClientData{ client }));

		if (found_room.clients.size() == 1)
		{
			found_room.hostid = client->clientid;
		}

		BinaryWriter joinGameWriter(12);
		joinGameWriter.Write(code);
		joinGameWriter.Write(client->clientid);
		joinGameWriter.Write(found_room.hostid);

		client->writer.BeginPacket(Opcode_Reliable);
		client->writer.Write(client->nonce(), true);
		client->writer.BeginPayload(Tag_JoinedGame);
		client->writer.Write(code);
		client->writer.Write(client->clientid);
		client->writer.Write(found_room.hostid);
		client->writer.WritePackedInt32(found_room.clients.size() - 1);

		for (std::map<int, ClientData>::iterator it = found_room.clients.begin(); it != found_room.clients.end(); it++)
		{
			RemoteClient* send_client = it->second.client;
			if (send_client->clientid != client->clientid)
			{
				send_client->writer.BeginPacket(Opcode_Reliable);
				send_client->writer.Write(it->second.client->nonce(), true);
				send_client->writer.BeginPayload(Tag_JoinGame);
				send_client->writer.Write(joinGameWriter);
				send_client->writer.End();

				SendTo(it->second.client);

				client->writer.WritePackedInt32(send_client->clientid);
			}
		}

		client->writer.End();
		client->writer.BeginPayload(Tag_AlterGame);
		client->writer.Write(code);
		client->writer.Write((char)1);
		client->writer.Write(found_room.is_public);
		client->writer.End();

		SendTo(client);
		break;
	}
	case Tag_StartGame:
	{
		break;
	}
	case Tag_RemoveGame:
		break;
	case Tag_RemovePlayer:
		break;
	case Tag_GameData:
	case Tag_GameDataTo:
	{
		int code;
		int recipient;

		reader.Read(&code);
		if (tag == Tag_GameDataTo)
		{
			reader.ReadPackedInt32(&recipient);
		}

		break;
	}
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

bool CPPolusServer::SendTo(RemoteClient* client, unsigned char* buf, unsigned int size)
{
	if (sendto(sock, reinterpret_cast<const char*>(buf), size, 0, (struct sockaddr*)client->remote, sizeof(*client->remote)) == SOCKET_ERROR)
	{
		printf("Error sending packet: %i\n", WSAGetLastError());
		return false;
	}

	return true;
}

bool CPPolusServer::SendTo(RemoteClient* client)
{
	client->writer.FitToSize();

	if (!SendTo(client, client->writer.data, client->writer.size))
	{
		client->writer.clear();
		return false;
	}

	client->writer.clear();
	return true;
}

bool CPPolusServer::SendRepeat(RemoteClient* client, unsigned char* buf, size_t size, unsigned short nonce)
{
	if (!(SendTo(client, buf, size)))
		return false;

	PendingTransmittedPacket packet = AppendSentReliable(client, nonce);
	pending.push_back(std::make_pair(*client, packet));

	return true;
}

bool CPPolusServer::SendRepeat(RemoteClient* client, unsigned short nonce)
{
	client->writer.FitToSize();

	if (!SendRepeat(client, client->writer.data, client->writer.size, nonce))
	{
		client->writer.clear();
		return false;
	}

	client->writer.clear();
	return true;
}

bool CPPolusServer::Acknowledge(RemoteClient* client, unsigned short nonce)
{
	client->writer.BeginPacket(Opcode_Acknowledge);
	client->writer.Write(nonce, true);
	client->writer.Write(CalculateMissing(client));

	return SendTo(client);
}

bool CPPolusServer::SetAcknowledged(RemoteClient* client, unsigned short nonce)
{
	for (unsigned int i = 0; i < min(client->sent.size(), 8); i++)
	{
		if (client->sent[i].nonce == nonce)
		{
			client->sent[i].received = true;
			return true;
		}
	}
	
	return false;
}

PendingTransmittedPacket CPPolusServer::AppendSentReliable(RemoteClient* client, unsigned short nonce)
{
	PendingTransmittedPacket packet{ nonce, false };
	client->sent.push_front(packet);

	if (client->sent.size() > 8)
	{
		client->sent.pop_back();
	}

	return packet;
}

PendingTransmittedPacket CPPolusServer::AppendReceivedReliable(RemoteClient* client, unsigned short nonce)
{
	PendingTransmittedPacket packet{ nonce, false };
	client->received.push_front(packet);

	if (client->received.size() > 8)
	{
		client->received.pop_back();
	}

	return packet;
}

unsigned char CPPolusServer::CalculateMissing(RemoteClient* client)
{
	unsigned char missing = 0;

	for (size_t i = 0; i < 8; i++)
	{
		if (i >= client->sent.size())
		{
			missing |= 1 << i;
		}
		else if (client->sent[i].received)
		{
			missing |= 1 << i;
		}
	}

	return missing;
}

bool CPPolusServer::Disconnect(RemoteClient* client, char reason, std::string message)
{
	client->writer.BeginPacket(Opcode_Disconnect);

	if (reason != Disconnect_NoReason)
	{
		client->writer.Write((char)reason);

		if (reason == Disconnect_Custom)
			client->writer.WriteString(message);
	}

	SendTo(client);

	return true;
}

bool CPPolusServer::JoinError(RemoteClient* client, char reason)
{
	client->writer.BeginPacket(Opcode_Reliable);
	client->writer.Write(client->nonce(), true);
	client->writer.BeginPayload(Tag_JoinGame);

	if (reason != Disconnect_NoReason)
	{
		client->writer.Write((char)reason);
		for (int i = 0; i < 3; i++) client->writer.Write(0);
	}

	SendTo(client);

	return true;
}

bool CPPolusServer::GetOrCreateClient(sockaddr_in& remote, RemoteClient* client)
{
	if (remotes.count(remote) == 0)
	{
		_clientid_incr++;

		if (_clientid_incr < 0)
			_clientid_incr = 1;

		*client = RemoteClient(&remote, _clientid_incr);
		remotes.insert(std::make_pair(remote, *client));

		return true;
	}

	std::map<sockaddr_in,RemoteClient>::const_iterator pos = remotes.find(remote);

	if (pos == remotes.end())
		return false;

	*client = pos->second;

	return true;
}