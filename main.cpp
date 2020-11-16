#include <iostream>

#include "src/CPPolusServer.h"

int main(int arc, const char* argv[])
{
	CPPolusServer& server = CPPolusServer::GetInstance();

	int host = 0;

	server.Bind(22023, (char*)host);
	server.Listen();

	return 0;
} 