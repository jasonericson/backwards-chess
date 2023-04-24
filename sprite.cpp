#include "sprite.h"

SpriteArray sprites[2][SPRITE_LAYERS];
uint32 next_id;

void sprite_init()
{
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < SPRITE_LAYERS; ++j)
        {
            sprites[i][j].count = 0;
        }
    }

    next_id = 1;
}

uint32 sprite_create(Texture* tex, short x, short y, uint16 depth_layer, float r, float g, float b)
{
    if (depth_layer >= SPRITE_LAYERS)
        return 0;

    uint32 this_id = next_id;

    SpriteArray* array = &sprites[tex->map_id][depth_layer];

    array->data[array->count].id = this_id;
    array->data[array->count].x = x;
    array->data[array->count].y = y;
    array->data[array->count].tex = tex;
    array->data[array->count].smooth_pos = false;
    array->data[array->count].r = r;
    array->data[array->count].g = g;
    array->data[array->count].b = b;

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
        for (int layer = 0; layer < SPRITE_LAYERS; ++layer)
        {
            SpriteArray* array = &sprites[map_id][layer];
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

        if (found)
            break;
    }
}

Sprite* sprite_find(uint32 id)
{
    for (int map_id = 0; map_id < 2; ++map_id)
    {
        for (int layer = 0; layer < SPRITE_LAYERS; ++layer)
        {
            SpriteArray* array = &sprites[map_id][layer];
            for (int i = 0; i < array->count; ++i)
            {
                if (array->data[i].id == id)
                {
                    return &array->data[i];
                }
            }
        }
    }

    return nullptr;
}

void sprite_set_pos(uint32 id, short x, short y, bool smooth)
{
    Sprite* sprite = sprite_find(id);
    if (sprite != nullptr)
    {
        sprite->x = x;
        sprite->y = y;
        sprite->smooth_pos = smooth;
    }
}

void sprite_set_layer(uint32 id, uint16 depth_layer)
{
    if (depth_layer >= SPRITE_LAYERS)
        return;

    if (id == 0)
        return;

    for (int map_id = 0; map_id < 2; ++map_id)
    {
        bool found = false;
        for (int layer = 0; layer < SPRITE_LAYERS; ++layer)
        {
            SpriteArray* src_array = &sprites[map_id][layer];
            for (int i = 0; i < src_array->count; ++i)
            {
                if (src_array->data[i].id == id)
                {
                    Sprite sprite_data = src_array->data[i];

                    --src_array->count;
                    src_array->data[i] = src_array->data[src_array->count];

                    SpriteArray* dst_array = &sprites[sprite_data.tex->map_id][depth_layer];

                    dst_array->data[dst_array->count].id = id;
                    dst_array->data[dst_array->count].x = sprite_data.x;
                    dst_array->data[dst_array->count].y = sprite_data.y;
                    dst_array->data[dst_array->count].tex = sprite_data.tex;
                    dst_array->data[dst_array->count].smooth_pos = false;

                    ++dst_array->count;

                    found = true;
                    break;
                }
            }

            if (found)
                break;
        }

        if (found)
            break;
    }
}
