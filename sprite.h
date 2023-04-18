#pragma once

#include "main.h"

#include <SDL.h>

struct Texture
{
	float u1, u2, v1, v2;
};

struct Sprite
{
	short x, y;
	Texture* tex;
	Uint64 timeCreated;
};

const int SPRITE_MAX = 128;

struct SpriteArray
{
	Sprite data[SPRITE_MAX];
	int count;
};

extern SpriteArray sprites;

void sprite_init();
void sprite_create(Texture* tex, short x, short y);
