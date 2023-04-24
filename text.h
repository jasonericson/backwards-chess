#pragma once

#include "main.h"

enum TextAlign
{
    Align_Left,
    Align_Center,
    Align_Right,
};

void text_init();
uint32 text_create(const char* text, short x, short y, TextAlign align = Align_Left, float r = 1.0f, float g = 1.0f, float b = 1.0f);
void text_delete(uint32 id);
