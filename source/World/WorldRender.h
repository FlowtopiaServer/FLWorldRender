#pragma once

#include "Renderer/SoftSurface.h"
#include "Renderer/RTFont.h"
#include "WorldScene.h"

struct WorldRenderOptions
{
	string resourcePath = "/Applications/Growtopia.app/Contents/Resources";
	string ownerName;
	string ownerCountry;
	float scale = 0.5f; //32px tiles times this
	int sceneX = 0, sceneY = 0, sceneW = 0, sceneH = 0; //tile rect, 0 wide means the whole world
};

class WorldRender
{
public:

	bool Init(const WorldRenderOptions &options);
	bool Render(const WorldScene &world, SoftSurface &out);

private:

	const SoftSurface * GetTexture(const string &fileName);
	float TileDrawSize() const;
	bool CutScene(SoftSurface &out);
	void DrawWeather(const WorldScene &world, SoftSurface &out);
	void DrawHill(SoftSurface &out, int hillIndex);
	void DrawTree(SoftSurface &out, const WorldScene &world, const SceneTile &tile, const SceneItem &seed, int tileX, int tileY, bool bShadow);
	void DrawTileLayer(const WorldScene &world, SoftSurface &out, bool bBackground);
	void DrawTileShadows(const WorldScene &world, SoftSurface &out);
	void DrawLiquids(const WorldScene &world, SoftSurface &out);
	void DrawDroppedStuff(const WorldScene &world, SoftSurface &out);
	void DrawBottomBar(const WorldScene &world, SoftSurface &out);
	bool DrawItemCell(SoftSurface &out, const WorldScene &world, const SceneItem &item, int tileX, int tileY, int cellX, int cellY, glColorBytes tint, bool bFlip, float offsetX = 0, float offsetY = 0);
	void DrawSeed(SoftSurface &out, const SceneItem &item, float x, float y, float size);
	bool BuildAvatar(const WorldScene &world, const ScenePlayer &player, SoftSurface &canvas);
	void DrawAvatar(SoftSurface &out, const WorldScene &world, const ScenePlayer &player, float x, float y, float scale, bool bFacingLeft);
	void DrawOwnerBlock(SoftSurface &out, const WorldScene &world, const ScenePlayer &player, float x, float top);

	WorldRenderOptions m_options;
	std::map<string, SoftSurface> m_textures;
	RTFont m_font;
	RTFont m_smallFont;
};

int GetSpreadOffset(const WorldScene &world, int x, int y, byte spreadType, bool bBackground, int &cellX, int &cellY);
