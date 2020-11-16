#pragma once

#include <intrin.h>
#include <string>

#include "SwitchBytes.h"

class BinaryReader
{
public:
	char* _bytes;
	size_t _size;
	size_t _cursor;

	BinaryReader(size_t size, char* bytes);

	char* data();
	size_t size();
	size_t left();

	bool Goto(size_t pos);
	bool Jump(size_t bytes);

	bool ReadString(std::string* val);

	template<typename T>
	bool Read(T* val, bool bigendian = false)
	{
		if (_cursor + sizeof T > _size)
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
			*val = switchbytes(*(T*)&_bytes[_cursor]);
		}
		else
		{
			*val = *(T*)&_bytes[_cursor];
		}
#endif
		_cursor += sizeof T;

		return true;
	}

	bool ReadPackedInt32(int* val);
};