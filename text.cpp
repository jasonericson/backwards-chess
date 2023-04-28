#include "text.h"

#include "sprite.h"

Texture characters[256];

const uint16 TEXT_MAX = 128;
struct TextInstance
{
    uint32 id, sprite_id_start, sprite_id_end;
    int16 x, y;
    TextAlign align;
    float r, g, b, a;
};
uint32 text_next_id;

struct TextArray
{
    TextInstance data[TEXT_MAX];
    int count;
} texts;

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

    text_next_id = 1;
    texts.count = 0;
}

uint32 text_create(char* text, short x, short y, TextAlign align /* = Align_Left */, float r, float g, float b, float a)
{
    char* next_char = text;
    short next_x = x;

    if (align == Align_Center)
    {
        int num_chars = 0;
        while (*next_char != 0)
        {
            ++num_chars;
            ++next_char;
        }

        next_x = x - ((7 * num_chars) / 2);
        next_char = text;
    }
    else if (align == Align_Right)
    {
        int num_chars = 0;
        while (*next_char != 0)
        {
            ++num_chars;
            ++next_char;
        }

        next_x = x - (7 * num_chars);
        next_char = text;
    }

    uint32 start_id = 0;
    uint32 end_id = 0;
    uint32 curr_id = 0;
    while (*next_char != 0)
    {
        curr_id = sprite_create(&characters[*next_char], next_x, y, 1, r, g, b, a);
        if (start_id == 0)
            start_id = curr_id;

        next_x += 7;
        ++next_char;
    }

    if (curr_id != 0)
    {
        end_id = curr_id;
        uint32 this_id = text_next_id;
        texts.data[texts.count] = { this_id, start_id, end_id, x, y, align, r, g, b, a };
        ++texts.count;
        ++text_next_id;

        return this_id;
    }
    else
    {
        return 0;
    }
}

uint32 text_create(const char* text, short x, short y, TextAlign align, float r, float g, float b, float a)
{
    return text_create((char*)text, x, y, align, r, g, b, a);
}

void text_delete(uint32 id)
{
    for (int i = 0; i < texts.count; ++i)
    {
        if (texts.data[i].id == id)
        {
            uint32 curr_id = texts.data[i].sprite_id_start;
            while (curr_id <= texts.data[i].sprite_id_end)
            {
                sprite_delete(curr_id);
                ++curr_id;
            }

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
            while (curr_id <= t->sprite_id_end)
            {
                sprite_delete(curr_id);
                ++curr_id;
            }

            char* next_char = text;
            short next_x = t->x;

            if (t->align == Align_Center)
            {
                int num_chars = 0;
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
                int num_chars = 0;
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
                curr_id = sprite_create(&characters[*next_char], next_x, t->y, 1, t->r, t->g, t->b, t->a);
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
