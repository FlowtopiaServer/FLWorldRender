#include "PNGWriter.h"
#include <zlib.h>

static void PutBigEndian(vector<byte> &out, uint32 v)
{
	out.push_back((byte)(v >> 24));
	out.push_back((byte)(v >> 16));
	out.push_back((byte)(v >> 8));
	out.push_back((byte)v);
}

static void WriteChunk(vector<byte> &file, const char *pType, const vector<byte> &data)
{
	PutBigEndian(file, (uint32)data.size());
	size_t typeStart = file.size();
	file.insert(file.end(), pType, pType + 4);
	file.insert(file.end(), data.begin(), data.end());
	uLong crc = crc32(0L, &file[typeStart], (uInt)(file.size() - typeStart));
	PutBigEndian(file, (uint32)crc);
}

//minimal PNG writer, RGBA 8 bit, one IDAT.  No libpng on purpose
bool SaveSurfaceAsPNG(const SoftSurface &surf, const string &fileName)
{
	int width = surf.GetWidth();
	int height = surf.GetHeight();
	vector<byte> raw;
	raw.reserve((size_t)(width * 4 + 1) * height);
	const byte *pPixels = surf.GetPixelData();
	for (int y = 0; y < height; y++)
	{
		raw.push_back(0);
		raw.insert(raw.end(), pPixels + (size_t)y * width * 4, pPixels + (size_t)(y + 1) * width * 4);
	}
	uLongf packedSize = compressBound(raw.size());
	vector<byte> packed(packedSize);
	if (compress2(packed.data(), &packedSize, raw.data(), raw.size(), 9) != Z_OK) return false;
	packed.resize(packedSize);

	vector<byte> file = { 0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a };
	vector<byte> header;
	PutBigEndian(header, width);
	PutBigEndian(header, height);
	header.insert(header.end(), { 8, 6, 0, 0, 0 });
	WriteChunk(file, "IHDR", header);
	WriteChunk(file, "IDAT", packed);
	WriteChunk(file, "IEND", vector<byte>());

	FILE *fp = fopen(fileName.c_str(), "wb");
	if (!fp) return false;
	bool bOk = fwrite(file.data(), 1, file.size(), fp) == file.size();
	fclose(fp);
	return bOk;
}
