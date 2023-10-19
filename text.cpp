#include "text.h"

#include "sprite.h"
#include <stdio.h>
#include <string>

const uint16 TEXT_MAX = 128;
const uint16 NUM_FONTS = 2;
struct TextInstance
{
    uint32 id, sprite_id_start, sprite_id_end;
    TextSettings settings;
    int16 x, y;
    float w, h;
    SpriteLayer depth_layer;
    float r, g, b, a;
};
uint32 text_next_id;

struct TextArray
{
    TextInstance data[TEXT_MAX];
    uint16 count;
} texts;

CharSettings characters[NUM_FONTS][256];
FontSettings fonts[NUM_FONTS];

void text_init()
{
    FILE* fp = fopen("font.fnt", "r");
    if (fp == NULL)
    {
        printf("Error: could not open file 'font.fnt'\n");
    }

    char line[256];
    while (fgets(line, 256, fp))
    {
        char tag[16];
        uint16 i = 0;
        uint16 j;
        for (j = 0; line[i] != ' '; ++i, ++j)
        {
            tag[j] = line[i];
        }
        tag[j] = 0;

        if (strcmp(tag, "char") != 0 && strcmp(tag, "common") != 0)
            continue;

        char c;
        uint16 x, y, w, h;
        CharSettings cs;

        while (true)
        {
            while (line[i] == ' ')
                ++i;

            char key[16];
            for (j = 0; line[i] != ' ' && line[i] != '='; ++i, ++j)
            {
                key[j] = line[i];
            }
            key[j] = 0;

            while (line[i] == ' ' || line[i] == '=')
                ++i;

            char val[16];
            for (j = 0; line[i] != ' ' && line[i] != '\0' && line[i] != '\n'; ++i, ++j)
            {
                val[j] = line[i];
            }
            val[j] = 0;

            if (strcmp(tag, "char") == 0)
            {
                if (strcmp(key, "id") == 0)
                {
                    c = atoi(val);
                }
                else if (strcmp(key, "x") == 0)
                {
                    x = atoi(val);
                }
                else if (strcmp(key, "y") == 0)
                {
                    y = atoi(val);
                }
                else if (strcmp(key, "width") == 0)
                {
                    w = atoi(val);
                }
                else if (strcmp(key, "height") == 0)
                {
                    h = atoi(val);
                }
                else if (strcmp(key, "xoffset") == 0)
                {
                    cs.x_offset = atoi(val);
                }
                else if (strcmp(key, "yoffset") == 0)
                {
                    cs.y_offset = atoi(val);
                }
                else if (strcmp(key, "xadvance") == 0)
                {
                    cs.x_advance = atoi(val);
                }
            }
            else if (strcmp(tag, "common") == 0)
            {
                if (strcmp(key, "lineHeight") == 0)
                {
                    fonts[0].line_height = atoi(val);
                }
                else if (strcmp(key, "base") == 0)
                {
                    fonts[0].base = atoi(val);
                }
            }

            while (line[i] == ' ')
                ++i;

            if (line[i] == '\0' || line[i] == '\n')
            {
                break;
            }
        }

        if (strcmp(tag, "char") == 0)
        {
            cs.tex = { MapId_Font, w, h, x, (uint16)(x + w), y, (uint16)(y + h) };
            characters[Font_Default][c] = cs;
        }
    }

    text_next_id = 1;
    texts.count = 0;

    // title font
    characters[Font_Title][' '] = {{ MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 63, 64, 63, 64 }, 0, 0, (int16)(11 * 1.5) };
    characters[Font_Title]['B'] = {{ MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5),  0, 11,  0, 17 }, 0, 0, (int16)(11 * 1.5) };
    characters[Font_Title]['C'] = {{ MapId_FontTitle, (int16)(12 * 1.5), (int16)(17 * 1.5),  0, 12, 36, 53 }, 0, 0, (int16)(12 * 1.5) };
    characters[Font_Title]['a'] = {{ MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 12, 23,  0, 17 }, 0, 0, (int16)(11 * 1.5) };
    characters[Font_Title]['c'] = {{ MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 24, 35,  0, 17 }, 0, 0, (int16)(11 * 1.5) };
    characters[Font_Title]['d'] = {{ MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 26, 37, 18, 35 }, 0, 0, (int16)(11 * 1.5) };
    characters[Font_Title]['e'] = {{ MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 25, 36, 36, 53 }, 0, 0, (int16)(11 * 1.5) };
    characters[Font_Title]['h'] = {{ MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 13, 24, 36, 53 }, 0, 0, (int16)(11 * 1.5) };
    characters[Font_Title]['k'] = {{ MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 36, 47,  0, 17 }, 0, 0, (int16)(11 * 1.5) };
    characters[Font_Title]['r'] = {{ MapId_FontTitle, (int16)(11 * 1.5), (int16)(17 * 1.5), 14, 25, 18, 35 }, 0, 0, (int16)(11 * 1.5) };
    characters[Font_Title]['s'] = {{ MapId_FontTitle, (int16)( 9 * 1.5), (int16)(17 * 1.5), 38, 47, 18, 35 }, 0, 0, (int16)( 9 * 1.5) };
    characters[Font_Title]['w'] = {{ MapId_FontTitle, (int16)(13 * 1.5), (int16)(17 * 1.5),  0, 13, 18, 35 }, 0, 0, (int16)(13 * 1.5) };

    fonts[1].line_height = 17;
    fonts[1].base = 17;
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
        uint16 total_width = 0;
        // walk through current row and find total width (for center or right align) and tallest height (for y pos)
        while (*next_char_in_row != 0 && *next_char_in_row != '\n')
        {
            total_width += characters[settings.font][*next_char_in_row].x_advance;
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
            next_y -= (fonts[settings.font].line_height + settings.line_spacing);

        // create a sprite for each letter of this row
        while (*next_char != 0 && *next_char != '\n')
        {
            CharSettings* cs = &characters[settings.font][*next_char];
            curr_id = sprite_create(&cs->tex, next_x + cs->x_offset, next_y + (fonts[settings.font].base - cs->tex.height) - cs->y_offset, depth_layer, w, h, 0.0f, r, g, b, a);
            if (start_id == 0)
                start_id = curr_id;

            next_x += cs->x_advance + settings.kerning;
            ++next_char;
        }
    }

    if (curr_id != 0)
    {
        end_id = curr_id;
        uint32 this_id = text_next_id;
        texts.data[texts.count] = { this_id, start_id, end_id, settings, x, y, w, h, depth_layer, r, g, b, a };
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

void text_change(uint32 id, char* text)
{
    for (uint16 i = 0; i < texts.count; ++i)
    {
        if (texts.data[i].id == id)
        {
            TextInstance* t = &texts.data[i];

            // delete sprites
            uint32 curr_id = t->sprite_id_start;
            while (curr_id != 0 && curr_id <= t->sprite_id_end)
            {
                sprite_delete(curr_id);
                ++curr_id;
            }

            char* next_char = text;
            uint16 next_x;
            uint16 next_y = t->y;
            
            t->sprite_id_start = 0;
            t->sprite_id_end = 0;
            curr_id = 0;
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
                uint16 total_width = 0;
                // walk through current row and find total width (for center or right align) and tallest height (for y pos)
                while (*next_char_in_row != 0 && *next_char_in_row != '\n')
                {
                    total_width += characters[t->settings.font][*next_char_in_row].x_advance;
                    ++next_char_in_row;
                }

                // set this row's x position based on alignment
                if (t->settings.align == Align_Center)
                    next_x = t->x - (total_width / 2);
                else if (t->settings.align == Align_Right)
                    next_x = t->x - total_width;
                else
                    next_x = t->x;

                // set this row's y position
                if (row > 0)
                    next_y -= (fonts[t->settings.font].line_height + t->settings.line_spacing);

                // create a sprite for each letter of this row
                while (*next_char != 0 && *next_char != '\n')
                {
                    CharSettings* cs = &characters[t->settings.font][*next_char];

                    curr_id = sprite_create(&cs->tex, next_x + cs->x_offset, next_y + (fonts[t->settings.font].base - cs->tex.height) - cs->y_offset, t->depth_layer, t->w, t->h, 0.0f, t->r, t->g, t->b, t->a);
                    if (t->sprite_id_start == 0)
                        t->sprite_id_start = curr_id;

                    next_x += cs->x_advance + t->settings.kerning;
                    ++next_char;
                }
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
            t->w = w;
            t->h = h;

            uint32 sprite_id = t->sprite_id_start + char_index;
            SDL_assert(sprite_id <= t->sprite_id_end);
            sprite_set_scale(sprite_id, t->w, t->h);

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
