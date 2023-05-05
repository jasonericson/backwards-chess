#pragma once

#include "main.h"

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

struct TextSettings
{
    Font font = Font_Default;
    TextAlign align = Align_Left;
    int16 kerning = 0;
    int16 line_spacing = 1;
};

void text_init();
uint32 text_create(const char* text, short x, short y, TextSettings settings = TextSettings(), float w = 1.0f, float h = 1.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
uint32 text_create(char* text, short x, short y, TextSettings settings = TextSettings(), float w = 1.0f, float h = 1.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
void text_delete(uint32 id);
void text_change(uint32 id, char* text);
void text_set_color(uint32 id, float r, float g, float b);
void text_set_alpha(uint32 id, float a);
void text_set_char_scale(uint32 id, uint16 char_index, float w, float h);
void text_set_pos(uint32 id, int16 x, int16 y);
