#include "main.h"

#include "render.h"
#include "sprite.h"

#include <SDL.h>
#include <stdlib.h>
#include <time.h>

Texture board_tex = Texture{ 112, 112, 0.5f, 0.625, 0.375f, 0.5f };
Texture pawn_white_tex = Texture  { 14, 14,  1.0f / 64.0f, 15.0f / 64.0f, 34.0f / 64.0f, 0.75f };
Texture pawn_black_tex = Texture  { 14, 14,  1.0f / 64.0f, 15.0f / 64.0f, 2.0f / 64.0f, 0.25f };
Texture rook_white_tex = Texture  { 14, 14, 17.0f / 64.0f, 31.0f / 64.0f, 34.0f / 64.0f, 0.75f };
Texture rook_black_tex = Texture  { 14, 14, 17.0f / 64.0f, 31.0f / 64.0f, 2.0f / 64.0f, 0.25f };
Texture knight_white_tex = Texture{ 14, 14, 33.0f / 64.0f, 47.0f / 64.0f, 34.0f / 64.0f, 0.75f };
Texture knight_black_tex = Texture{ 14, 14, 33.0f / 64.0f, 47.0f / 64.0f, 2.0f / 64.0f, 0.25f };
Texture bishop_white_tex = Texture{ 14, 14, 49.0f / 64.0f, 63.0f / 64.0f, 34.0f / 64.0f, 0.75f };
Texture bishop_black_tex = Texture{ 14, 14, 49.0f / 64.0f, 63.0f / 64.0f, 2.0f / 64.0f, 0.25f };
Texture queen_white_tex = Texture { 14, 14,  1.0f / 64.0f, 15.0f / 64.0f, 50.0f / 64.0f, 1.0f };
Texture queen_black_tex = Texture { 14, 14,  1.0f / 64.0f, 15.0f / 64.0f, 18.0f / 64.0f, 0.5f };
Texture king_white_tex = Texture  { 14, 14, 17.0f / 64.0f, 31.0f / 64.0f, 50.0f / 64.0f, 1.0f };
Texture king_black_tex = Texture  { 14, 14, 17.0f / 64.0f, 31.0f / 64.0f, 18.0f / 64.0f, 0.5f };

uint32 grid_sprites[8][8] = { 0 };

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

    SDL_Cursor* hoverCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

    short gridPosX = 160 - 112 - 8;
    short gridPosY = 160 - 112 - 8;
    sprite_create(&board_tex, gridPosX, gridPosY);

    bool quit = false;
    while (!quit)
    {
        Uint64 start = SDL_GetPerformanceCounter();

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        int posX = mouseX / 4;
        int posY = (640 - mouseY) / 4;

        bool mouseDown = false;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                mouseDown = true;
            }
        }

        short gridMousePosX = posX - gridPosX;
        short gridMousePosY = posY - gridPosY;
        if (gridMousePosX > 0 && gridMousePosX < 112 && gridMousePosY > 0 && gridMousePosY < 112)
        {
            SDL_SetCursor(hoverCursor);
            if (mouseDown)
            {
                short gridCol = gridMousePosX / 14;
                short gridRow = gridMousePosY / 14;
                short gridSquarePosX = gridCol * 14 + gridPosX;
                short gridSquarePosY = gridRow * 14 + gridPosY;

                if (grid_sprites[gridCol][gridRow] != 0)
                {
                    sprite_delete(grid_sprites[gridCol][gridRow]);
                }

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

                grid_sprites[gridCol][gridRow] = sprite_create(tex, gridSquarePosX, gridSquarePosY);
            }
        }
        else
        {
            SDL_SetCursor(SDL_GetDefaultCursor());
        }

        render_update();

        Uint64 end = SDL_GetPerformanceCounter();
        double elapsedMs = ((end - start) / (double)SDL_GetPerformanceFrequency()) * 1000.0f;
        SDL_Delay(floor(fmax((1000.0 / 60.0) - elapsedMs, 0.0)));
    }

    render_cleanup();

    SDL_FreeCursor(hoverCursor);

    SDL_Quit();

    return 0;
}
