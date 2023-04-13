#include <SDL.h>

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* win = SDL_CreateWindow("GAME",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 640, 0);

    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, 0);

    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);

    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);

    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                quit = true;
        }
    }

    SDL_Quit();

    return 0;
}
