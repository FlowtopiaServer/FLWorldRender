#include "RTTexLoader.h"
#include <zlib.h>

const int C_RTTEX_HEADER_SIZE = 124;
const int C_RTPACK_HEADER_SIZE = 32;
const int C_GL_UNSIGNED_BYTE = 5121;
const uint32 C_MAX_UNPACKED_SIZE = 64 * 1024 * 1024;

static int32 GetInt(const byte *p)
{
	int32 v;
	memcpy(&v, p, 4);
	return v;
}

bool UnpackRTPack(const vector<byte> &in, vector<byte> &out)
{
	if (in.size() < (size_t)C_RTPACK_HEADER_SIZE || memcmp(in.data(), "RTPACK", 6) != 0)
	{
		out = in;
		return true;
	}
	uint32 compressedSize = (uint32)GetInt(&in[8]);
	uint32 decompressedSize = (uint32)GetInt(&in[12]);
	if (compressedSize > in.size() - C_RTPACK_HEADER_SIZE || decompressedSize > C_MAX_UNPACKED_SIZE) return false;
	if (in[16] == 0)
	{
		out.assign(in.begin() + C_RTPACK_HEADER_SIZE, in.begin() + C_RTPACK_HEADER_SIZE + compressedSize);
		return true;
	}
	uLongf outSize = decompressedSize;
	out.resize(outSize);
	int result = uncompress(out.data(), &outSize, &in[C_RTPACK_HEADER_SIZE], compressedSize);
	out.resize(outSize);
	return result == Z_OK;
}

//rows are stored bottom-up and the canvas is padded to a power of two, we crop back to the original size
bool LoadRTTEXFromMemory(const byte *pData, size_t size, SoftSurface &surf)
{
	if (size < (size_t)C_RTTEX_HEADER_SIZE || memcmp(pData, "RTTXTR", 6) != 0) return false;
	int height = GetInt(pData + 8);
	int width = GetInt(pData + 12);
	int format = GetInt(pData + 16);
	int originalHeight = GetInt(pData + 20);
	int originalWidth = GetInt(pData + 24);
	bool bUsesAlpha = pData[28] != 0;
	if (format != C_GL_UNSIGNED_BYTE || pData[29] != 0) return false; //compressed/PVR textures not supported, never needed them yet
	int bpp = bUsesAlpha ? 4 : 3;
	if (size < (size_t)C_RTTEX_HEADER_SIZE + (size_t)width * height * bpp) return false;
	if (originalWidth > width || originalHeight > height) return false;

	surf.Init(originalWidth, originalHeight);
	byte *pOut = surf.GetPixelData();
	const byte *pPixels = pData + C_RTTEX_HEADER_SIZE;
	for (int y = 0; y < originalHeight; y++)
	{
		const byte *pRow = pPixels + (size_t)(height - 1 - y) * width * bpp;
		for (int x = 0; x < originalWidth; x++)
		{
			const byte *pSrc = pRow + (size_t)x * bpp;
			byte *pDst = pOut + ((size_t)y * originalWidth + x) * 4;
			pDst[0] = pSrc[0];
			pDst[1] = pSrc[1];
			pDst[2] = pSrc[2];
			pDst[3] = bUsesAlpha ? pSrc[3] : 255;
			//if (pDst[3] == 0) pDst[0] = pDst[1] = pDst[2] = 0; //premultiply test, made the edges darker than the client
		}
	}
	return true;
}

bool LoadRTTEX(const string &fileName, SoftSurface &surf)
{
	vector<byte> raw, unpacked;
	if (!LoadFileIntoMemory(fileName, raw) || !UnpackRTPack(raw, unpacked))
	{
		LogError("Can't load %s", fileName.c_str());
		return false;
	}
	if (!LoadRTTEXFromMemory(unpacked.data(), unpacked.size(), surf))
	{
		LogError("%s isn't an RTTEX we understand", fileName.c_str());
		return false;
	}
	return true;
}
