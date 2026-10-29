#pragma once

#include "PlatformPrecomp.h"

struct glColorBytes
{
	glColorBytes() : r(255), g(255), b(255), a(255) {}
	glColorBytes(byte red, byte green, byte blue, byte alpha) : r(red), g(green), b(blue), a(alpha) {}
	byte r, g, b, a;
};

struct rtRectf
{
	rtRectf() : left(0), top(0), right(0), bottom(0) {}
	rtRectf(float l, float t, float r, float b) : left(l), top(t), right(r), bottom(b) {}
	float GetWidth() const { return right - left; }
	float GetHeight() const { return bottom - top; }
	float left, top, right, bottom;
};

class SoftSurface
{
public:

	SoftSurface();

	bool Init(int width, int height);
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	bool IsActive() const { return m_width > 0; }
	byte * GetPixelData() { return m_pixels.data(); }
	const byte * GetPixelData() const { return m_pixels.data(); }
	void SetPixelData(int width, int height, const byte *pRGBA);

	void FillColor(glColorBytes color);
	void FillRect(const rtRectf &dst, glColorBytes color);
	void BlitEx(const SoftSurface &src, const rtRectf &srcRect, const rtRectf &dst, glColorBytes tint, bool bFlipX = false, bool bNearest = false);
	void BlitScaled(const SoftSurface &src, float x, float y, float scale, glColorBytes tint = glColorBytes());
	//nearest-sampled 1:1 blit turned by angleDeg around (pivotX, pivotY) in target space
	void BlitRotated(const SoftSurface &src, const rtRectf &srcRect, float left, float top, float pivotX, float pivotY, float angleDeg, glColorBytes tint);

private:

	void SampleLinear(float u, float v, float out[4]) const;
	void SampleNearest(float u, float v, float out[4]) const;
	void BlendPixel(int x, int y, const float src[4]);

	int m_width;
	int m_height;
	vector<byte> m_pixels;
};
