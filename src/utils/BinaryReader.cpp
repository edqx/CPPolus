#include "BinaryReader.h"

BinaryReader::BinaryReader(size_t size, char* bytes)
{
	_bytes = bytes;
	_size = size;
	_cursor = 0;
}

char* BinaryReader::data()
{
	return _bytes;
}

size_t BinaryReader::size()
{
	return _size;
}

size_t BinaryReader::left()
{
	return _size - _cursor;
}


bool BinaryReader::Goto(size_t pos)
{
	if (pos >= _size)
		return false;

	_cursor = pos;

	return true;
}

bool BinaryReader::Jump(size_t bytes)
{
	if (_cursor + bytes > _size)
		return false;

	_cursor += bytes;

	return true;
}

bool BinaryReader::ReadString(std::string* val)
{
	int length;
	
	if (!ReadPackedInt32(&length))
		return false;

	if (_cursor + length > _size)
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