#include "title.h"

#include "sprite.h"
#include "text.h"

Texture square_tex2 = Texture{ MapId_Pieces, 14, 14,  48, 49, 31, 32 };

uint32 title_id;
uint32 top_bar_id;
uint32 bot_bar_id;
uint32 fade_id;

void title_init()
{
    TextSettings text_settings;
    text_settings.font = Font_Title;
    text_settings.line_spacing = 10;
    text_settings.kerning = 1;
    title_id = text_create("Backwards\nChess", 4, 102, Layer_Board, text_settings, -1.0f);

    top_bar_id = sprite_create(&square_tex2, 73, 150, Layer_PlacedPiece, 12.0f, 3.0f, 0.0f, 0.0f, 0.0f);
    bot_bar_id = sprite_create(&square_tex2, 73, -2, Layer_PlacedPiece, 12.0f, 3.0f, 0.0f, 0.0f, 0.0f);

    fade_id = sprite_create(&square_tex2, 80, 80, Layer_HeldPiece, 12.0f, 12.0f, 0.0f, 0.0f, 0.0f);
}

float title_char_w = 1.0f;
float title_spin_dir = -1.0f;

uint16 step = 0;

const float fade_in_time = 0.5f;

float animate_start_time = 1.0f;
const float next_char_delay = 0.04f;
const float title_spin_duration = 0.5f;
const float total_title_anim_time = (14 - 1) * next_char_delay + title_spin_duration;

bool animate_title = true;
const int16 s16_one = 1;

void title_update()
{
    if (g_space_down)
    {
        animate_start_time = SDL_GetTicks64() / 1000.0f;
    }

    float time_s = SDL_GetTicks64() / 1000.0f;

    if (time_s < fade_in_time)
    {
        float a = 1.0f - (time_s / fade_in_time);
        sprite_set_color(fade_id, 0.0f, 0.0f, 0.0f, a);
    }
    else if (fade_id != 0)
    {
        sprite_delete(fade_id);
        fade_id = 0;
    }

    if (animate_title)
    {
        if (time_s > animate_start_time && time_s < total_title_anim_time)
        {
            float time_elapsed = time_s - animate_start_time;
            for (uint16 i = 0; i < 14; ++i)
            {
                float time_since_char_start = time_elapsed - i * next_char_delay;
                float a = SDL_clamp(time_since_char_start / title_spin_duration, 0.0f, 1.0f);
                float v = a < 0.5f ? 2.0f * a * a : 1.0f - (-2.0f * a + 2.0f) * (-2.0f * a + 2.0f) / 2.0f;
                float w = v * 2.0f - 1.0f;
                text_set_char_scale(title_id, i, w, 1.0f);
            }
        }
        else if (time_s >= total_title_anim_time)
        {
            for (uint16 i = 0; i < 14; ++i)
            {
                text_set_char_scale(title_id, i, 1.0f, 1.0f);
            }

            animate_title = false;
        }
    }
}

void title_cleanup()
{

}
