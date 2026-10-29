#include "WorldScene.h"

class SceneReader
{
public:

	SceneReader(const vector<byte> &data) : m_data(data), m_pos(0), m_bOverflow(false) {}

	template <class T> T Get()
	{
		T v{};
		if (m_pos + sizeof(T) > m_data.size())
		{
			m_bOverflow = true;
			return v;
		}
		memcpy(&v, &m_data[m_pos], sizeof(T));
		m_pos += sizeof(T);
		return v;
	}

	string GetString()
	{
		uint16 len = Get<uint16>();
		if (m_pos + len > m_data.size())
		{
			m_bOverflow = true;
			return "";
		}
		string s((const char *)&m_data[m_pos], len);
		m_pos += len;
		return s;
	}

	bool IsBad() const { return m_bOverflow; }

private:

	const vector<byte> &m_data;
	size_t m_pos;
	bool m_bOverflow;
};

//RTWR v2, written by .tools/scene-export.  Format is in 14-handoff
bool WorldScene::Load(const string &fileName)
{
	vector<byte> data;
	if (!LoadFileIntoMemory(fileName, data) || data.size() < 8 || memcmp(data.data(), "RTWR", 4) != 0)
	{
		LogError("%s isn't a world scene", fileName.c_str());
		return false;
	}
	SceneReader r(data);
	r.Get<uint32>();
	uint32 version = r.Get<uint32>();
	if (version < 1 || version > 3)
	{
		LogError("Unknown scene version in %s", fileName.c_str());
		return false;
	}
	m_name = r.GetString();
	m_width = r.Get<int32>();
	m_height = r.Get<int32>();
	m_baseWeather = r.Get<uint16>();
	m_currentWeather = r.Get<uint16>();

	uint32 itemCount = r.Get<uint32>();
	for (uint32 i = 0; i < itemCount && !r.IsBad(); i++)
	{
		SceneItem item;
		item.id = r.Get<uint16>();
		item.texture = r.GetString();
		item.textureX = r.Get<byte>();
		item.textureY = r.Get<byte>();
		item.spreadType = r.Get<byte>();
		item.seedBase = r.Get<byte>();
		item.seedOverlay = r.Get<byte>();
		item.treeBase = r.Get<byte>();
		item.treeLeaves = r.Get<byte>();
		item.seedColor = r.Get<int32>();
		item.seedOverlayColor = r.Get<int32>();
		item.growTime = r.Get<int32>();
		if (version >= 2) item.clothingType = r.Get<byte>();
		m_items[item.id] = item;
	}

	if (m_width <= 0 || m_height <= 0 || m_width > 1000 || m_height > 1000)
	{
		LogError("Silly world size %dx%d", m_width, m_height);
		return false;
	}
	m_tiles.resize((size_t)m_width * m_height);
	for (SceneTile &tile : m_tiles)
	{
		tile.fg = r.Get<uint16>();
		tile.bg = r.Get<uint16>();
		tile.flags = r.Get<uint16>();
		tile.treeAge = r.Get<uint32>();
		tile.fruitCount = r.Get<byte>();
		if (version >= 3) tile.frame = r.Get<byte>();
	}

	uint32 objectCount = r.Get<uint32>();
	for (uint32 i = 0; i < objectCount && !r.IsBad(); i++)
	{
		SceneObject obj;
		obj.itemID = r.Get<uint16>();
		obj.count = r.Get<byte>();
		obj.x = r.Get<float>();
		obj.y = r.Get<float>();
		m_objects.push_back(obj);
	}

	if (version >= 2)
	{
		m_ownerUserID = r.Get<uint32>();
		uint32 accessCount = r.Get<uint32>();
		for (uint32 i = 0; i < accessCount && !r.IsBad(); i++)
		{
			m_accessUserIDs.push_back(r.Get<uint32>());
		}
		uint32 playerCount = r.Get<uint32>();
		for (uint32 i = 0; i < playerCount && !r.IsBad(); i++)
		{
			ScenePlayer player;
			player.netID = r.Get<int32>();
			player.userID = r.Get<uint32>();
			player.name = r.GetString();
			player.country = r.GetString();
			player.flags = r.Get<uint32>();
			player.x = r.Get<float>();
			player.y = r.Get<float>();
			for (int s = 0; s < CLOTHING_COUNT; s++)
			{
				player.clothing[s] = r.Get<uint16>();
			}
			player.skinColor = r.Get<uint32>();
			m_players.push_back(player);
		}
	}

	if (r.IsBad())
	{
		LogError("%s is truncated", fileName.c_str());
		return false;
	}
	return true;
}

const SceneItem * WorldScene::GetItem(uint16 id) const
{
	auto itor = m_items.find(id);
	return itor == m_items.end() ? NULL : &itor->second;
}

const SceneTile * WorldScene::GetTile(int x, int y) const
{
	if (x < 0 || y < 0 || x >= m_width || y >= m_height) return NULL;
	return &m_tiles[(size_t)y * m_width + x];
}

vector<const ScenePlayer *> WorldScene::GetOwners() const
{
	vector<const ScenePlayer *> owners;
	for (const ScenePlayer &player : m_players)
	{
		bool bOwner = m_ownerUserID != 0 && player.userID == m_ownerUserID;
		bool bAccess = std::find(m_accessUserIDs.begin(), m_accessUserIDs.end(), player.userID) != m_accessUserIDs.end();
		if (bOwner) owners.insert(owners.begin(), &player);
		else if (bAccess) owners.push_back(&player);
	}
	if (owners.empty())
	{
		for (const ScenePlayer &player : m_players)
		{
			if (player.flags & C_PLAYER_FLAG_LOCAL) owners.push_back(&player);
		}
	}
	return owners;
}
