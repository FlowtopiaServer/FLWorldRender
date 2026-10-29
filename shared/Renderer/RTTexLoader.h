#pragma once

#include "SoftSurface.h"

bool UnpackRTPack(const vector<byte> &in, vector<byte> &out);
bool LoadRTTEXFromMemory(const byte *pData, size_t size, SoftSurface &surf);
bool LoadRTTEX(const string &fileName, SoftSurface &surf);
