#include "main.h"

#include "game.h"
#include "render.h"
#include "sprite.h"
#include "text.h"

#include <SDL.h>
#include <stdlib.h>
#include <time.h>

bool g_mouse_down = false;
bool g_mouse_up = false;
bool g_mouse_long_up = false;
bool g_space_down = false;
int g_mouse_x, g_mouse_y, g_mouse_raw_x, g_mouse_raw_y;

SDL_Cursor* hand_cursor;

void cursor_set(bool hand)
{
    if (hand)
        SDL_SetCursor(hand_cursor);
    else
        SDL_SetCursor(SDL_GetDefaultCursor());
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    render_init();
    sprite_init();
    text_init();

    game_init();

    hand_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

    Uint32 last_mouse_down = 0;

    time_t t;
    srand((unsigned) time(&t));

    bool quit = false;
    while (!quit)
    {
        Uint64 start = SDL_GetPerformanceCounter();

        SDL_GetMouseState(&g_mouse_raw_x, &g_mouse_raw_y);
        g_mouse_raw_y = 640 - g_mouse_raw_y;
        g_mouse_x = g_mouse_raw_x / 4;
        g_mouse_y = g_mouse_raw_y / 4;

        g_mouse_down = false;
        g_mouse_up = false;
        g_mouse_long_up = false;
        g_space_down = false;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                g_mouse_down = true;
                last_mouse_down = SDL_GetTicks();
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                g_mouse_up = true;
                if (SDL_GetTicks() - last_mouse_down > 300)
                    g_mouse_long_up = true;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_SPACE)
                {
                    g_space_down = true;
                }
            }
        }

        game_update();
        render_update();

        Uint64 end = SDL_GetPerformanceCounter();
        double elapsed_ms = ((end - start) / (double)SDL_GetPerformanceFrequency()) * 1000.0f;
        SDL_Delay(floor(fmax((1000.0 / 60.0) - elapsed_ms, 0.0)));
    }

    game_cleanup();
    render_cleanup();

    SDL_FreeCursor(hand_cursor);

    SDL_Quit();

    return 0;
}
