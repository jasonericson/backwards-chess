#pragma once

#include "main.h"

#include <SDL.h>

struct Texture
{
    short map_id;
    short width, height;
    float u1, u2, v1, v2;
};

struct Sprite
{
    uint32 id;
    short x, y;
    bool smooth_pos;
    Texture* tex;
    Uint64 timeCreated;
    float r, g, b;
};

const int SPRITE_MAX = 128;

struct SpriteArray
{
    Sprite data[SPRITE_MAX];
    int count;
};

const uint16 SPRITE_LAYERS = 3;

extern SpriteArray sprites[2][SPRITE_LAYERS];

void sprite_init();
uint32 sprite_create(Texture* tex, short x, short y, uint16 depth_layer, float r = 1.0f, float g = 1.0f, float b = 1.0f);
void sprite_delete(uint32 id);
Sprite* sprite_find(uint32 id);
void sprite_set_pos(uint32 id, short x, short y, bool smooth = false);
void sprite_set_layer(uint32 id, uint16 depth_layer);
