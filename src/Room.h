#pragma once

#include <map>

#include "GameSettings.h"
#include "ClientData.h"

struct Room
{
	int code;
	GameSettings settings;
	bool is_public;
	bool started;

	int hostid;

	std::map<int, ClientData> clients;
};