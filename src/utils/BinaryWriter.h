#pragma once

#include <intrin.h>
#include <string>

#include "SwitchBytes.h"

class BinaryWriter
{
	size_t original_sz;
	bool _dynamic;
public:
	unsigned char* data;
	size_t size;
	size_t cursor;
	size_t written;

	BinaryWriter(size_t _size, bool dynamic = false);

	bool clear();

	bool Realloc(size_t size);
	bool Expand(size_t bytes);
	bool FitToSize();

	bool Goto(size_t pos);
	bool Jump(size_t bytes);

	bool WriteString(std::string string);
	bool Write(BinaryWriter writer);

	template<typename T>
	bool Write(T val, bool bigendian = false)
	{
		if (!Expand(sizeof T))
			return false;

#if IS_BIG_ENDIAN
		if (bigendian)
		{
			memcpy(data + cursor, &val, sizeof T);
		}
		else
		{
			T switched = switchbytes(val);
			memcpy(data + cursor, &switched, sizeof T);
		}
#else
		if (bigendian)
		{
			T switched = switchbytes(val);
			memcpy(data + cursor, &switched, sizeof T);
		}
		else
		{
			memcpy(data + cursor, &val, sizeof T);
		}
#endif
		cursor += sizeof T;
		written += sizeof T;

		return true;
	}

	bool WritePackedInt32(int val);
};