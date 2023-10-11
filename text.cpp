#include "text.h"

#include "sprite.h"


const uint16 TEXT_MAX = 128;
const uint16 NUM_FONTS = 2;
struct TextInstance
{
    uint32 id, sprite_id_start, sprite_id_end;
    int16 x, y;
    SpriteLayer depth_layer;
    TextAlign align;
    float r, g, b, a;
};
uint32 text_next_id;

struct TextArray
{
    TextInstance data[TEXT_MAX];
    uint16 count;
} texts;

Texture characters[NUM_FONTS][256];

void text_init()
{
    // default font
    const char* order = "~1234567890-+!@#$%^&*()_={}[]|\\:;\"'<,>.?/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    uint16 col = 0;
    uint16 row = 0;
    for (uint16 i = 0; i < 93; ++i)
    {
        characters[Font_Default][order[i]] = { MapId_Font, 8, 8, (uint16)(col * 8), (uint16)((col + 1) * 8), (uint16)(row * 8), (uint16)((row + 1) * 8) };
        col += 1;
        if (col > 11)
        {
            col = 0;
            row += 1;
        }
    }

    text_next_id = 1;
    texts.count = 0;

    // title font
    characters[Font_Title][' '] = { MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 63, 64, 63, 64 };
    characters[Font_Title]['B'] = { MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5),  0, 11,  0, 17 };
    characters[Font_Title]['C'] = { MapId_FontTitle, (int16)(12 * 1.5), (int16)(17 * 1.5),  0, 12, 36, 53 };
    characters[Font_Title]['a'] = { MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 12, 23,  0, 17 };
    characters[Font_Title]['c'] = { MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 24, 35,  0, 17 };
    characters[Font_Title]['d'] = { MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 26, 37, 18, 35 };
    characters[Font_Title]['e'] = { MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 25, 36, 36, 53 };
    characters[Font_Title]['h'] = { MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 13, 24, 36, 53 };
    characters[Font_Title]['k'] = { MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 36, 47,  0, 17 };
    characters[Font_Title]['r'] = { MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 14, 25, 18, 35 };
    characters[Font_Title]['s'] = { MapId_FontTitle, (int16)( 9 * 1.5), (int16)(17 * 1.5), 38, 47, 18, 35 };
    characters[Font_Title]['w'] = { MapId_FontTitle, (int16)(13 * 1.5), (int16)(17 * 1.5),  0, 13, 18, 35 };
}

uint32 text_create(char* text, int16 x, int16 y, SpriteLayer depth_layer, TextSettings settings, float w, float h, float r, float g, float b, float a)
{
    char* next_char = text;
    uint16 next_x;
    uint16 next_y = y;
    
    uint32 start_id = 0;
    uint32 end_id = 0;
    uint32 curr_id = 0;
    uint16 row = 0;

    while (*next_char != 0)
    {
        if (*next_char == '\n')
        {
            // start another row when we hit a newline
            ++next_char;
            ++row;
        }
        char* next_char_in_row = next_char;
        uint16 tallest_height = 0;
        uint16 total_width = 0;
        // walk through current row and find total width (for center or right align) and tallest height (for y pos)
        while (*next_char_in_row != 0 && *next_char_in_row != '\n')
        {
            total_width += characters[settings.font][*next_char_in_row].width;
            tallest_height = SDL_max(tallest_height, characters[settings.font][*next_char_in_row].height);
            ++next_char_in_row;
        }

        // set this row's x position based on alignment
        if (settings.align == Align_Center)
            next_x = x - (total_width / 2);
        else if (settings.align == Align_Right)
            next_x = x - total_width;
        else
            next_x = x;

        // set this row's y position
        if (row > 0)
            next_y -= (tallest_height + settings.line_spacing);

        // create a sprite for each letter of this row
        while (*next_char != 0 && *next_char != '\n')
        {
            Texture* tex = &characters[settings.font][*next_char];
            curr_id = sprite_create(tex, next_x, next_y, depth_layer, w, h, r, g, b, a);
            if (start_id == 0)
                start_id = curr_id;

            next_x += tex->width + settings.kerning;
            ++next_char;
        }
    }

    if (curr_id != 0)
    {
        end_id = curr_id;
        uint32 this_id = text_next_id;
        texts.data[texts.count] = { this_id, start_id, end_id, x, y, depth_layer, settings.align, r, g, b, a };
        ++texts.count;
        ++text_next_id;

        return this_id;
    }
    else
    {
        return 0;
    }
}

