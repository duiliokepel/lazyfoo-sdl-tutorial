#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "assert.h"
#include "embed/press_x_to_close.bmp.h"
#include "trace.h"

int init_SDL(SDL_Window** window, SDL_Surface** screen_surface);
int load_media(SDL_Surface** press_x_to_close);
void close_SDL(SDL_Window** window, SDL_Surface** press_x_to_close);

int init_SDL(SDL_Window** window, SDL_Surface** screen_surface) {
    int result = 0;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;

    if (!C_ASSERT(window != NULL)) {
        TRACE("Invalid parameter window");
        return -1;
    }
    if (!C_ASSERT(screen_surface != NULL)) {
        TRACE("Invalid parameter screen_surface");
        return -1;
    }

    if (!C_ASSERT(*window == NULL)) {
        TRACE("window must be NULL before init");
        return -1;
    }
    if (!C_ASSERT(*screen_surface == NULL)) {
        TRACE("screen_surface must be NULL before init");
        return -1;
    }

    TRACE("Initializing SDL");
    result = SDL_Init(SDL_INIT_VIDEO);
    if (!C_ASSERT(result >= 0)) {
        TRACE("SDL_init_SDL() error=[%s]", SDL_GetError());
        return -1;
    }

    TRACE("Creating window");
    *window = SDL_CreateWindow("SDL Tutorial 03 - Event Driven Programming", SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!C_ASSERT(*window != NULL)) {
        TRACE("SDL_CreateWindow() error=[%s]", SDL_GetError());
        return -1;
    }

    TRACE("Getting window surface");
    *screen_surface = SDL_GetWindowSurface(*window);
    if (!C_ASSERT(*screen_surface != NULL)) {
        TRACE("SDL_GetWindowSurface() error=[%s]", SDL_GetError());
        return -1;
    }

    return 0;
}

int load_media(SDL_Surface** press_x_to_close) {
    int result = 0;
    SDL_RWops* image_RWops = NULL;

    if (!C_ASSERT(press_x_to_close != NULL)) {
        TRACE("Invalid parameter press_x_to_close");
        return -1;
    }

    if (!C_ASSERT(*press_x_to_close == NULL)) {
        TRACE("press_x_to_close must be NULL before load_media");
        return -1;
    }

    TRACE("Opening sream to embedded press_x_to_close.bmp");
    image_RWops = SDL_RWFromConstMem(_embed_press_x_to_close_bmp_start, (int)_embed_press_x_to_close_bmp_size);
    if (!C_ASSERT(image_RWops != NULL)) {
        TRACE("SDL_RWFromConstMem() error=[%s]", SDL_GetError());
        return -1;
    }

    TRACE("Loading surface press_x_to_close");
    *press_x_to_close = SDL_LoadBMP_RW(image_RWops, 0);
    if (!C_ASSERT(*press_x_to_close != NULL)) {
        TRACE("SDL_LoadBMP_RW() error=[%s]", SDL_GetError());
        return -1;
    }

    TRACE("Closing stream to embedded press_x_to_close.bmp");
    result = SDL_RWclose(image_RWops);
    if (!C_ASSERT(result == 0)) {
        TRACE("SDL_RWclose() error=[%s]", SDL_GetError());
        return -1;
    }

    TRACE("Image width=[%d] height=[%d]", (*press_x_to_close)->w, (*press_x_to_close)->h);
    return 0;
}

void close_SDL(SDL_Window** window, SDL_Surface** press_x_to_close) {
    if (!C_ASSERT(window != NULL)) {
        TRACE("Invalidd parameter window");
        return;
    }
    if (!C_ASSERT(press_x_to_close != NULL)) {
        TRACE("Invalidd parameter press_x_to_close");
        return;
    }

    if (*press_x_to_close != NULL) {
        TRACE("Freeing surface press_x_to_close");
        SDL_FreeSurface(*press_x_to_close);
        press_x_to_close = NULL;
    }
    if (*window != NULL) {
        TRACE("Detroying window");
        SDL_DestroyWindow(*window);
        window = NULL;
    }

    TRACE("Quitting SDL");
    SDL_Quit();
    return;
}

int main(int argc, char** argv) {
    int result = 0;
    SDL_Window* window = NULL;
    SDL_Surface* screen_surface = NULL;
    SDL_Surface* press_x_to_close = NULL;
    SDL_Event eventBuffer;
    bool quit = false;

    TRACE("start");

    // Command line
    TRACE("argc=[%d]", argc);
    for (int i = 0; i < argc; i++) {
        TRACE("argv[%d]=[%s]", i, argv[i]);
    }

    TRACE("Initializing");
    result = init_SDL(&window, &screen_surface);
    if (!C_ASSERT(result == 0)) {
        TRACE("init_SDL() error");
        close_SDL(&window, &press_x_to_close);
        return -1;
    }

    TRACE("Loading media");
    result = load_media(&press_x_to_close);
    if (!C_ASSERT(result >= 0)) {
        TRACE("load_media() error");
        close_SDL(&window, &press_x_to_close);
        return -1;
    }

    TRACE("Blitting surface to window");
    result = SDL_BlitSurface(press_x_to_close, NULL, screen_surface, NULL);
    if (!C_ASSERT(result == 0)) {
        TRACE("SDL_BlitSurface() error=[%s]", SDL_GetError());
        close_SDL(&window, &press_x_to_close);
        return -1;
    }

    TRACE("Main loop start");
    while (quit == false) {
        // Update the surface
        result = SDL_UpdateWindowSurface(window);
        if (result != 0) {
            TRACE("SDL_UpdateWindowSurface() error=[%s]", SDL_GetError());
            SDL_Quit();
            return -1;
        }

        // Poll for currently pending events
        do {
            result = SDL_PollEvent(&eventBuffer);
            if (result == 0) {
                break;
            }
            if (eventBuffer.type == SDL_QUIT) {
                TRACE("Quit");
                quit = true;
            }
        } while (result == 1);

        // sleep
        nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = (1000000000 / 60)}, NULL);
    }

    TRACE("Closing");
    close_SDL(&window, &press_x_to_close);

    TRACE("end");
    return 0;
}
