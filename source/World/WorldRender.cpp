#include "WorldRender.h"
#include "Renderer/RTTexLoader.h"
#include "Avatar/AvatarLayers.h"

const int C_TILE_SIZE = 32;
const float C_DEFAULT_WORLD_SCALE = 0.5f;
const float C_SHADOW_OFFSET_X = -4;
const float C_SHADOW_OFFSET_Y = 4;
const byte C_SHADOW_ALPHA = 128; //grid search on the our world rports 128 beats 112/120/143, offset (-4,4) stays
const byte C_LIQUID_ALPHA = 145;
const byte C_LOGO_ALPHA = 229;
const float C_TREE_SIZE = 38;
const float C_TREE_OFFSET_X = 1;
const float C_FRUIT_SIZE = 10;
const float C_BOTTOM_BAR_HEIGHT = 100;
const byte C_BOTTOM_BAR_ALPHA = 179;
const uint16 C_TILE_FLAG_FLIPPED = 0x0020;
const uint16 C_TILE_FLAG_WATER = 0x0400;
const uint16 C_TILE_FLAG_FIRE = 0x1000;
const uint16 C_TILE_FLAG_RED = 0x2000;
const uint16 C_TILE_FLAG_GREEN = 0x4000;
const uint16 C_TILE_FLAG_BLUE = 0x8000;
const uint16 C_ITEM_GEMS = 112;
const uint16 g_lockItems[] = { 202, 204, 206, 242, 1796, 4428, 7188 };
const int g_smartEdgeLUT[256] =
{
	12, 0, 11, 0, 0, 0, 0, 0, 30, 0, 44, 8, 0, 0, 0, 0, 29, 0, 43, 0, 0, 0, 7, 0, 28, 0, 42, 41, 0, 0, 40, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	10, 0, 9, 0, 0, 0, 0, 0, 46, 0, 36, 35, 0, 0, 0, 0, 45, 0, 33, 0, 0, 0, 32, 0, 39, 0, 27, 23, 0, 0, 24, 18,
	0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 34, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38, 0, 25, 20, 0, 0, 21, 16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 31, 0, 0, 0, 3, 0, 37, 0, 26, 22, 0, 0, 19, 15,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 17, 14, 0, 0, 13, 0
};
const int g_smartOuterLUT[16] = { 12, 11, 15, 8, 14, 7, 13, 2, 10, 9, 6, 4, 5, 3, 1, 0 };

static bool IsLock(uint16 id)
{
	for (uint16 lock : g_lockItems)
	{
		if (lock == id) return true;
	}
	return false;
}

static glColorBytes ColorFromItemInt(int32 c)
{
	uint32 v = (uint32)c;
	return glColorBytes((byte)(v >> 8), (byte)(v >> 16), (byte)(v >> 24), 255);
}

static bool SameAs(const WorldScene &world, int x, int y, uint16 id, bool bBackground, bool bEdgeCounts)
{
	if (x < 0 || x >= world.m_width) return bEdgeCounts;
	const SceneTile *pTile = world.GetTile(x, y);
	if (!pTile) return false;
	return (bBackground ? pTile->bg : pTile->fg) == id;
}

