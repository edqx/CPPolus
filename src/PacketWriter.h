#pragma once

#include <vector>

#include "utils/BinaryWriter.h"

class PacketWriter : public BinaryWriter
{
	std::vector<size_t> len_stack;
public:
	PacketWriter() : BinaryWriter(0, true) {};
	PacketWriter(size_t size) : BinaryWriter(size, true) {};

	bool BeginPacket(char opcode);
	bool BeginPayload(char tag);
	bool BeginMessage(char type);

	bool End();
};