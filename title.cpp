#include "title.h"

#include "sprite.h"
#include "text.h"

Texture square_tex2 = Texture{ MapId_Pieces, 14, 14,  48, 49, 31, 32 };
Texture knight_tex = Texture{ MapId_Pieces, 14, 14, 33, 47,  2, 16 };

uint32 title_id;
uint32 top_bar_id;
uint32 bot_bar_id;
uint32 fade_id;
uint32 piece_id;
uint32 text_id;

const int16 piece_x_start = -22;
const int16 piece_x_end = 96;
const int16 piece_y = 66;

void title_init()
{
    TextSettings title_settings;
    title_settings.font = Font_Title;
    title_settings.line_spacing = 10;
    title_settings.kerning = 1;
    title_id = text_create("Backwards\nChess", 4, 102, Layer_Board, title_settings, -1.0f);

    top_bar_id = sprite_create(&square_tex2, 73, 150, Layer_HeldPiece, 12.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    bot_bar_id = sprite_create(&square_tex2, 73, -2, Layer_HeldPiece, 12.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    fade_id = sprite_create(&square_tex2, 70, 80, Layer_HeldPiece, 16.0f, 16.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    piece_id = sprite_create(&knight_tex, piece_x_start, piece_y, Layer_PlacedPiece, 2.0f, 2.0f, 25.0f);

    TextSettings text_settings;
    text_settings.font = Font_Default;
    text_settings.align = Align_Center;
    text_id = text_create("Click to Start", 80, 38, Layer_Board, text_settings, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

float title_char_w = 1.0f;
float title_spin_dir = -1.0f;

uint16 step = 0;

const float fade_start_time = 0.5f;
const float fade_in_time = 0.5f;

const float animate_start_time = 0.8f;
const float next_char_delay = 0.03f;
const float title_spin_duration = 0.4f;
const float total_title_anim_time = (14 - 1) * next_char_delay + title_spin_duration;

const float flash_start_time = animate_start_time + total_title_anim_time;
const float flash_up_time = 0.4f;
const float flash_down_time = 0.3f;

const float piece_start_time = animate_start_time + total_title_anim_time + 0.2f;
const float piece_move_time = 0.2f;

const float text_start_time = piece_start_time + piece_move_time;
const float text_flash_time = 0.7f;
bool text_on = false;
float text_timer = text_flash_time;

const int16 s16_one = 1;
float last_time_s = 0.0f;

void title_update()
{
    float time_s = SDL_GetTicks64() / 1000.0f;
    float delta_time_s = time_s - last_time_s;

    if (g_mouse_down)
        start_game();

    if (time_s > fade_start_time && time_s <= fade_start_time + fade_in_time)
    {
        float a = 1.0f - ((time_s - fade_start_time) / fade_in_time);
        sprite_set_color(fade_id, 0.0f, 0.0f, 0.0f, a);
    }

    if (time_s > animate_start_time && time_s <= animate_start_time + total_title_anim_time)
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
    else if (time_s > animate_start_time + total_title_anim_time)
    {
        for (uint16 i = 0; i < 14; ++i)
        {
            text_set_char_scale(title_id, i, 1.0f, 1.0f);
        }
    }

    if (time_s > flash_start_time && time_s <= flash_start_time + flash_up_time)
    {
        float a = (time_s - flash_start_time) / flash_up_time;
        sprite_set_color(fade_id, 1.0f, 1.0f, 1.0f, a);
    }
    else if (time_s > flash_start_time + flash_up_time && time_s <= flash_start_time + flash_up_time + flash_down_time)
    {
        float a = 1.0f - ((time_s - (flash_start_time + flash_up_time)) / flash_down_time);
        sprite_set_color(fade_id, 1.0f, 1.0f, 1.0f, a);
    }

    if (time_s > piece_start_time && time_s <= piece_start_time + piece_move_time)
    {
        const int16 start = piece_x_start;
        int16 x = ((time_s - piece_start_time) / piece_move_time) * (piece_x_end - piece_x_start) + piece_x_start;
        sprite_set_pos(piece_id, x, piece_y);
    }

    if (time_s > text_start_time)
    {
        text_timer -= delta_time_s;
        if (text_timer <= 0.0f)
        {
            text_on = !text_on;
            if (text_on)
                text_set_alpha(text_id, 1.0f);
            else
                text_set_alpha(text_id, 0.0f);

            text_timer = text_flash_time;
        }
    }

    last_time_s = time_s;
}

void title_cleanup()
{
    text_delete(title_id);
    sprite_delete(top_bar_id);
    sprite_delete(bot_bar_id);
    sprite_delete(fade_id);
    sprite_delete(piece_id);
    text_delete(text_id);
}
