#include "PacketWriter.h"

bool PacketWriter::BeginPacket(char opcode)
{
	return Write(opcode);
}

bool PacketWriter::BeginPayload(char tag)
{
	len_stack.push_back(cursor);

	if (!Jump(0x02))
		return false;

	return Write(tag);
}

bool PacketWriter::BeginMessage(char type)
{
	len_stack.push_back(cursor);

	if (!Jump(0x02))
		return false;

	return Write(type);
}

bool PacketWriter::End()
{
	if (len_stack.size() < 1)
		return false;

	size_t curpos = cursor;
	size_t stackpos = len_stack.back();
	if (!Goto(stackpos))
		return false;

	if (!Write((short)(curpos - stackpos)))
		return false;

	return Goto(curpos);
}