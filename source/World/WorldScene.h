#pragma once

#include "PlatformPrecomp.h"

const uint32 C_NO_TREE_AGE = 0xFFFFFFFF;

struct SceneItem
{
	uint16 id = 0;
	string texture;
	byte textureX = 0;
	byte textureY = 0;
	byte spreadType = 0;
	byte seedBase = 0;
	byte seedOverlay = 0;
	byte treeBase = 0;
	byte treeLeaves = 0;
	int32 seedColor = 0;
	int32 seedOverlayColor = 0;
	int32 growTime = 0;
	byte clothingType = 0;
};

struct SceneTile
{
	uint16 fg = 0;
	uint16 bg = 0;
	uint16 flags = 0;
	uint32 treeAge = C_NO_TREE_AGE;
	byte fruitCount = 0;
	byte frame = 0; //fgcell? open chests yup, dice faces yup, ready providers yup?
};

struct SceneObject
{
	uint16 itemID = 0;
	byte count = 0;
	float x = 0;
	float y = 0;
};

enum eClothingSlot
{
	CLOTHING_HAT,
	CLOTHING_SHIRT,
	CLOTHING_PANTS,
	CLOTHING_FEET,
	CLOTHING_FACE,
	CLOTHING_HAND,
	CLOTHING_BACK,
	CLOTHING_HAIR,
	CLOTHING_NECKLACE,
	CLOTHING_ANCES,
	CLOTHING_COUNT
};

const uint32 C_PLAYER_FLAG_LOCAL = 1;

struct ScenePlayer
{
	int32 netID = -1;
	uint32 userID = 0;
	string name;
	string country;
	uint32 flags = 0;
	float x = 0;
	float y = 0;
	uint16 clothing[CLOTHING_COUNT] = {};
	uint32 skinColor = 0;
};

class WorldScene
{
public:

	bool Load(const string &fileName);
	const SceneItem * GetItem(uint16 id) const;
	const SceneTile * GetTile(int x, int y) const;

	string m_name;
	int m_width = 0;
	int m_height = 0;
	uint16 m_baseWeather = 0;
	uint16 m_currentWeather = 0;
	std::map<uint16, SceneItem> m_items;
	vector<SceneTile> m_tiles;
	vector<SceneObject> m_objects;
	uint32 m_ownerUserID = 0;
	vector<uint32> m_accessUserIDs;
	vector<ScenePlayer> m_players;

	vector<const ScenePlayer *> GetOwners() const;
};
