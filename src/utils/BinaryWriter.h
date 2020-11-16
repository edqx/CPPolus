#pragma once

#include <intrin.h>
#include <string>

#include "SwitchBytes.h"

class BinaryWriter
{
	bool _dynamic;
public:
	char* _bytes;
	int _size;
	int _cursor;

	BinaryWriter(int size, bool dynamic = false);

	bool Expand(int bytes);

	int size();
	bool dynamic();
	char* data();

	bool Goto(int pos);
	bool Jump(int bytes);

	bool WriteString(std::string string);
	bool Write(BinaryWriter writer);

	template<typename T>
	bool Write(T val)
	{
		if (!Expand(sizeof T))
			return false;

#if IS_BIG_ENDIAN
		memcpy(_bytes + _cursor, switchbytes(&val), sizeof T);
#else
		memcpy(_bytes + _cursor, &val, sizeof T);
#endif
		_cursor += sizeof T;

		return true;
	}

	bool WritePackedInt32(int val);
};