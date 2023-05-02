#include "sprite.h"

SpriteArray sprites[NUM_MAPS];
const char* spritemap_filenames[NUM_MAPS] = { "font.png", "pieces.png", "title_font.png" };
uint32 next_id;

void sprite_init()
{
    for (int i = 0; i < 2; ++i)
    {
        sprites[i].count = 0;
    }

    next_id = 1;
}

void debug_check_sprites()
{
    // debug check that everything's cool here
    for (uint16 map_id = 0; map_id < NUM_MAPS; ++map_id)
    {
        SpriteArray* array = &sprites[map_id];
        int16 last_layer = INT16_MIN;
        for (uint16 i = 0; i < array->count; ++i)
        {
            // check that layers are in order
            SDL_assert(array->data[i].depth_layer >= last_layer);
            last_layer = array->data[i].depth_layer;

            // check for duplicate IDs
            for (uint16 map_id_2 = 0; map_id_2 < NUM_MAPS; ++map_id_2)
            {
                for (uint16 j = 0; j < sprites[map_id_2].count; ++j)
                {
                    if (map_id == map_id_2 && i == j)
                        continue;
                    
                    SDL_assert(array->data[i].id != sprites[map_id_2].data[j].id);
                }
            }
        }
    }
}

uint32 sprite_create(Texture* tex, short x, short y, int16 depth_layer, float w, float h, float r, float g, float b, float a)
{
    uint32 this_id = next_id;
    Sprite insert_sprite;
    insert_sprite.id = this_id;
    insert_sprite.x = x;
    insert_sprite.y = y;
    insert_sprite.w = w;
    insert_sprite.h = h;
    insert_sprite.depth_layer = depth_layer;
    insert_sprite.tex = tex;
    insert_sprite.smooth_pos = false;
    insert_sprite.r = r;
    insert_sprite.g = g;
    insert_sprite.b = b;
    insert_sprite.a = a;

    SpriteArray* array = &sprites[tex->map_id];
    
    SDL_assert(array->count < SPRITE_MAX);

    uint16 i;
    for (i = 0; i < array->count; ++i)
    {
        if (array->data[i].depth_layer > insert_sprite.depth_layer)
        {
            Sprite swap_sprite = array->data[i];
            array->data[i] = insert_sprite;
            insert_sprite = swap_sprite;
        }
    }

    array->data[i] = insert_sprite;

    ++array->count;
    ++next_id;

#if _DEBUG
    debug_check_sprites();
#endif

    return this_id;
}

void sprite_delete(uint32 id)
{
    if (id == 0)
        return;

    int32 replace_index = -1;
    int16 layer;

    for (int map_id = 0; map_id < NUM_MAPS; ++map_id)
    {
        SpriteArray* array = &sprites[map_id];
        uint16 i;
        for (i = 0; i < array->count; ++i)
        {
            if (replace_index < 0)
            {
                if (array->data[i].id == id)
                {
                    replace_index = i;
                    layer = array->data[i].depth_layer;
                }
            }
            else
            {
                if (array->data[i].depth_layer > layer)
                {
                    if (replace_index != i - 1)
                    {
                        array->data[replace_index] = array->data[i - 1];
                        replace_index = i - 1;
                    }
                    layer = array->data[i].depth_layer;
                }
            }
        }

        if (replace_index >= 0)
        {
            if (replace_index != i - 1)
            {
                array->data[replace_index] = array->data[i - 1];
            }
            
            --array->count;
            break;
        }
    }

#ifdef _DEBUG
    debug_check_sprites();
#endif
}

Sprite* sprite_find(uint32 id)
{
    for (int map_id = 0; map_id < NUM_MAPS; ++map_id)
    {
        SpriteArray* array = &sprites[map_id];
        for (int i = 0; i < array->count; ++i)
        {
            if (array->data[i].id == id)
            {
                return &array->data[i];
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

void sprite_set_layer(uint32 id, int16 depth_layer)
{
    if (id == 0)
        return;

    int32 src_index = -1;
    int16 src_layer;
    
    for (uint16 map_id = 0; map_id < NUM_MAPS; ++map_id)
    {
        bool found = false;
        SpriteArray* array = &sprites[map_id];
        for (uint16 i = 0; i < array->count; ++i)
        {
            if (src_index < 0)
            {
                if (array->data[i].id == id)
                {
                    src_index = i;
                    src_layer = array->data[i].depth_layer;
                    array->data[i].depth_layer = depth_layer;

                    found = true;
                    break;
                }
            }
        }

        if (src_index >= 0)
        {
            if (depth_layer > src_layer)
            {
                uint16 i;
                for (i = src_index + 1; i < array->count; ++i)
                {
                    if (array->data[i].depth_layer > src_layer)
                    {
                        Sprite swap = array->data[src_index];
                        array->data[src_index] = array->data[i - 1];
                        array->data[i - 1] = swap;

                        if (array->data[i].depth_layer >= depth_layer)
                        {
                            src_index = -1;
                            break;
                        }

                        src_index = i - 1;
                        src_layer = array->data[i].depth_layer;
                    }
                }

                if (src_index >= 0)
                {
                    Sprite swap = array->data[src_index];
                    array->data[src_index] = array->data[i - 1];
                    array->data[i - 1] = swap;
                }
            }
            else if (depth_layer < src_layer)
            {
                int32 i;
                for (i = src_index - 1; i >= 0; --i)
                {
                    if (array->data[i].depth_layer < src_layer)
                    {
                        Sprite swap = array->data[src_index];
                        array->data[src_index] = array->data[i + 1];
                        array->data[i + 1] = swap;

                        if (array->data[i].depth_layer <= depth_layer)
                        {
                            src_index = -1;
                            break;
                        }

                        src_index = i + 1;
                        src_layer = array->data[i].depth_layer;
                    }
                }

                if (src_index >= 0)
                {
                    Sprite swap = array->data[src_index];
                    array->data[src_index] = array->data[i + 1];
                    array->data[i + 1] = swap;
                }
            }
        }

        if (found)
            break;
    }

#if _DEBUG
    debug_check_sprites();
#endif
}

void sprite_set_color(uint32 id, float r, float g, float b, float a)
{
    Sprite* sprite = sprite_find(id);
    if (sprite != nullptr)
    {
        sprite->r = r;
        sprite->g = g;
        sprite->b = b;
        sprite->a = a;
    }
}

void sprite_set_scale(uint32 id, float w, float h)
{
    Sprite* sprite = sprite_find(id);
    if (sprite != nullptr)
    {
        sprite->w = w;
        sprite->h = h;
    }
}
