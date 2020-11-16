#pragma once

#include "utils/BinaryReader.h"

struct GameSettings
{
	char version;
	char maxPlayers;
	unsigned int language;
	char mapID;
	float playerSpeed;
	float crewVision;
	float imposterVision;
	float killCooldown;
	char commonTasks;
	char longTasks;
	char shortTasks;
	int emergencies;
	char imposterCount;
	char killDistance;
	int discussionTime;
	int votingTime;
	bool isDefault;
	char emergencyCooldown;
	bool confirmEjects;
	bool visualTasks;
	bool anonymousVoting;
	bool taskBarUpdates;
};

bool ReadGameSettings(BinaryReader& reader, GameSettings* settings);