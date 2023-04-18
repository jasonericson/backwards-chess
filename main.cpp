#include "main.h"

#include "render.h"
#include "sprite.h"

#include <SDL.h>
#include <stdlib.h>
#include <time.h>

Texture pawn_white_tex = Texture{ 0.0f, 0.25f, 0.5f, 0.75f };
Texture pawn_black_tex = Texture{ 0.0f, 0.25f, 0.0f, 0.25f };
Texture rook_white_tex = Texture{ 0.25f, 0.5f, 0.5f, 0.75f };
Texture rook_black_tex = Texture{ 0.25f, 0.5f, 0.0f, 0.25f };
Texture knight_white_tex = Texture{ 0.5f, 0.75f, 0.5f, 0.75f };
Texture knight_black_tex = Texture{ 0.5f, 0.75f, 0.0f, 0.25f };
Texture bishop_white_tex = Texture{ 0.75f, 1.0f, 0.5f, 0.75f };
Texture bishop_black_tex = Texture{ 0.75f, 1.0f, 0.0f, 0.25f };
Texture queen_white_tex = Texture{ 0.0f, 0.25f, 0.75f, 1.0f };
Texture queen_black_tex = Texture{ 0.0f, 0.25f, 0.25f, 0.5f };
Texture king_white_tex = Texture{ 0.25f, 0.5f, 0.75f, 1.0f };
Texture king_black_tex = Texture{ 0.25f, 0.5f, 0.25f, 0.5f };

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    render_init();
    sprite_init();

    float r = 1.0f;
    float g = 0.0f;
    float b = 1.0f;

    time_t t;
    srand((unsigned) time(&t));

    int16 texWidth = 16;
    int16 texHeight = 16;

    bool white = true;
    Texture curr_tex = king_white_tex;

    bool quit = false;
    while (!quit)
    {
        Uint64 start = SDL_GetPerformanceCounter();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                int posX = mouseX / 4;
                int posY = (640 - mouseY) / 4;

                Texture* tex;
                int rand_index = rand() % 12;
                switch (rand_index)
                {
                case 0:
                    tex = &pawn_white_tex;
                    break;
                case 1:
                    tex = &pawn_black_tex;
                    break;
                case 2:
                    tex = &rook_white_tex;
                    break;
                case 3:
                    tex = &rook_black_tex;
                    break;
                case 4:
                    tex = &knight_white_tex;
                    break;
                case 5:
                    tex = &knight_black_tex;
                    break;
                case 6:
                    tex = &bishop_white_tex;
                    break;
                case 7:
                    tex = &bishop_black_tex;
                    break;
                case 8:
                    tex = &queen_white_tex;
                    break;
                case 9:
                    tex = &queen_black_tex;
                    break;
                case 10:
                    tex = &king_white_tex;
                    break;
                default:
                    tex = &king_black_tex;
                    break;
                }

                sprite_create(tex, posX, posY);
            }
        }

        render_update();

        Uint64 end = SDL_GetPerformanceCounter();
        double elapsedMs = ((end - start) / (double)SDL_GetPerformanceFrequency()) * 1000.0f;
        SDL_Delay(floor(fmax((1000.0 / 60.0) - elapsedMs, 0.0)));
    }

    render_cleanup();

    SDL_Quit();

    return 0;
}
