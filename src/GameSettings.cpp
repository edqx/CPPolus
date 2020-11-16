#include "GameSettings.h"

bool ReadGameSettings(BinaryReader& reader, GameSettings* settings)
{
	size_t length;
	reader.ReadPackedInt32((int*)&length);

	if (reader.left() < length)
		return false;

	reader.Read(&settings->version);
	reader.Read(&settings->maxPlayers);
	reader.Read(&settings->language);
	reader.Read(&settings->mapID);
	reader.Read(&settings->playerSpeed);
	reader.Read(&settings->crewVision);
	reader.Read(&settings->imposterVision);
	reader.Read(&settings->killCooldown);
	reader.Read(&settings->commonTasks);
	reader.Read(&settings->longTasks);
	reader.Read(&settings->shortTasks);
	reader.Read(&settings->emergencies);
	reader.Read(&settings->imposterCount);
	reader.Read(&settings->killDistance);
	reader.Read(&settings->discussionTime);
	reader.Read(&settings->votingTime);
	reader.Read(&settings->isDefault);

	if (settings->version >= 2)
	{
		reader.Read(&settings->emergencyCooldown);
	}

	if (settings->version >= 3)
	{
		reader.Read(&settings->confirmEjects);
		reader.Read(&settings->visualTasks);
	}

	if (settings->version >= 4)
	{
		reader.Read(&settings->anonymousVoting);
		reader.Read(&settings->taskBarUpdates);
	}

	return true;
}