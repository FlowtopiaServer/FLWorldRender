#include "SoftSurface.h"

static inline byte ToByte(float f)
{
	if (f <= 0) return 0;
	if (f >= 255) return 255;
	return (byte)(f + 0.5f);
}

SoftSurface::SoftSurface() : m_width(0), m_height(0)
{
}

bool SoftSurface::Init(int width, int height)
{
	if (width <= 0 || height <= 0) return false;
	m_width = width;
	m_height = height;
	m_pixels.assign((size_t)width * height * 4, 0);
	return true;
}

void SoftSurface::SetPixelData(int width, int height, const byte *pRGBA)
{
	Init(width, height);
	memcpy(m_pixels.data(), pRGBA, m_pixels.size());
}

void SoftSurface::FillColor(glColorBytes color)
{
	for (size_t i = 0; i < m_pixels.size(); i += 4)
	{
		m_pixels[i] = color.r;
		m_pixels[i + 1] = color.g;
		m_pixels[i + 2] = color.b;
		m_pixels[i + 3] = color.a;
	}
}

void SoftSurface::BlendPixel(int x, int y, const float src[4])
{
	byte *pDst = &m_pixels[((size_t)y * m_width + x) * 4];
	float alpha = src[3] / 255.0f;
	float inv = 1.0f - alpha;
	pDst[0] = ToByte(src[0] * alpha + pDst[0] * inv);
	pDst[1] = ToByte(src[1] * alpha + pDst[1] * inv);
	pDst[2] = ToByte(src[2] * alpha + pDst[2] * inv);
	pDst[3] = ToByte(src[3] + pDst[3] * inv);
}

void SoftSurface::FillRect(const rtRectf &dst, glColorBytes color)
{
	int x0 = std::max(0, (int)std::ceil(dst.left - 0.5f));
	int y0 = std::max(0, (int)std::ceil(dst.top - 0.5f));
	int x1 = std::min(m_width, (int)std::ceil(dst.right - 0.5f));
	int y1 = std::min(m_height, (int)std::ceil(dst.bottom - 0.5f));
	float src[4] = { (float)color.r, (float)color.g, (float)color.b, (float)color.a };
	for (int y = y0; y < y1; y++)
	{
		for (int x = x0; x < x1; x++)
		{
			BlendPixel(x, y, src);
		}
	}
}

void SoftSurface::SampleLinear(float u, float v, float out[4]) const
{
	float fx = u - 0.5f;
	float fy = v - 0.5f;
	int ix = (int)std::floor(fx);
	int iy = (int)std::floor(fy);
	float tx = fx - ix;
	float ty = fy - iy;
	int x0 = std::min(std::max(ix, 0), m_width - 1);
	int x1 = std::min(std::max(ix + 1, 0), m_width - 1);
	int y0 = std::min(std::max(iy, 0), m_height - 1);
	int y1 = std::min(std::max(iy + 1, 0), m_height - 1);
	const byte *p00 = &m_pixels[((size_t)y0 * m_width + x0) * 4];
	const byte *p10 = &m_pixels[((size_t)y0 * m_width + x1) * 4];
	const byte *p01 = &m_pixels[((size_t)y1 * m_width + x0) * 4];
	const byte *p11 = &m_pixels[((size_t)y1 * m_width + x1) * 4];
	//weight colour by alpha (premultiplied filtering) so transparent texels don't bleed their black rgb
	//into the edges of scaled art (the dark rims on hills and clouds)
	float w00 = (1 - tx) * (1 - ty) * p00[3], w10 = tx * (1 - ty) * p10[3];
	float w01 = (1 - tx) * ty * p01[3], w11 = tx * ty * p11[3];
	float alpha = w00 + w10 + w01 + w11;
	out[3] = alpha;
	if (alpha <= 0)
	{
		out[0] = out[1] = out[2] = 0;
		return;
	}
	for (int c = 0; c < 3; c++)
	{
		out[c] = (p00[c] * w00 + p10[c] * w10 + p01[c] * w01 + p11[c] * w11) / alpha;
	}
}

void SoftSurface::SampleNearest(float u, float v, float out[4]) const
{
	int x = std::min(std::max((int)std::floor(u), 0), m_width - 1);
	int y = std::min(std::max((int)std::floor(v), 0), m_height - 1);
	const byte *p = &m_pixels[((size_t)y * m_width + x) * 4];
	for (int c = 0; c < 4; c++)
	{
		out[c] = p[c];
	}
}

