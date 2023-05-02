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

void text_init();
uint32 text_create(const char* text, short x, short y, TextAlign align = Align_Left, Font font = Font_Default, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
uint32 text_create(char* text, short x, short y, TextAlign align = Align_Left, Font font = Font_Default, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
void text_delete(uint32 id);
void text_change(uint32 id, char* text);
void text_set_color(uint32 id, float r, float g, float b);
void text_set_alpha(uint32 id, float a);
