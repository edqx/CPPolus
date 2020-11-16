#include "Versions.h"

int EncodeVersion(int year, int month, int day, int build)
{
	return EncodeVersion(VersionInfo{ year, month, day, build });
}

int EncodeVersion(VersionInfo version)
{
	return (version.year * 25000) +
		(version.month * 1800) +
		(version.day * 50) +
		version.build;
}

VersionInfo DecodeVersion(int version)
{
	VersionInfo info{};

	info.year = floor(version / 25000);
	version %= 25000;
	info.month = floor(version / 1800);
	version %= 1800;
	info.day = floor(version / 50);
	info.build = version % 50;

	return info;
}

std::string FormatVersion(int year, int month, int day, int build)
{
	return FormatVersion(VersionInfo{ year, month, day, build });
}

std::string FormatVersion(VersionInfo version)
{
	std::stringstream strm;
	strm << version.year << "." << version.month << "." << version.day << "." << version.build;

	return std::string(strm.str());
}

std::string FormatVersion(int version)
{
	return FormatVersion(DecodeVersion(version));
}