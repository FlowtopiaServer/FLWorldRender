#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

typedef uint8_t byte;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int32_t int32;

using std::string;
using std::vector;

void LogMsg(const char *pFormat, ...);
void LogError(const char *pFormat, ...);
bool LoadFileIntoMemory(const string &fileName, vector<byte> &out);
