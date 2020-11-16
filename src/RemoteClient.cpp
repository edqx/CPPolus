#include "RemoteClient.h"

RemoteClient::RemoteClient()
{
	remote = NULL;
	clientid = 0;
	version = 0;

	identified = false;
	sent_disconnect = false;

	_nonce = 0;
}

RemoteClient::RemoteClient(sockaddr_in* _remote, unsigned int _clientid)
{
	remote = _remote;
	clientid = _clientid;
	version = 0;

	identified = false;
	sent_disconnect = false;

	_nonce = 0;
}

unsigned short RemoteClient::nonce()
{
	_nonce++;
	return _nonce;
}