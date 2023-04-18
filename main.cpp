#include "main.h"

#include "render.h"

#include <SDL.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    render_init();

    float r = 1.0f;
    float g = 0.0f;
    float b = 1.0f;

    time_t t;
    srand((unsigned) time(&t));

    int16 texWidth = 16;
    int16 texHeight = 16;

    float u1 = 0.0f;
    float u2 = 0.25f;
    float v1 = 0.0f;
    float v2 = 0.25f;

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
                if (v1 < .1f)
                {
                    v1 = 0.5f;
                    v2 = 0.75f;
                }
                else
                {
                    v1 = 0.0f;
                    v2 = 0.25f;
                }
            }
        }

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        int posX = mouseX / 4;
        int posY = (640 - mouseY) / 4;

        // top right
        vertices[0] = posX + texWidth;
        vertices[1] = posY;

        // bottom right
        vertices[2] = posX + texWidth;
        vertices[3] = posY + texHeight;

        // top left
        vertices[4] = posX;
        vertices[5] = posY;

        // bottom right
        vertices[6] = posX + texWidth;
        vertices[7] = posY + texHeight;

        // bottom left
        vertices[8] = posX;
        vertices[9] = posY + texHeight;

        // top left
        vertices[10] = posX;
        vertices[11] = posY;

        uvs[0] = u2;
        uvs[1] = v2;

        uvs[2] = u2;
        uvs[3] = v1;

        uvs[4] = u1;
        uvs[5] = v2;

        uvs[6] = u2;
        uvs[7] = v1;

        uvs[8] = u1;
        uvs[9] = v1;

        uvs[10] = u1;
        uvs[11] = v2;

        render_update();

        Uint64 end = SDL_GetPerformanceCounter();
        double elapsedMs = ((end - start) / (double)SDL_GetPerformanceFrequency()) * 1000.0f;
        SDL_Delay(floor(fmax((1000.0 / 60.0) - elapsedMs, 0.0)));
    }

    render_cleanup();

    SDL_Quit();

    return 0;
}
