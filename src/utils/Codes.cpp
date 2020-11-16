#include "Codes.h"

int V1Code2Int(std::string codestr)
{
	return *(int*)codestr.c_str();
}

std::string V1Int2Code(int codeint)
{
	const char* codestr = (const char*)&codeint;

	std::stringstream code;
	code << codestr[0] << codestr[1] << codestr[2] << codestr[3];

	return std::string(code.str());
}

int V1GenerateCode()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<> dis(0, 25);

	char code[4];
	code[0] = 65 + dis(gen);
	code[1] = 65 + dis(gen);
	code[2] = 65 + dis(gen);
	code[3] = 65 + dis(gen);

	return *(int*)&code[0];
}

static const char* V2Chars = "QWXRTYLPESDFGHUJKZOCVBINMA";
static char V2CharMap[] = { 25, 21, 19, 10, 8, 11, 12, 13, 22, 15, 16, 6, 24, 23, 18, 7, 0, 3, 9, 4, 14, 20, 1, 2, 5, 17 };

int V2Code2Int(std::string codestr)
{
	char a = V2CharMap[codestr[0] - 65];
	char b = V2CharMap[codestr[1] - 65];
	char c = V2CharMap[codestr[2] - 65];
	char d = V2CharMap[codestr[3] - 65];
	char e = V2CharMap[codestr[4] - 65];
	char f = V2CharMap[codestr[5] - 65];

	int one = (a + 26 * b) & 0x3FF;
	int two = (c + 26 * (d + 26 * (e + 26 * f)));

	return (one | ((two << 10) & 0x3FFFFC00) | 0x80000000);
}

std::string V2Int2Code(int codeint)
{
	int a = codeint & 0x3FF;
	int b = (codeint >> 10) & 0xFFFFF;

	std::stringstream code;
	code << V2Chars[a % 26] << V2Chars[a / 26] << V2Chars[b % 26]
		<< V2Chars[b / 26 % 26] << V2Chars[b / (26 * 26) % 26] << V2Chars[b / (26 * 26 * 26) % 26];

	return std::string(code.str());
}

int V2GenerateCode()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<> dis(0, 25);

	char code[6];
	code[0] = 65 + dis(gen);
	code[1] = 65 + dis(gen);
	code[2] = 65 + dis(gen);
	code[3] = 65 + dis(gen);
	code[4] = 65 + dis(gen);
	code[5] = 65 + dis(gen);

	return V2Code2Int(code);
}