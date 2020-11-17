#include "BinaryReader.h"

#define max(a, b) ((a) >= (b) ? (a) : (b))

BinaryReader::BinaryReader(size_t _size, char* _data)
{
	data = _data;
	size = _size;
	cursor = 0;
}

size_t BinaryReader::left()
{
	return max(size - cursor, 0);
}


bool BinaryReader::Goto(size_t pos)
{
	if (pos > size)
		return false;

	cursor = pos;

	return true;
}

bool BinaryReader::Jump(size_t bytes)
{
	if (cursor + bytes > size)
		return false;

	cursor += bytes;

	return true;
}

bool BinaryReader::ReadString(std::string* val)
{
	int length;
	
	if (!ReadPackedInt32(&length))
		return false;

	if (cursor + length > size)
		return false;

	std::string result;

	for (int i = 0; i < length; i++) {
		char byte;
		if (!Read(&byte))
			return false;

		result += byte;
	}

	*val = result;

	return true;
}

bool BinaryReader::ReadPackedInt32(int* val)
{
	*val = 0;

	int shift = 0;
	bool doRead = true;

	while (doRead)
	{
		char byte;
		if (!Read(&byte))
			return false;

		if (byte & 0x80)
		{
			byte ^= 0x80;
		} else
		{
			doRead = false;
		}

		*val |= (byte << shift);
		shift += 7;
	}

	return true;
}

bool BinaryReader::ReadMessage(size_t len, char* buf)
{
	if (!buf)
		return false;
	
	memcpy(buf, &data[cursor], len);
	cursor += len;

	return true;
}