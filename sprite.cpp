#include "sprite.h"

SpriteArray sprites[2];
uint32 next_id;

void sprite_init()
{
    for (int i = 0; i < 2; ++i)
    {
        sprites[i].count = 0;
    }

    next_id = 1;
}

uint32 sprite_create(Texture* tex, short x, short y)
{
    uint32 this_id = next_id;

    SpriteArray* array = sprites + tex->map_id;

    array->data[array->count].id = this_id;
    array->data[array->count].x = x;
    array->data[array->count].y = y;
    array->data[array->count].tex = tex;
    array->data[array->count].smooth_pos = false;

    ++array->count;
    ++next_id;

    return this_id;
}

void sprite_delete(uint32 id)
{
    if (id == 0)
        return;

    for (int map_id = 0; map_id < 2; ++map_id)
    {
        bool found = false;
        SpriteArray* array = sprites + map_id;
        for (int i = 0; i < array->count; ++i)
        {
            if (array->data[i].id == id)
            {
                --array->count;
                array->data[i] = array->data[array->count];

                found = true;
                break;
            }
        }

        if (found)
            break;
    }

}

void sprite_set_pos(uint32 id, short x, short y, bool smooth)
{
    for (int map_id = 0; map_id < 2; ++map_id)
    {
        bool found = false;
        SpriteArray* array = sprites + map_id;
        for (int i = 0; i < array->count; ++i)
        {
            if (array->data[i].id == id)
            {
                array->data[i].x = x;
                array->data[i].y = y;
                array->data[i].smooth_pos = smooth;

                found = true;
                break;
            }
        }

        if (found)
            break;
    }
}
