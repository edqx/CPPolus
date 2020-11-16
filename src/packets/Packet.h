#pragma once

#include "../utils/BinaryWriter.h"

class Packet : BinaryWriter
{
	char opcode;
public:
	Packet(char _opcode);

	unsigned char* data();
};