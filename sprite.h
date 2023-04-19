#pragma once

#include "main.h"

#include <SDL.h>

struct Texture
{
    short width, height;
    float u1, u2, v1, v2;
};

struct Sprite
{
    uint32 id;
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
uint32 sprite_create(Texture* tex, short x, short y);
void sprite_delete(uint32 id);
void sprite_set_pos(uint32 id, short x, short y);
