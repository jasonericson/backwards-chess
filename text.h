#pragma once

#include "main.h"

enum TextAlign
{
    Align_Left,
    Align_Center,
    Align_Right,
};

void text_init();
uint32 text_create(char* text, short x, short y, TextAlign align = Align_Left);
void text_delete(uint32 id);
