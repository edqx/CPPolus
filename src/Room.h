#pragma once

#include <map>

#include "GameSettings.h"
#include "ClientData.h"

struct Room
{
	int code;
	GameSettings settings;
	bool is_public;

	std::map<int, ClientData> clients;
};