uint32 text_create(const char* text, int16 x, int16 y, SpriteLayer depth_layer, TextSettings settings, float w, float h, float r, float g, float b, float a)
{
    return text_create((char*)text, x, y, depth_layer, settings, w, h, r, g, b, a);
}

void text_delete(uint32 id)
{
    for (uint16 i = 0; i < texts.count; ++i)
    {
        if (texts.data[i].id == id)
        {
            // delete sprites
            uint32 curr_id = texts.data[i].sprite_id_start;
            while (curr_id <= texts.data[i].sprite_id_end)
            {
                sprite_delete(curr_id);
                ++curr_id;
            }

            // delete text instance
            --texts.count;
            texts.data[i] = texts.data[texts.count];

            break;
        }
    }
}

// @TODO: update for variable width fonts (maybe just reuse text_create?)
void text_change(uint32 id, char* text)
{
    for (uint16 i = 0; i < texts.count; ++i)
    {
        if (texts.data[i].id == id)
        {
            TextInstance* t = &texts.data[i];

            // delete sprites
            uint32 curr_id = t->sprite_id_start;
            while (curr_id <= t->sprite_id_end)
            {
                sprite_delete(curr_id);
                ++curr_id;
            }

            char* next_char = text;
            int16 next_x = t->x;

            if (t->align == Align_Center)
            {
                int16 num_chars = 0;
                while (*next_char != 0)
                {
                    ++num_chars;
                    ++next_char;
                }

                next_x = t->x - ((7 * num_chars) / 2);
                next_char = text;
            }
            else if (t->align == Align_Right)
            {
                int16 num_chars = 0;
                while (*next_char != 0)
                {
                    ++num_chars;
                    ++next_char;
                }

                next_x = t->x - (7 * num_chars);
                next_char = text;
            }

            t->sprite_id_start = 0;
            t->sprite_id_end = 0;
            curr_id = 0;
            while (*next_char != 0)
            {
                curr_id = sprite_create(&characters[Font_Default][*next_char], next_x, t->y, t->depth_layer, 1.0f, 1.0f, t->r, t->g, t->b, t->a);
                if (t->sprite_id_start == 0)
                    t->sprite_id_start = curr_id;

                next_x += 7;
                ++next_char;
            }

            if (curr_id != 0)
            {
                t->sprite_id_end = curr_id;
            }

            break;
        }
    }
}

void text_set_color(uint32 id, float r, float g, float b)
{
    for (uint16 i = 0; i < texts.count; ++i)
    {
        if (texts.data[i].id == id)
        {
            TextInstance* t = &texts.data[i];
            t->r = r;
            t->g = g;
            t->b = b;
            
            for (uint32 curr_id = t->sprite_id_start; curr_id <= t->sprite_id_end; ++curr_id)
            {
                sprite_set_color(curr_id, r, g, b, t->a);
            }

            break;
        }
    }
}

void text_set_alpha(uint32 id, float a)
{
    for (uint16 i = 0; i < texts.count; ++i)
    {
        if (texts.data[i].id == id)
        {
            TextInstance* t = &texts.data[i];
            t->a = a;
            
            for (uint32 curr_id = t->sprite_id_start; curr_id <= t->sprite_id_end; ++curr_id)
            {
                sprite_set_color(curr_id, t->r, t->g, t->b, a);
            }

            break;
        }
    }
}

void text_set_char_scale(uint32 id, uint16 char_index, float w, float h)
{
    for (uint16 i = 0; i < texts.count; ++i)
    {
        if (texts.data[i].id == id)
        {
            TextInstance* t = &texts.data[i];
            uint32 sprite_id = t->sprite_id_start + char_index;
            SDL_assert(sprite_id <= t->sprite_id_end);
            sprite_set_scale(sprite_id, w, h);

            break;
        }
    }
}

void text_set_pos(uint32 id, int16 x, int16 y)
{   
    for (uint16 i = 0; i < texts.count; ++i)
    {
        if (texts.data[i].id == id)
        {
            TextInstance* t = &texts.data[i];

            for (uint16 sprite_id = t->sprite_id_start; sprite_id <= t->sprite_id_end; ++sprite_id)
            {
                Sprite* sprite = sprite_find(sprite_id);
                int16 offset_x = sprite->x - t->x;
                int16 offset_y = sprite->y - t->y;
                sprite->x = x + offset_x;
                sprite->y = y + offset_y;
            }

            t->x = x;
            t->y = y;

            break;
        }
    }

}
