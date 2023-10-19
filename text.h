#pragma once

#include "main.h"

#include "sprite.h"

enum TextAlign
{
    Align_Left,
    Align_Center,
    Align_Right,
};

enum Font : uint16
{
    Font_Default = 0,
    Font_Title = 1,
};

struct FontSettings
{
    uint16 line_height, base;
};

struct TextSettings
{
    Font font = Font_Default;
    TextAlign align = Align_Left;
    int16 kerning = 0;
    int16 line_spacing = 1;
    float scale = 1.0f;
};

struct CharSettings
{
    Texture tex;
    int16 x_offset, y_offset, x_advance;
};

void text_init();
uint32 text_create(const char* text, int16 x, int16 y, SpriteLayer depth_layer, TextSettings settings = TextSettings(), float w = 1.0f, float h = 1.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
uint32 text_create(char* text, int16 x, int16 y, SpriteLayer depth_layer, TextSettings settings = TextSettings(), float w = 1.0f, float h = 1.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
void text_delete(uint32 id);
void text_change(uint32 id, char* text);
void text_set_color(uint32 id, float r, float g, float b);
void text_set_alpha(uint32 id, float a);
void text_set_char_scale(uint32 id, uint16 char_index, float w, float h);
void text_set_pos(uint32 id, int16 x, int16 y);
