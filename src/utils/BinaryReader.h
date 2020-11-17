#pragma once

#include <intrin.h>
#include <string>

#include "SwitchBytes.h"

class BinaryReader
{
public:
	char* data;
	size_t size;
	size_t cursor;

	BinaryReader(size_t _size, char* _data);

	size_t left();

	bool Goto(size_t pos);
	bool Jump(size_t bytes);

	bool ReadString(std::string* val);

	template<typename T>
	bool Read(T* val, bool bigendian = false)
	{
		if (cursor + sizeof T > size)
			return false;

#if IS_BIG_ENDIAN
		if (bigendian)
		{
			*val = *(T*)&_bytes[_cursor];
		}
		else
		{
			*val = switchbytes(*(T*)&_bytes[_cursor]);
		}
#else
		if (bigendian)
		{
			*val = switchbytes(*(T*)&data[cursor]);
		}
		else
		{
			*val = *(T*)&data[cursor];
		}
#endif
		cursor += sizeof T;

		return true;
	}

	bool ReadPackedInt32(int* val);
	bool ReadMessage(size_t len, char* buf);
};