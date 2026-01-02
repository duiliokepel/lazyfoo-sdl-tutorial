#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "assert.h"
#include "trace.h"

int main(int argc, char** argv) {
    int result = 0;
    SDL_Window* window = NULL;
    SDL_Surface* screen_surface = NULL;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    SDL_Event event_buffer;
    bool quit = false;

    TRACE("start");

    // Command line
    TRACE("argc=[%d]", argc);
    for (int i = 0; i < argc; i++) {
        TRACE("argv[%d]=[%s]", i, argv[i]);
    }

    TRACE("Initializing SDL");
    result = SDL_Init(SDL_INIT_VIDEO);
    ASSERT(result == 0, return -1;, "SDL_Init() error=[%s]", SDL_GetError());

    TRACE("Creating window");
    window = SDL_CreateWindow("SDL Tutorial 01 - Hello SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    ASSERT(window != NULL, return -1;, "SDL_CreateWindow() error=[%s]", SDL_GetError());

    TRACE("Getting window surface");
    screen_surface = SDL_GetWindowSurface(window);

    TRACE("Filling the surface with color");
    result = SDL_FillRect(screen_surface, NULL, SDL_MapRGB(screen_surface->format, 0x00, 0x80, 0x80));
    ASSERT(result == 0, return -1;, "SDL_FillRect() error=[%s]", SDL_GetError());

    TRACE("Main loop start");
    while (quit == false) {
        // Update the surface
        result = SDL_UpdateWindowSurface(window);
        ASSERT(result == 0, SDL_DestroyWindow(window); SDL_Quit(); return -1;
               , "SDL_UpdateWindowSurface() error=[%s]", SDL_GetError());

        // Poll for currently pending events
        do {
            result = SDL_PollEvent(&event_buffer);
            if (result == 1 && event_buffer.type == SDL_QUIT) {
                TRACE("Quit");
                quit = true;
            }
        } while (result == 1);

        // sleep
        nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = (1000000000 / 60)}, NULL);
    }

    TRACE("Destroying window");
    SDL_DestroyWindow(window);

    TRACE("Quitting SDL");
    SDL_Quit();

    TRACE("end");
    return 0;
}
