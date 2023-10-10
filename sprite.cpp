#include "sprite.h"

SpriteArray sprites;
const char* spritemap_filenames[NUM_MAPS] = { "font.png", "pieces.png", "title_font.png" };
uint32 next_id;

void sprite_init()
{
    for (int i = 0; i < 2; ++i)
    {
        sprites.count = 0;
    }

    next_id = 1;
}

void debug_check_sprites()
{
    // debug check that everything's cool here
    int16 last_layer = INT16_MIN;
    SpriteMapId last_map_id = (SpriteMapId)0;
    for (uint16 i = 0; i < sprites.count; ++i)
    {
        // check that layers are in order
        SDL_assert(sprites.data[i].depth_layer > last_layer || (sprites.data[i].depth_layer == last_layer && sprites.data[i].tex->map_id >= last_map_id));
        last_layer = sprites.data[i].depth_layer;
        last_map_id = sprites.data[i].tex->map_id;
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
    
    SDL_assert(sprites.count < SPRITE_MAX);

    uint16 i;
    for (i = 0; i < sprites.count; ++i)
    {
        if (sprites.data[i].depth_layer > insert_sprite.depth_layer ||
            (sprites.data[i].tex->map_id > insert_sprite.tex->map_id && sprites.data[i].depth_layer == insert_sprite.depth_layer))
        {
            Sprite swap_sprite = sprites.data[i];
            sprites.data[i] = insert_sprite;
            insert_sprite = swap_sprite;
        }
    }

    sprites.data[i] = insert_sprite;

    ++sprites.count;
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
    SpriteMapId map_id;

    uint16 i;
    for (i = 0; i < sprites.count; ++i)
    {
        if (replace_index < 0)
        {
            if (sprites.data[i].id == id)
            {
                replace_index = i;
                layer = sprites.data[i].depth_layer;
                map_id = sprites.data[i].tex->map_id;
            }
        }
        else
        {
            if (sprites.data[i].depth_layer > layer ||
                (sprites.data[i].tex->map_id > map_id && sprites.data[i].depth_layer == layer))
            {
                if (replace_index != i - 1)
                {
                    sprites.data[replace_index] = sprites.data[i - 1];
                    replace_index = i - 1;
                }
                layer = sprites.data[i].depth_layer;
                map_id = sprites.data[i].tex->map_id;
            }
        }
    }

    if (replace_index >= 0)
    {
        if (replace_index != i - 1)
        {
            sprites.data[replace_index] = sprites.data[i - 1];
        }
        
        --sprites.count;
    }

#ifdef _DEBUG
    debug_check_sprites();
#endif
}

Sprite* sprite_find(uint32 id)
{
    for (int i = 0; i < sprites.count; ++i)
    {
        if (sprites.data[i].id == id)
        {
            return &sprites.data[i];
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
    Sprite src_sprite;

    for (uint16 i = 0; i < sprites.count; ++i)
    {
        if (sprites.data[i].id == id)
        {
            src_index = i;
            src_sprite = sprites.data[i];
            sprites.data[i].depth_layer = depth_layer;

            break;
        }
    }

    if (src_index >= 0)
    {
        if (depth_layer > src_sprite.depth_layer)
        {
            uint16 i;
            for (i = src_index + 1; i < sprites.count; ++i)
            {
                if (sprites.data[i].depth_layer > src_sprite.depth_layer ||
                    (sprites.data[i].tex->map_id > src_sprite.tex->map_id && sprites.data[i].depth_layer == src_sprite.depth_layer))
                {
                    Sprite swap = sprites.data[src_index];
                    sprites.data[src_index] = sprites.data[i - 1];
                    sprites.data[i - 1] = swap;

                    if (sprites.data[i].depth_layer >= depth_layer)
                    {
                        src_index = -1;
                        break;
                    }

                    src_index = i - 1;
                    src_sprite = sprites.data[i];
                }
            }

            if (src_index >= 0)
            {
                Sprite swap = sprites.data[src_index];
                sprites.data[src_index] = sprites.data[i - 1];
                sprites.data[i - 1] = swap;
            }
        }
        else if (depth_layer < src_sprite.depth_layer)
        {
            int32 i;
            for (i = src_index - 1; i >= 0; --i)
            {
                if (sprites.data[i].depth_layer < src_sprite.depth_layer ||
                    (sprites.data[i].tex->map_id < src_sprite.tex->map_id && sprites.data[i].depth_layer == src_sprite.depth_layer))
                {
                    Sprite swap = sprites.data[src_index];
                    sprites.data[src_index] = sprites.data[i + 1];
                    sprites.data[i + 1] = swap;

                    if (sprites.data[i].depth_layer <= depth_layer)
                    {
                        src_index = -1;
                        break;
                    }

                    src_index = i + 1;
                    src_sprite = sprites.data[i];
                }
            }

            if (src_index >= 0)
            {
                Sprite swap = sprites.data[src_index];
                sprites.data[src_index] = sprites.data[i + 1];
                sprites.data[i + 1] = swap;
            }
        }
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
