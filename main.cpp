#include <SDL.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    SDL_Window* win = SDL_CreateWindow("GAME",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 640, 0);

    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, 0);
    int r = 255;
    int g = 0;
    int b = 255;

    time_t t;
    srand((unsigned) time(&t));

    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                r = rand() % 256;
                g = rand() % 256;
                b = rand() % 256;
            }
        }
        
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_Quit();

    return 0;
}
