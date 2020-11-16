#pragma once

#define IS_LITTLE_ENDIAN (1 == *(unsigned char*)&(const int){1})

template<typename T>
T switchbytes(T val)
{
	char* temp = (char*)malloc(sizeof T);
	if (temp)
	{
		memcpy(temp, &val, sizeof T);
		std::reverse(temp, temp + sizeof(val));

		return *(T*)temp;
	}
	else
	{
		return 0;
	}
}