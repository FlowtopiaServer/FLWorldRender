#include "PlatformPrecomp.h"
#include <cstdarg>

void LogMsg(const char *pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vfprintf(stdout, pFormat, args);
	va_end(args);
	fputc('\n', stdout);
}

void LogError(const char *pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	fputs("Error: ", stderr);
	vfprintf(stderr, pFormat, args);
	va_end(args);
	fputc('\n', stderr);
}

bool LoadFileIntoMemory(const string &fileName, vector<byte> &out)
{
	FILE *fp = fopen(fileName.c_str(), "rb");
	if (!fp) return false;
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	out.resize(size > 0 ? size : 0);
	bool bOk = size <= 0 || fread(out.data(), 1, size, fp) == (size_t)size;
	fclose(fp);
	return bOk;
}
