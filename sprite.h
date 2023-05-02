#pragma once

#include "main.h"

#include <SDL.h>

enum SpriteMapId : uint16
{
    MapId_Font = 0,
    MapId_Pieces = 1,
};

struct Texture
{
    SpriteMapId map_id;
    short width, height;
    uint16 u1, u2, v1, v2;
};

struct Sprite
{
    uint32 id;
    short x, y;
    float w, h;
    int16 depth_layer;
    bool smooth_pos;
    Texture* tex;
    Uint64 timeCreated;
    float r, g, b, a;
};

const int SPRITE_MAX = 512;

struct SpriteArray
{
    Sprite data[SPRITE_MAX];
    int count;
};

const uint16 NUM_MAPS = 3;

extern SpriteArray sprites[NUM_MAPS];
extern const char* spritemap_filenames[NUM_MAPS];

void sprite_init();
uint32 sprite_create(Texture* tex, short x, short y, int16 depth_layer, float w = 1.0f, float h = 1.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
void sprite_delete(uint32 id);
Sprite* sprite_find(uint32 id);
void sprite_set_pos(uint32 id, short x, short y, bool smooth = false);
void sprite_set_layer(uint32 id, int16 depth_layer);
void sprite_set_color(uint32 id, float r, float g, float b, float a = 1.0f);