//int GetSpreadOffsetOld(const WorldScene &world, int x, int y, byte spreadType, int &cellX, int &cellY); hmm
int GetSpreadOffset(const WorldScene &world, int x, int y, byte spreadType, bool bBackground, int &cellX, int &cellY)
{
	const SceneTile *pTile = world.GetTile(x, y);
	uint16 id = bBackground ? pTile->bg : pTile->fg;
	cellX = 0;
	cellY = 0;

	switch (spreadType)
	{
	case 2:
	{
		const int dx[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
		const int dy[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
		int bits = 0;
		for (int i = 0; i < 8; i++)
		{
			if (SameAs(world, x + dx[i], y + dy[i], id, bBackground, true)) bits |= 1 << i;
		}
		if (!(bits & 8) || !(bits & 2)) bits &= ~1;
		if (!(bits & 16) || !(bits & 2)) bits &= ~4;
		if (!(bits & 8) || !(bits & 64)) bits &= ~32;
		if (!(bits & 16) || !(bits & 64)) bits &= ~128;
		int cell = g_smartEdgeLUT[bits];
		cellX = cell % 8;
		cellY = cell / 8;
		break;
	}
	case 3:
	{
		bool bLeft = SameAs(world, x - 1, y, id, bBackground, true);
		bool bRight = SameAs(world, x + 1, y, id, bBackground, true);
		cellX = bLeft && bRight ? 1 : bLeft ? 2 : bRight ? 0 : 3;
		break;
	}
	case 4:
	{
		const int dx[4] = { -1, 0, 1, 0 };
		const int dy[4] = { 0, -1, 0, 1 };
		cellX = 4;
		for (int i = 0; i < 4; i++)
		{
			if (SameAs(world, x + dx[i], y + dy[i], id, bBackground, true)) cellX = i;
		}
		break;
	}
	case 5:
	{
		int bits = 0;
		if (SameAs(world, x, y - 1, id, bBackground, false)) bits |= 1;
		if (SameAs(world, x - 1, y, id, bBackground, true)) bits |= 2;
		if (SameAs(world, x + 1, y, id, bBackground, true)) bits |= 4;
		if (SameAs(world, x, y + 1, id, bBackground, false)) bits |= 8;
		int cell = g_smartOuterLUT[bits];
		cellX = cell % 8;
		cellY = cell / 8;
		break;
	}
	case 7:
	{
		bool bAbove = SameAs(world, x, y - 1, id, bBackground, false);
		bool bBelow = SameAs(world, x, y + 1, id, bBackground, false);
		cellX = bAbove && bBelow ? 1 : bAbove ? 2 : bBelow ? 0 : 3;
		break;
	}
	case 9:
	{
		const int dx[4] = { 1, -1, 0, 0 };
		const int dy[4] = { 0, 0, 1, -1 };
		cellX = 3;
		for (int i = 0; i < 4; i++)
		{
			if (SameAs(world, x + dx[i], y + dy[i], id, bBackground, true)) cellX = i;
		}
		break;
	}
	default:
		break;
	}

	if (!bBackground && IsLock(id)) cellX = 2;
	return cellX + cellY * 8;
}

float WorldRender::TileDrawSize() const
{
	return C_TILE_SIZE * m_options.scale;
}

bool WorldRender::CutScene(SoftSurface &out)
{
	//a scene is just a tile rect of the finished world, no bottom bar
	float tile = TileDrawSize();
	int left = (int)(m_options.sceneX * tile);
	int top = (int)(m_options.sceneY * tile);
	int width = (int)(m_options.sceneW * tile);
	int height = (int)(m_options.sceneH * tile);
	if (left < 0 || top < 0 || width <= 0 || height <= 0 || left + width > out.GetWidth() || top + height > out.GetHeight())
	{
		LogError("Scene %d,%d %dx%d is outside the world", m_options.sceneX, m_options.sceneY, m_options.sceneW, m_options.sceneH);
		return false;
	}
	vector<byte> pixels((size_t)width * height * 4);
	const byte *pSrc = out.GetPixelData();
	for (int y = 0; y < height; y++)
	{
		memcpy(&pixels[(size_t)y * width * 4], pSrc + ((size_t)(top + y) * out.GetWidth() + left) * 4, (size_t)width * 4);
	}
	out.SetPixelData(width, height, pixels.data());
	return true;
}

bool WorldRender::Init(const WorldRenderOptions &options)
{
	m_options = options;
	if (m_options.scale <= 0) m_options.scale = C_DEFAULT_WORLD_SCALE;
	return m_font.Load(m_options.resourcePath + "/interface/font_century_gothic_bigx2.rtfont")
		&& m_smallFont.Load(m_options.resourcePath + "/interface/font_century_gothic.rtfont");
}

const SoftSurface * WorldRender::GetTexture(const string &fileName)
{
	auto itor = m_textures.find(fileName);
	if (itor != m_textures.end()) return itor->second.IsActive() ? &itor->second : NULL;
	SoftSurface &surf = m_textures[fileName];
	LoadRTTEX(m_options.resourcePath + "/" + fileName, surf);
	return surf.IsActive() ? &surf : NULL;
}

bool WorldRender::DrawItemCell(SoftSurface &out, const WorldScene &world, const SceneItem &item, int tileX, int tileY, int cellX, int cellY, glColorBytes tint, bool bFlip, float offsetX, float offsetY)
{
	const SoftSurface *pSheet = GetTexture("game/" + item.texture);
	if (!pSheet) return false;
	float srcX = (float)(item.textureX + cellX) * C_TILE_SIZE;
	float srcY = (float)(item.textureY + cellY) * C_TILE_SIZE;
	float left = tileX * TileDrawSize() + offsetX * m_options.scale;
	float top = tileY * TileDrawSize() + offsetY * m_options.scale;
	out.BlitEx(*pSheet, rtRectf(srcX, srcY, srcX + C_TILE_SIZE, srcY + C_TILE_SIZE), rtRectf(left, top, left + TileDrawSize(), top + TileDrawSize()), tint, bFlip);
	return true;
}

void WorldRender::DrawSeed(SoftSurface &out, const SceneItem &item, float x, float y, float size)
{
	const SoftSurface *pSeed = GetTexture("game/seed.rttex");
	if (!pSeed) return;
	float baseX = item.seedBase * 16.0f;
	float overlayX = item.seedOverlay * 16.0f;
	out.BlitEx(*pSeed, rtRectf(baseX, 0, baseX + 16, 16), rtRectf(x, y, x + size, y + size), ColorFromItemInt(item.seedColor));
	out.BlitEx(*pSeed, rtRectf(overlayX, 16, overlayX + 16, 32), rtRectf(x, y, x + size, y + size), ColorFromItemInt(item.seedOverlayColor));
}

void WorldRender::DrawHill(SoftSurface &out, int hillIndex)
{
	const SoftSurface *pHill = GetTexture("game/hills" + std::to_string(3 - hillIndex) + ".rttex");
	if (!pHill) return;
	float width = (float)out.GetWidth();
	float scale = width / (pHill->GetWidth() - 1.0f);
	float bottom = out.GetHeight() + 2.0f;
	if (hillIndex == 0) bottom -= scale * 46.0f;
	else if (hillIndex == 1) bottom += scale * 60.0f;
	out.BlitScaled(*pHill, 0, bottom - pHill->GetHeight() * scale, scale);
}

void WorldRender::DrawWeather(const WorldScene &world, SoftSurface &out)
{
	out.FillColor(glColorBytes(96, 215, 242, 255));
	const SoftSurface *pCloud = GetTexture("game/cloud.rttex");
	const SoftSurface *pSun = GetTexture("game/sun.rttex");
	float sky = m_options.scale / C_DEFAULT_WORLD_SCALE; //cloud and sun spots were measured on a half size render
	if (pCloud) out.BlitScaled(*pCloud, 1240 * sky, 215 * sky, 0.72f * sky, glColorBytes(255, 255, 255, 180));
	if (pSun) out.BlitScaled(*pSun, 1121 * sky, 95 * sky, 1.62f * sky);
	DrawHill(out, 0);
	DrawHill(out, 1);
	DrawHill(out, 2);
	if (!pCloud) return;
	struct CloudSpot { float x, y, scale; byte alpha; bool bFlip; };
	const CloudSpot clouds[] =
	{
		{ 567, 94, 0.90f, 230, true }, { 340, 161, 0.69f, 180, false }, { 852, 164, 0.76f, 205, false },
		{ 1390, 88, 0.72f, 180, false }, { 1443, 205, 0.73f, 180, false }
	};
	for (const CloudSpot &c : clouds)
	{
		rtRectf src(0, 0, (float)pCloud->GetWidth(), (float)pCloud->GetHeight());
		rtRectf dst(c.x * sky, c.y * sky, (c.x + pCloud->GetWidth() * c.scale) * sky, (c.y + pCloud->GetHeight() * c.scale) * sky);
		out.BlitEx(*pCloud, src, dst, glColorBytes(255, 255, 255, c.alpha), c.bFlip);
	}
}

void WorldRender::DrawTree(SoftSurface &out, const WorldScene &world, const SceneTile &tile, const SceneItem &seed, int tileX, int tileY, bool bShadow)
{
	if (getenv("RTWR_NO_TREES")) return;
	const SoftSurface *pSheet = GetTexture("game/tiles_page1.rttex");
	if (!pSheet) return;
	float bloom = 1.0f;
	if (seed.growTime > 0 && tile.treeAge != C_NO_TREE_AGE) bloom = std::min(1.0f, (float)tile.treeAge / seed.growTime);
	//fitted on the 1ETUU ground truth pair: a ripe tree is 38 px (at 1x) and sits 1 px right of the tile centre
	float treeScale = bloom >= 1.0f ? 1.0f : 0.25f + 0.75f * bloom;
	float size = C_TREE_SIZE * m_options.scale * treeScale;
	float centerX = tileX * TileDrawSize() + TileDrawSize() * 0.5f + C_TREE_OFFSET_X * m_options.scale;
	float bottom = (tileY + 1) * TileDrawSize();
	if (bShadow)
	{
		centerX += C_SHADOW_OFFSET_X * m_options.scale;
		bottom += C_SHADOW_OFFSET_Y * m_options.scale;
	}
	rtRectf dst(centerX - size * 0.5f, bottom - size, centerX + size * 0.5f, bottom);
	glColorBytes baseColor = bShadow ? glColorBytes(0, 0, 0, C_SHADOW_ALPHA) : ColorFromItemInt(seed.seedColor);
	glColorBytes overlayColor = bShadow ? glColorBytes(0, 0, 0, C_SHADOW_ALPHA) : ColorFromItemInt(seed.seedOverlayColor);
	out.BlitEx(*pSheet, rtRectf(seed.treeBase * 32.0f, 19 * 32.0f, seed.treeBase * 32.0f + 32, 20 * 32.0f), dst, baseColor);
	out.BlitEx(*pSheet, rtRectf(seed.treeLeaves * 32.0f, 18 * 32.0f, seed.treeLeaves * 32.0f + 32, 19 * 32.0f), dst, overlayColor);
	if (bShadow || bloom < 1.0f || tile.fruitCount == 0) return;

	const SceneItem *pFruit = world.GetItem(seed.id - 1);
	if (!pFruit) return;
	const SoftSurface *pFruitSheet = GetTexture("game/" + pFruit->texture);
	if (!pFruitSheet) return;
	//fitted on 1ETUU (trees with 1-4 fruit), the fifth spot continues the grid and is a guess
	const float fruitOffsets[5][2] = { { 6, 0 }, { 18, 0 }, { 0, 10 }, { 11, 10 }, { 22, 10 } };
	int cellX = pFruit->spreadType == 2 ? 4 : 0;
	int cellY = pFruit->spreadType == 2 ? 1 : 0;
	float srcX = (pFruit->textureX + cellX) * 32.0f;
	float srcY = (pFruit->textureY + cellY) * 32.0f;
	float fruitSize = C_FRUIT_SIZE * m_options.scale;
	for (int i = 0; i < std::min<int>(tile.fruitCount, 5); i++)
	{
		float fx = tileX * TileDrawSize() + fruitOffsets[i][0] * m_options.scale;
		float fy = tileY * TileDrawSize() + fruitOffsets[i][1] * m_options.scale;
		out.BlitEx(*pFruitSheet, rtRectf(srcX, srcY, srcX + 32, srcY + 32), rtRectf(fx, fy, fx + fruitSize, fy + fruitSize), glColorBytes());
	}
}

void WorldRender::DrawTileLayer(const WorldScene &world, SoftSurface &out, bool bBackground)
{
	for (int y = 0; y < world.m_height; y++)
	{
		for (int x = 0; x < world.m_width; x++)
		{
			const SceneTile *pTile = world.GetTile(x, y);
			uint16 id = bBackground ? pTile->bg : pTile->fg;
			if (id == 0) continue;
			const SceneItem *pItem = world.GetItem(id);
			if (!pItem) continue;
			if (!bBackground && (id % 2) == 1)
			{
				DrawTree(out, world, *pTile, *pItem, x, y, false);
				continue;
			}
			glColorBytes tint;
			if (!bBackground && (pTile->flags & (C_TILE_FLAG_RED | C_TILE_FLAG_GREEN | C_TILE_FLAG_BLUE)))
			{
				tint = glColorBytes(pTile->flags & C_TILE_FLAG_RED ? 255 : 0, pTile->flags & C_TILE_FLAG_GREEN ? 255 : 0, pTile->flags & C_TILE_FLAG_BLUE ? 255 : 0, 255);
			}
			int cellX, cellY;
			GetSpreadOffset(world, x, y, pItem->spreadType, bBackground, cellX, cellY);
			if (!bBackground) cellX += pTile->frame;
			DrawItemCell(out, world, *pItem, x, y, cellX, cellY, tint, !bBackground && (pTile->flags & C_TILE_FLAG_FLIPPED));
		}
	}
}

//the official render probably draws shadows into their own layer and composites once,
//we blend each one straight into the target.  Overlaps stack here, check with a ground truth pair
static float CalibrationValue(const char *pName, float fallback)
{
	const char *pValue = getenv(pName);
	return pValue ? (float)atof(pValue) : fallback;
}

void WorldRender::DrawTileShadows(const WorldScene &world, SoftSurface &out)
{
	const float shadowX = CalibrationValue("RTWR_SHADOW_DX", C_SHADOW_OFFSET_X);
	const float shadowY = CalibrationValue("RTWR_SHADOW_DY", C_SHADOW_OFFSET_Y);
	const byte shadowAlpha = (byte)CalibrationValue("RTWR_SHADOW_ALPHA", C_SHADOW_ALPHA);
	for (int y = 0; y < world.m_height; y++)
	{
		for (int x = 0; x < world.m_width; x++)
		{
			const SceneTile *pTile = world.GetTile(x, y);
			if (pTile->fg == 0) continue;
			const SceneItem *pItem = world.GetItem(pTile->fg);
			if (!pItem) continue;
			if ((pTile->fg % 2) == 1)
			{
				DrawTree(out, world, *pTile, *pItem, x, y, true);
				continue;
			}
			int cellX, cellY;
			GetSpreadOffset(world, x, y, pItem->spreadType, false, cellX, cellY);
			DrawItemCell(out, world, *pItem, x, y, cellX, cellY, glColorBytes(0, 0, 0, shadowAlpha), pTile->flags & C_TILE_FLAG_FLIPPED, shadowX, shadowY);
		}
	}
}

void WorldRender::DrawLiquids(const WorldScene &world, SoftSurface &out)
{
	const SoftSurface *pWater = GetTexture("game/water.rttex");
	const SoftSurface *pFire = GetTexture("game/fire.rttex");
	for (int y = 0; y < world.m_height; y++)
	{
		for (int x = 0; x < world.m_width; x++)
		{
			const SceneTile *pTile = world.GetTile(x, y);
			float left = x * TileDrawSize();
			float top = y * TileDrawSize();
			rtRectf dst(left, top, left + TileDrawSize(), top + TileDrawSize());
			if ((pTile->flags & C_TILE_FLAG_WATER) && pWater) out.BlitEx(*pWater, rtRectf(0, 0, 32, 32), dst, glColorBytes(255, 255, 255, C_LIQUID_ALPHA));
			if ((pTile->flags & C_TILE_FLAG_FIRE) && pFire) out.BlitEx(*pFire, rtRectf(0, 0, 32, 32), dst, glColorBytes(255, 255, 255, C_LIQUID_ALPHA));
		}
	}
}

void WorldRender::DrawDroppedStuff(const WorldScene &world, SoftSurface &out)
{
	const float iconSize = 19 * m_options.scale;
	for (const SceneObject &obj : world.m_objects)
	{
		const SceneItem *pItem = world.GetItem(obj.itemID);
		if (!pItem) continue;
		float x = obj.x * m_options.scale;
		float y = obj.y * m_options.scale;
		if (obj.itemID % 2 == 1)
		{
			DrawSeed(out, *pItem, x, y, 16 * m_options.scale);
			continue;
		}
		const SoftSurface *pSheet = GetTexture("game/" + pItem->texture);
		if (!pSheet) continue;
		int cellX = pItem->spreadType == 2 ? 4 : 0;
		int cellY = pItem->spreadType == 2 ? 1 : 0;
		if (obj.itemID == C_ITEM_GEMS)
		{
			cellX = obj.count >= 100 ? 4 : obj.count >= 50 ? 3 : obj.count >= 10 ? 2 : obj.count >= 5 ? 1 : 0;
		}
		float srcX = (float)(pItem->textureX + cellX) * C_TILE_SIZE;
		float srcY = (float)(pItem->textureY + cellY) * C_TILE_SIZE;
		out.BlitEx(*pSheet, rtRectf(srcX, srcY, srcX + C_TILE_SIZE, srcY + C_TILE_SIZE), rtRectf(x, y, x + iconSize, y + iconSize), glColorBytes());
		if (obj.count > 1 && obj.itemID != C_ITEM_GEMS)
		{
			const SoftSurface *pBox = GetTexture("game/pickup_box.rttex");
			if (pBox) out.BlitEx(*pBox, rtRectf(0, 0, 20, 20), rtRectf(x, y, x + 20 * m_options.scale, y + 20 * m_options.scale), glColorBytes());
		}
	}
}

void WorldRender::DrawBottomBar(const WorldScene &world, SoftSurface &out)
{
	float top = out.GetHeight() - C_BOTTOM_BAR_HEIGHT;
	out.FillRect(rtRectf(0, top, (float)out.GetWidth(), (float)out.GetHeight()), glColorBytes(0, 0, 0, C_BOTTOM_BAR_ALPHA));

	const float textScale = 0.4f;
	const glColorBytes pink(255, 144, 243, 255);
	const glColorBytes white(255, 255, 255, 255);
	string first = "Visit ";
	string name = "\"" + world.m_name + "\" ";
	string last = "in";
	float width = m_font.MeasureText(first + name + last, textScale);
	float x = out.GetWidth() - 15 - width;
	float y = top + 7;
	m_font.DrawScaled(out, x, y, first, textScale, pink);
	x += m_font.MeasureText(first, textScale);
	m_font.DrawScaled(out, x, y, name, textScale, white);
	x += m_font.MeasureText(name, textScale);
	m_font.DrawScaled(out, x, y, last, textScale, pink);

	const SoftSurface *pLogo = GetTexture("interface/large/game_title.rttex");
	if (pLogo)
	{
		float logoScale = 204.0f / pLogo->GetWidth();
		out.BlitScaled(*pLogo, out.GetWidth() - pLogo->GetWidth() * logoScale, top + 20, logoScale, glColorBytes(255, 255, 255, C_LOGO_ALPHA));
	}

	vector<const ScenePlayer *> owners = world.GetOwners();
	float ownerX = 0;
	for (const ScenePlayer *pOwner : owners)
	{
		DrawOwnerBlock(out, world, *pOwner, ownerX, top);
		ownerX += 110;
	}
	if (owners.empty() && !m_options.ownerName.empty())
	{
		ScenePlayer named;
		named.name = "`2" + m_options.ownerName;
		named.country = m_options.ownerCountry;
		DrawOwnerBlock(out, world, named, 0, top);
	}
}

bool WorldRender::BuildAvatar(const WorldScene &world, const ScenePlayer &player, SoftSurface &canvas)
{
	canvas.Init(C_AVATAR_CANVAS_SIZE, C_AVATAR_CANVAS_SIZE);
	uint32 v = player.skinColor;
	glColorBytes skin((byte)(v >> 8), (byte)(v >> 16), (byte)(v >> 24), 255);
	if (v == 0) skin = glColorBytes(255, 202, 160, 255);
	for (const AvatarLayer &layer : g_avatarLayers)
	{
		const SoftSurface *pSheet = NULL;
		rtRectf src;
		/** avatar source credit: setplannergt.com*/
		if (layer.source == AVATAR_SOURCE_SHEET)
		{
			pSheet = GetTexture(string("game/") + layer.pSheet);
			if (!pSheet) continue;
			if (layer.cellX < 0) src = rtRectf(0, 0, (float)pSheet->GetWidth(), (float)pSheet->GetHeight());
			else src = rtRectf(layer.cellX * 32.0f, layer.cellY * 32.0f, layer.cellX * 32.0f + 32, layer.cellY * 32.0f + 32);
		}
		else
		{
			uint16 id = player.clothing[layer.clothingSlot];
			const SceneItem *pItem = id ? world.GetItem(id) : NULL;
			if (!pItem) continue;
			pSheet = GetTexture("game/" + pItem->texture);
			if (!pSheet) continue;
			src = rtRectf(pItem->textureX * 32.0f, pItem->textureY * 32.0f, pItem->textureX * 32.0f + 32, pItem->textureY * 32.0f + 32);
		}
		float left = (float)(C_AVATAR_ORIGIN_X + layer.offsetX);
		float top = (float)(C_AVATAR_ORIGIN_Y + layer.offsetY);
		canvas.BlitEx(*pSheet, src, rtRectf(left, top, left + src.GetWidth(), top + src.GetHeight()), layer.bSkinTint ? skin : glColorBytes(), false, true);
	}
	return true;
}

void WorldRender::DrawAvatar(SoftSurface &out, const WorldScene &world, const ScenePlayer &player, float x, float y, float scale, bool bFacingLeft)
{
	SoftSurface canvas;
	if (!BuildAvatar(world, player, canvas)) return;
	rtRectf src(0, 0, (float)canvas.GetWidth(), (float)canvas.GetHeight());
	out.BlitEx(canvas, src, rtRectf(x, y, x + canvas.GetWidth() * scale, y + canvas.GetHeight() * scale), glColorBytes(), bFacingLeft, true);
}

void WorldRender::DrawOwnerBlock(SoftSurface &out, const WorldScene &world, const ScenePlayer &player, float x, float top)
{
	string country = player.country.substr(0, player.country.find('|'));
	const SoftSurface *pFlag = country.empty() ? NULL : GetTexture("interface/flags/" + country + ".rttex");
	if (pFlag) out.BlitEx(*pFlag, rtRectf(0, 0, (float)pFlag->GetWidth(), (float)pFlag->GetHeight()), rtRectf(x + 33, top + 14, x + 52, top + 27), glColorBytes());
	m_smallFont.DrawColorCoded(out, x + 54.5f, top + 13.5f, player.name, 0.61f, glColorBytes(73, 252, 0, 255));
	DrawAvatar(out, world, player, x - 7, top - 15, 2.0f, false);
}

//#define DUMP_LAYERS //write each pass to its own png BUT WHY WHY WHY WHY

bool WorldRender::Render(const WorldScene &world, SoftSurface &out)
{
	out.Init((int)(world.m_width * TileDrawSize()), (int)(world.m_height * TileDrawSize()));
	DrawWeather(world, out);
	//DrawTileShadows(world, out); //note tomorrow: check if it cast shadows in the official renders?
	DrawTileLayer(world, out, true);
	DrawTileShadows(world, out);
	DrawTileLayer(world, out, false);
	DrawLiquids(world, out);
	MakeOpaque(out); //the official world is alpha 255 everywhere, only the bar lowers it (to 202)
	if (m_options.sceneW > 0) return CutScene(out);
	DrawBottomBar(world, out);

#ifdef _DEBUG
	//LogMsg("Render done, toal %d items in the scene", (int)world.m_items.size());
#endif
	return true;
}