//samples like GL_LINEAR (alpha weighted), colour blends like glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA),
//alpha composites "over" so near-invisible texels never punch holes in the alpha (the final image is opaque anyway)
void SoftSurface::BlitEx(const SoftSurface &src, const rtRectf &srcRect, const rtRectf &dst, glColorBytes tint, bool bFlipX, bool bNearest)
{
	if (!src.IsActive() || dst.GetWidth() <= 0 || dst.GetHeight() <= 0) return;
	int x0 = std::max(0, (int)std::ceil(dst.left - 0.5f));
	int y0 = std::max(0, (int)std::ceil(dst.top - 0.5f));
	int x1 = std::min(m_width, (int)std::ceil(dst.right - 0.5f));
	int y1 = std::min(m_height, (int)std::ceil(dst.bottom - 0.5f));
	float scaleU = srcRect.GetWidth() / dst.GetWidth();
	float scaleV = srcRect.GetHeight() / dst.GetHeight();
	float sample[4];
	for (int y = y0; y < y1; y++)
	{
		float v = srcRect.top + ((y + 0.5f) - dst.top) * scaleV;
		for (int x = x0; x < x1; x++)
		{
			float offset = ((x + 0.5f) - dst.left) * scaleU;
			float u = bFlipX ? srcRect.right - offset : srcRect.left + offset;
			if (bNearest) src.SampleNearest(u, v, sample);
			else src.SampleLinear(u, v, sample);
			if (sample[3] <= 0) continue;
			sample[0] = sample[0] * tint.r / 255.0f;
			sample[1] = sample[1] * tint.g / 255.0f;
			sample[2] = sample[2] * tint.b / 255.0f;
			sample[3] = sample[3] * tint.a / 255.0f;
			BlendPixel(x, y, sample);
		}
	}
}

void SoftSurface::BlitScaled(const SoftSurface &src, float x, float y, float scale, glColorBytes tint)
{
	rtRectf srcRect(0, 0, (float)src.GetWidth(), (float)src.GetHeight());
	rtRectf dst(x, y, x + src.GetWidth() * scale, y + src.GetHeight() * scale);
	BlitEx(src, srcRect, dst, tint);
}

void SoftSurface::BlitRotated(const SoftSurface &src, const rtRectf &srcRect, float left, float top, float pivotX, float pivotY, float angleDeg, glColorBytes tint)
{
	if (!src.IsActive()) return;
	float radians = angleDeg * 3.14159265f / 180.0f;
	float c = cosf(radians), s = sinf(radians);
	float w = srcRect.GetWidth(), h = srcRect.GetHeight();
	float cornersX[4] = { left, left + w, left, left + w };
	float cornersY[4] = { top, top, top + h, top + h };
	float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
	for (int i = 0; i < 4; i++)
	{
		float dx = cornersX[i] - pivotX, dy = cornersY[i] - pivotY;
		float rx = pivotX + dx * c - dy * s, ry = pivotY + dx * s + dy * c;
		minX = std::min(minX, rx); maxX = std::max(maxX, rx);
		minY = std::min(minY, ry); maxY = std::max(maxY, ry);
	}
	int x0 = std::max(0, (int)floorf(minX)), y0 = std::max(0, (int)floorf(minY));
	int x1 = std::min(m_width, (int)ceilf(maxX) + 1), y1 = std::min(m_height, (int)ceilf(maxY) + 1);
	float sample[4];
	for (int y = y0; y < y1; y++)
	{
		for (int x = x0; x < x1; x++)
		{
			float dx = x + 0.5f - pivotX, dy = y + 0.5f - pivotY;
			float lx = pivotX + dx * c + dy * s - left;
			float ly = pivotY - dx * s + dy * c - top;
			if (lx < 0 || ly < 0 || lx >= w || ly >= h) continue;
			src.SampleNearest(srcRect.left + lx, srcRect.top + ly, sample);
			if (sample[3] <= 0) continue;
			sample[0] = sample[0] * tint.r / 255.0f;
			sample[1] = sample[1] * tint.g / 255.0f;
			sample[2] = sample[2] * tint.b / 255.0f;
			sample[3] = sample[3] * tint.a / 255.0f;
			BlendPixel(x, y, sample);
		}
	}
}
