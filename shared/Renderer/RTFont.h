#pragma once

#include "SoftSurface.h"

struct RTFontGlyph
{
	int16_t atlasX, atlasY, width, height, offsetX, offsetY, advance;
};

class RTFont
{
public:

	bool Load(const string &fileName);
	float MeasureText(const string &text, float scale) const;
	void DrawScaled(SoftSurface &target, float x, float y, const string &text, float scale, glColorBytes color) const;
	void DrawColorCoded(SoftSurface &target, float x, float y, const string &text, float scale, glColorBytes defaultColor) const;
	static bool GetColorFromCode(char code, glColorBytes &color);
	static string StripColorCodes(const string &text);
	float GetLineHeight() const { return m_lineHeight; }

private:

	const RTFontGlyph * GetGlyph(unsigned char c) const;

	SoftSurface m_atlas;
	vector<RTFontGlyph> m_glyphs;
	int m_firstChar = 0;
	float m_charSpacing = 0;
	float m_lineHeight = 0;
	float m_blankWidth = 0;
};
