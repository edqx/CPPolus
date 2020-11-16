#pragma once

#define IS_LITTLE_ENDIAN (1 == *(unsigned char*)&(const int){1})

template<typename T>
T switchbytes(T val)
{
	char* temp = (char*)&val;
	std::reverse(temp, temp + sizeof(val));

	return *(T*)temp;
}