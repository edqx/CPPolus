#include "RemoteClient.h"

RemoteClient::RemoteClient()
{
	identified = false;
	sent_disconnect = false;

	_nonce = 0;
}

RemoteClient::RemoteClient(sockaddr_in* _remote, unsigned int _clientid)
{
	remote = _remote;
	clientid = _clientid;

	identified = false;
	sent_disconnect = false;

	_nonce = 0;
}

unsigned short RemoteClient::nonce()
{
	_nonce++;
	return _nonce;
}