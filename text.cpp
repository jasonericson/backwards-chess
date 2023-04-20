#include "text.h"

#include "sprite.h"

Texture characters[256];

void text_init()
{
    const char* order = "~1234567890-+!@#$%^&*()_={}[]|\\:;\"'<,>.?/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int16 col = 0;
    int16 row = 0;
    for (int i = 0; i < 93; ++i)
    {
        characters[order[i]] = { 1, 8, 8, (float)col * (8.0f / 128.0f), (float)(col + 1) * (8.0f / 128.0f), (float)row * (8.0f / 128.0f), (float)(row + 1) * (8.0f / 128.0f) };
        col += 1;
        if (col > 11)
        {
            col = 0;
            row += 1;
        }
    }
}

void text_create(const char* text, short x, short y)
{
    char* next_char = (char *)text;
    short next_x = x;
    while (*next_char != 0)
    {
        sprite_create(&characters[*next_char], next_x, y);
        next_x += 7;
        ++next_char;
    }
}
