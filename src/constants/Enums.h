#pragma once

enum PacketOpcode
{
	Opcode_Unreliable,
	Opcode_Reliable,
	Opcode_Hello = 8,
	Opcode_Disconnect,
	Opcode_Acknowledge,
	Opcode_Ping = 12
};

enum DisconnectReason
{
	Disconnect_NoReason = -1,
	Disconnect_None,
	Disconnect_GameFull,
	Disconnect_GameStarted,
	Disconnect_GameNotFound,
	Disconnect_IncorrectVersion = 5,
	Disconnect_Banned,
	Disconnect_Kicked,
	Disconnect_Custom,
	Disconnect_InvalidName,
	Disconnect_Hacking,
	Disconnect_Destroy = 16,
	Disconnect_Error,
	Disconnect_IncorrectGame,
	Disconnect_ServerRequest,
	Disconnect_ServerFull,
	Disconnect_FocusLostBackground = 207,
	Disconnect_IntentionalLeaving = 208,
	Disconnect_FocusLost = 209,
	Disconnect_NewConnection = 210
};

enum Tag
{
	Tag_HostGame = 0,
	Tag_JoinGame = 1,
	Tag_StartGame = 2,
	Tag_RemoveGame = 3,
	Tag_RemovePlayer = 4,
	Tag_GameData = 5,
	Tag_GameDataTo = 6,
	Tag_JoinedGame = 7,
	Tag_EndGame = 8,
	Tag_GetGameList = 9,
	Tag_AlterGame = 10,
	Tag_KickPlayer = 11,
	Tag_Redirect = 13,
	Tag_ReselectServer = 14,
	Tag_GetGameListV2 = 16,
};

enum MapBitfield
{
	MapBit_TheSkeld = 1,
	MapBit_MiraHQ = 2,
	MapBit_Polus = 4
};

enum Maps
{
	Map_TheSkeld = 1,
	Map_MiraHQ = 2,
	Map_Polus = 3
};