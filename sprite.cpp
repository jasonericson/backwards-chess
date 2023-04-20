#include "sprite.h"

SpriteArray sprites;
uint32 next_id;

void sprite_init()
{
    sprites.count = 0;
    next_id = 1;
}

uint32 sprite_create(Texture* tex, short x, short y)
{
    uint32 this_id = next_id;
    sprites.data[sprites.count].id = this_id;
    sprites.data[sprites.count].x = x;
    sprites.data[sprites.count].y = y;
    sprites.data[sprites.count].tex = tex;

    ++sprites.count;
    ++next_id;

    return this_id;
}

void sprite_delete(uint32 id)
{
    if (id == 0)
        return;

    for (int i = 0; i < sprites.count; ++i)
    {
        if (sprites.data[i].id == id)
        {
            --sprites.count;
            sprites.data[i] = sprites.data[sprites.count];
            break;
        }
    }
}

void sprite_set_pos(uint32 id, short x, short y)
{
    for (int i = 0; i < sprites.count; ++i)
    {
        if (sprites.data[i].id == id)
        {
            sprites.data[i].x = x;
            sprites.data[i].y = y;
            break;
        }
    }
}
