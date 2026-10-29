#include "RTFont.h"
#include "RTTexLoader.h"

const size_t C_RTFONT_HEADER_SIZE = 152;
const size_t C_RTFONT_GLYPH_SIZE = 32;
const size_t C_RTFONT_STATE_SIZE = 8;

static int16_t GetShort(const byte *p)
{
	int16_t v;
	memcpy(&v, p, 2);
	return v;
}

bool RTFont::Load(const string &fileName)
{
	vector<byte> raw, data;
	if (!LoadFileIntoMemory(fileName, raw) || !UnpackRTPack(raw, data) || data.size() < C_RTFONT_HEADER_SIZE || memcmp(data.data(), "RTFONT", 6) != 0)
	{
		LogError("Can't load font %s", fileName.c_str());
		return false;
	}
	m_charSpacing = GetShort(&data[8]);
	m_lineHeight = GetShort(&data[10]);
	m_firstChar = GetShort(&data[18]);
	int lastChar = GetShort(&data[20]);
	m_blankWidth = GetShort(&data[22]);
	int stateCount = GetShort(&data[24]);
	int glyphCount = std::max(0, lastChar - m_firstChar);
	size_t statesStart = C_RTFONT_HEADER_SIZE + glyphCount * C_RTFONT_GLYPH_SIZE;
	size_t statesEnd = statesStart + stateCount * C_RTFONT_STATE_SIZE;
	if (data.size() < statesEnd) return false;
	for (int i = 0; i < glyphCount; i++)
	{
		const byte *p = &data[C_RTFONT_HEADER_SIZE + i * C_RTFONT_GLYPH_SIZE];
		RTFontGlyph g = { GetShort(p), GetShort(p + 2), GetShort(p + 4), GetShort(p + 6), GetShort(p + 8), GetShort(p + 10), GetShort(p + 28) };
		m_glyphs.push_back(g);
	}
	for (size_t i = statesEnd; i + 6 <= data.size(); i++)
	{
		if (memcmp(&data[i], "RTTXTR", 6) == 0)
		{
			return LoadRTTEXFromMemory(&data[i], data.size() - i, m_atlas);
		}
	}
	return false;
}

const RTFontGlyph * RTFont::GetGlyph(unsigned char c) const
{
	int index = (int)c - m_firstChar;
	if (index < 0 || index >= (int)m_glyphs.size()) return NULL;
	return &m_glyphs[index];
}

float RTFont::MeasureText(const string &text, float scale) const
{
	float width = 0;
	for (unsigned char c : text)
	{
		const RTFontGlyph *pGlyph = GetGlyph(c);
		width += ((pGlyph ? pGlyph->advance : m_blankWidth) + m_charSpacing) * scale;
	}
	return width;
}

void RTFont::DrawScaled(SoftSurface &target, float x, float y, const string &text, float scale, glColorBytes color) const
{
	float penX = x;
	for (unsigned char c : text)
	{
		const RTFontGlyph *pGlyph = GetGlyph(c);
		if (!pGlyph)
		{
			penX += (m_blankWidth + m_charSpacing) * scale;
			continue;
		}
		if (pGlyph->width > 0 && pGlyph->height > 0)
		{
			rtRectf src(pGlyph->atlasX, pGlyph->atlasY, pGlyph->atlasX + pGlyph->width, pGlyph->atlasY + pGlyph->height);
			float left = penX + pGlyph->offsetX * scale;
			float top = y + pGlyph->offsetY * scale;
			target.BlitEx(m_atlas, src, rtRectf(left, top, left + pGlyph->width * scale, top + pGlyph->height * scale), color);
		}
		penX += (pGlyph->advance + m_charSpacing) * scale;
	}
}

//color code table v4 (Jan 2013 onwards), matches the client today
bool RTFont::GetColorFromCode(char code, glColorBytes &color)
{
	switch (code)
	{
	case '0': color = glColorBytes(255, 255, 255, 255); return true;
	case '1': color = glColorBytes(173, 244, 255, 255); return true;
	case '2': color = glColorBytes(73, 252, 0, 255); return true;
	case '3': color = glColorBytes(191, 218, 255, 255); return true;
	case '4': color = glColorBytes(255, 39, 29, 255); return true;
	case '5': color = glColorBytes(235, 183, 255, 255); return true;
	case '6': color = glColorBytes(255, 202, 111, 255); return true;
	case '7': color = glColorBytes(230, 230, 230, 255); return true;
	case '8': color = glColorBytes(255, 148, 69, 255); return true;
	case '9': color = glColorBytes(255, 238, 125, 255); return true;
	case '!': color = glColorBytes(209, 255, 249, 255); return true;
	case '@': color = glColorBytes(255, 205, 201, 255); return true;
	case '#': color = glColorBytes(255, 143, 243, 255); return true;
	case '$': color = glColorBytes(255, 252, 197, 255); return true;
	case '^': color = glColorBytes(181, 255, 151, 255); return true;
	case '&': color = glColorBytes(254, 235, 255, 255); return true;
	case 'w': color = glColorBytes(255, 255, 255, 255); return true;
	case 'o': color = glColorBytes(252, 230, 186, 255); return true;
	case 'p': color = glColorBytes(255, 223, 241, 255); return true;
	case 'b': color = glColorBytes(0, 0, 0, 255); return true;
	case 'q': color = glColorBytes(12, 96, 164, 255); return true;
	case 'e': color = glColorBytes(25, 185, 255, 255); return true;
	case 'r': color = glColorBytes(111, 211, 87, 255); return true;
	case 't': color = glColorBytes(47, 131, 13, 255); return true;
	case 'a': color = glColorBytes(81, 81, 81, 255); return true;
	case 's': color = glColorBytes(158, 158, 158, 255); return true;
	case 'c': color = glColorBytes(80, 255, 255, 255); return true;
	default: return false;
	}
}

string RTFont::StripColorCodes(const string &text)
{
	string out;
	for (size_t i = 0; i < text.size(); i++)
	{
		if (text[i] == '`' && i + 1 < text.size())
		{
			i++;
			continue;
		}
		out += text[i];
	}
	return out;
}

void RTFont::DrawColorCoded(SoftSurface &target, float x, float y, const string &text, float scale, glColorBytes defaultColor) const
{
	glColorBytes color = defaultColor;
	string run;
	float penX = x;
	auto flush = [&]()
	{
		if (run.empty()) return;
		DrawScaled(target, penX, y, run, scale, color);
		penX += MeasureText(run, scale);
		run.clear();
	};
	for (size_t i = 0; i < text.size(); i++)
	{
		if (text[i] == '`' && i + 1 < text.size())
		{
			flush();
			char code = text[++i];
			if (code == '`' || !GetColorFromCode(code, color)) color = defaultColor;
			continue;
		}
		run += text[i];
	}
	flush();
}
