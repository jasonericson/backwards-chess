#include <SDL.h>
#include <SDL_opengl.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    SDL_Window* win = SDL_CreateWindow("GAME",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 640, SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(win);

    float r = 1.0f;
    float g = 0.0f;
    float b = 1.0f;

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
                r = (float)rand() / (float)(RAND_MAX);
                g = (float)rand() / (float)(RAND_MAX);
                b = (float)rand() / (float)(RAND_MAX);
            }
        }

        glViewport(0, 0, 640, 640);
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(win);
    }

    SDL_Quit();

    return 0;
}
