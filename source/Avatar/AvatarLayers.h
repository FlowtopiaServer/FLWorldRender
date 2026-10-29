#pragma once

#include "World/WorldScene.h"

enum eAvatarLayerSource
{
	AVATAR_SOURCE_SHEET,
	AVATAR_SOURCE_CLOTHING
};

struct AvatarLayer
{
	eAvatarLayerSource source;
	const char *pSheet;
	int cellX, cellY;
	int clothingSlot;
	int offsetX, offsetY;
	bool bSkinTint;
	int srcX = 0, srcY = 0, srcW = 32, srcH = 32;
};

//Credit: setplannergt.com
const int C_AVATAR_CANVAS_SIZE = 128;
const int C_AVATAR_ORIGIN_X = 48;
const int C_AVATAR_ORIGIN_Y = 48;
const int C_LONG_HAND_OFFSET_X = -23;
const int C_LONG_HAND_OFFSET_Y = 10;
const int C_FRONT_ARM_X = 6;
const int C_BACK_ARM_X = 21;
const float C_FRONT_ARM_PIVOT_X = 9;
const float C_BACK_ARM_PIVOT_X = 24;
const float C_ARM_PIVOT_Y = 18;
const float C_POSE_FRONT_ARM = 55; //ft_a?
const float C_POSE_BACK_ARM = -75;

const AvatarLayer g_avatarLayers[] =
{
	{ AVATAR_SOURCE_CLOTHING, NULL, 0, 0, CLOTHING_BACK, 0, 0, false },
	{ AVATAR_SOURCE_SHEET, "player_arm.rttex", -1, -1, -1, 21, 18, true },
	{ AVATAR_SOURCE_SHEET, "player_body_only.rttex", 0, 0, -1, 0, 0, true },
	{ AVATAR_SOURCE_SHEET, "player_head.rttex", 0, 0, -1, 0, 0, true },
	{ AVATAR_SOURCE_SHEET, "player_extraleg.rttex", 0, 0, -1, 8, 28, true, 8, 28, 16, 1 },
	{ AVATAR_SOURCE_SHEET, "player_feet.rttex", 0, 1, -1, 0, 0, true },
	{ AVATAR_SOURCE_SHEET, "player_feet.rttex", 0, 2, -1, 0, 0, true },
	{ AVATAR_SOURCE_SHEET, "player_eyes.rttex", 0, 0, -1, 0, 0, false },
	{ AVATAR_SOURCE_SHEET, "player_eyes.rttex", 0, 4, -1, 0, 0, true },
	{ AVATAR_SOURCE_CLOTHING, NULL, 0, 1, CLOTHING_FEET, 0, 0, false },
	{ AVATAR_SOURCE_CLOTHING, NULL, 0, 2, CLOTHING_FEET, 0, 0, false },
	{ AVATAR_SOURCE_CLOTHING, NULL, 0, 0, CLOTHING_PANTS, 0, 0, false },
	{ AVATAR_SOURCE_CLOTHING, NULL, 0, 0, CLOTHING_SHIRT, 0, 0, false },
	{ AVATAR_SOURCE_CLOTHING, NULL, 0, 0, CLOTHING_NECKLACE, 0, 0, false },
	{ AVATAR_SOURCE_SHEET, "player_arm.rttex", -1, -1, -1, 6, 18, true },
	{ AVATAR_SOURCE_CLOTHING, NULL, 0, 0, CLOTHING_FACE, 0, 0, false },
	{ AVATAR_SOURCE_CLOTHING, NULL, 0, 0, CLOTHING_HAIR, 0, -16, false },
	{ AVATAR_SOURCE_CLOTHING, NULL, 0, 0, CLOTHING_HAT, 0, -16, false },
	{ AVATAR_SOURCE_CLOTHING, NULL, 0, 0, CLOTHING_HAND, -7, 18, false },
	{ AVATAR_SOURCE_CLOTHING, NULL, 0, 0, CLOTHING_ANCES, 0, 0, false }
};
