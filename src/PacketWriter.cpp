#include "PacketWriter.h"

bool PacketWriter::BeginPacket(char opcode)
{
	return Write(opcode);
}

bool PacketWriter::BeginPayload(char tag)
{
	if (!Jump(0x02))
		return false;

	len_stack.push_back(cursor);

	return Write(tag);
}

bool PacketWriter::BeginMessage(char type)
{
	if (!Jump(0x02))
		return false;

	len_stack.push_back(cursor);

	return Write(type);
}

bool PacketWriter::End()
{
	if (len_stack.size() < 1)
		return false;

	size_t curpos = cursor;
	size_t stackpos = len_stack.back();
	if (!Goto(stackpos - 2))
		return false;

	if (!Write((short)(curpos - stackpos - 1)))
		return false;

	return Goto(curpos);
}