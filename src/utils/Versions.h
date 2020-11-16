#pragma once

#include <string>
#include <sstream>

struct VersionInfo
{
	int year;
	int month;
	int day;
	int build;
};

int EncodeVersion(int year, int month, int day, int build);
int EncodeVersion(VersionInfo version);
VersionInfo DecodeVersion(int version);
std::string FormatVersion(VersionInfo version);
std::string FormatVersion(int version);