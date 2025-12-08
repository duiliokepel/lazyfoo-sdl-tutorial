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
    SDL_Surface* screenSurface = NULL;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    SDL_Event eventBuffer;
    bool quit = false;

    TRACE("start");

    // Command line
    TRACE("argc=[%d]", argc);
    for (int i = 0; i < argc; i++) {
        TRACE("argv[%d]=[%s]", i, argv[i]);
    }

    TRACE("Initializing SDL");
    result = SDL_Init(SDL_INIT_VIDEO);
    if (!C_ASSERT(result >= 0)) {
        TRACE("SDL_Init() error=[%s]", SDL_GetError());
        return -1;
    }

    TRACE("Creating window");
    window = SDL_CreateWindow("SDL Tutorial 01 - Hello SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!C_ASSERT(window != NULL)) {
        TRACE("SDL_CreateWindow() error=[%s]", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    TRACE("Getting window surface");
    screenSurface = SDL_GetWindowSurface(window);

    TRACE("Filling the surface with color");
    result = SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x00, 0x80, 0x80));
    if (!C_ASSERT(result == 0)) {
        TRACE("SDL_FillRect() error=[%s]", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    TRACE("Main loop start");
    while (quit == false) {
        // Update the surface
        result = SDL_UpdateWindowSurface(window);
        if (result != 0) {
            TRACE("SDL_UpdateWindowSurface() error=[%s]", SDL_GetError());
            SDL_DestroyWindow(window);
            SDL_Quit();
            return -1;
        }

        // Poll for currently pending events
        do {
            result = SDL_PollEvent(&eventBuffer);
            if (result == 1 && eventBuffer.type == SDL_QUIT) {
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
