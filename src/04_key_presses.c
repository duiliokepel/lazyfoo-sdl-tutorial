#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "assert.h"
#include "embed/press_default.bmp.h"
#include "embed/press_down.bmp.h"
#include "embed/press_left.bmp.h"
#include "embed/press_right.bmp.h"
#include "embed/press_up.bmp.h"
#include "trace.h"

enum key_press_surfaces {
    KEY_PRESS_DEFAULT,
    KEY_PRESS_UP,
    KEY_PRESS_DOWN,
    KEY_PRESS_LEFT,
    KEY_PRESS_RIGHT,
    KEY_PRESS_TOTAL
};

int init_SDL(SDL_Window** window, SDL_Surface** screen_surface);
SDL_Surface* load_bmp_embedded(const void* data, const size_t size);
int load_media(SDL_Surface* key_press_surface[]);
void main_loop(SDL_Window* window, SDL_Surface* screen_surface, SDL_Surface* key_press_surface[]);
void close_SDL(SDL_Window** window, SDL_Surface* key_press_surface[]);

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
        TRACE("SDL_Init() error=[%s]", SDL_GetError());
        return -1;
    }

    TRACE("Creating window");
    *window = SDL_CreateWindow("SDL Tutorial 04 - Key Presses", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

SDL_Surface* load_bmp_embedded(const void* data, const size_t size) {
    SDL_RWops* rwops = NULL;
    SDL_Surface* return_surface = NULL;

    if (!C_ASSERT(data != NULL)) {
        TRACE("Invalid parameter data");
        return NULL;
    }
    if (!C_ASSERT(size > 0)) {
        TRACE("Invalid parameter size");
        return NULL;
    }
    if (!C_ASSERT(size <= INT_MAX)) {
        TRACE("Size exceeds maximum allowed");
        return NULL;
    }

    TRACE("Opening stream to embedded");
    rwops = SDL_RWFromConstMem(data, (int)size);
    if (!C_ASSERT(rwops != NULL)) {
        TRACE("SDL_RWFromConstMem() error=[%s]", SDL_GetError());
        return NULL;
    }

    TRACE("Loading surface");
    return_surface = SDL_LoadBMP_RW(rwops, 1);
    if (!C_ASSERT(return_surface != NULL)) {
        TRACE("SDL_LoadBMP_RW() error=[%s]", SDL_GetError());
        return NULL;
    }

    TRACE("Image width=[%d] height=[%d]", return_surface->w, return_surface->h);

    return return_surface;
}

int load_media(SDL_Surface* key_press_surface[]) {
    if (!C_ASSERT(key_press_surface != NULL)) {
        TRACE("Invalid parameter key_press_surface");
        return -1;
    }

    TRACE("Loading surface press_default");
    if (!C_ASSERT(key_press_surface[KEY_PRESS_DEFAULT] == NULL)) {
        TRACE("Surface must be NULL before calling load_media");
        return -1;
    }
    key_press_surface[KEY_PRESS_DEFAULT] =
        load_bmp_embedded(_embed_press_default_bmp_start, _embed_press_default_bmp_size);
    if (!C_ASSERT(key_press_surface[KEY_PRESS_DEFAULT] != NULL)) {
        TRACE("load_bmp_embedded() error");
        return -1;
    }

    TRACE("Loading surface press_up");
    if (!C_ASSERT(key_press_surface[KEY_PRESS_UP] == NULL)) {
        TRACE("Surface must be NULL before calling load_media");
        return -1;
    }
    key_press_surface[KEY_PRESS_UP] = load_bmp_embedded(_embed_press_up_bmp_start, _embed_press_up_bmp_size);
    if (!C_ASSERT(key_press_surface[KEY_PRESS_UP] != NULL)) {
        TRACE("load_bmp_embedded() error");
        return -1;
    }

    TRACE("Loading surface press_down");
    if (!C_ASSERT(key_press_surface[KEY_PRESS_DOWN] == NULL)) {
        TRACE("Surface must be NULL before calling load_media");
        return -1;
    }
    key_press_surface[KEY_PRESS_DOWN] = load_bmp_embedded(_embed_press_down_bmp_start, _embed_press_down_bmp_size);
    if (!C_ASSERT(key_press_surface[KEY_PRESS_DOWN] != NULL)) {
        TRACE("load_bmp_embedded() error");
        return -1;
    }

    TRACE("Loading surface press_left");
    if (!C_ASSERT(key_press_surface[KEY_PRESS_LEFT] == NULL)) {
        TRACE("Surface must be NULL before calling load_media");
        return -1;
    }
    key_press_surface[KEY_PRESS_LEFT] = load_bmp_embedded(_embed_press_left_bmp_start, _embed_press_left_bmp_size);
    if (!C_ASSERT(key_press_surface[KEY_PRESS_LEFT] != NULL)) {
        TRACE("load_bmp_embedded() error");
        return -1;
    }

    TRACE("Loading surface press_right");
    if (!C_ASSERT(key_press_surface[KEY_PRESS_RIGHT] == NULL)) {
        TRACE("Surface must be NULL before calling load_media");
        return -1;
    }
    key_press_surface[KEY_PRESS_RIGHT] = load_bmp_embedded(_embed_press_right_bmp_start, _embed_press_right_bmp_size);
    if (!C_ASSERT(key_press_surface[KEY_PRESS_RIGHT] != NULL)) {
        TRACE("load_bmp_embedded() error");
        return -1;
    }

    return 0;
}

void close_SDL(SDL_Window** window, SDL_Surface* key_press_surface[]) {
    int counter = 0;
    if (!C_ASSERT(window != NULL)) {
        TRACE("Invalid parameter window");
        return;
    }

    for (counter = 0; counter < KEY_PRESS_TOTAL; counter++) {
        if (key_press_surface[counter] != NULL) {
            TRACE("Freeing surface %d", counter);
            SDL_FreeSurface(key_press_surface[counter]);
            key_press_surface[counter] = NULL;
        }
    }

    if (*window != NULL) {
        TRACE("Destroying window");
        SDL_DestroyWindow(*window);
        *window = NULL;
    }

    TRACE("Quitting SDL");
    SDL_Quit();
    return;
}

void main_loop(SDL_Window* window, SDL_Surface* screen_surface, SDL_Surface* key_press_surface[]) {
    int result = 0;
    SDL_Event eventBuffer;
    bool quit = false;
    int last_surface = KEY_PRESS_TOTAL;
    int current_surface = KEY_PRESS_DEFAULT;

    TRACE("Main loop start");
    while (quit == false) {
        // Update current image shown on screen
        if (current_surface != last_surface) {
            TRACE("Blitting surface %d to window", current_surface);
            result = SDL_BlitSurface(key_press_surface[current_surface], NULL, screen_surface, NULL);
            if (!C_ASSERT(result == 0)) {
                TRACE("SDL_BlitSurface() error=[%s]", SDL_GetError());
                return;
            }
            last_surface = current_surface;
        }

        // Update the window surface
        result = SDL_UpdateWindowSurface(window);
        if (result != 0) {
            TRACE("SDL_UpdateWindowSurface() error=[%s]", SDL_GetError());
            return;
        }

        // Poll for currently pending events
        do {
            result = SDL_PollEvent(&eventBuffer);
            if (result == 0) {
                break;
            }
            switch (eventBuffer.type) {
                case SDL_KEYDOWN: {
                    switch (eventBuffer.key.keysym.sym) {
                        case SDLK_UP: {
                            current_surface = KEY_PRESS_UP;
                            break;
                        }

                        case SDLK_DOWN: {
                            current_surface = KEY_PRESS_DOWN;
                            break;
                        }

                        case SDLK_LEFT: {
                            current_surface = KEY_PRESS_LEFT;
                            break;
                        }

                        case SDLK_RIGHT: {
                            current_surface = KEY_PRESS_RIGHT;
                            break;
                        }

                        default: {
                            current_surface = KEY_PRESS_DEFAULT;
                            break;
                        }
                    }
                    break;
                }
                case SDL_KEYUP: {
                    current_surface = KEY_PRESS_DEFAULT;
                    break;
                }
                case SDL_QUIT: {
                    TRACE("Quit");
                    quit = true;
                    break;
                }
                default: {
                    break;
                }
            }
        } while (result == 1);

        // sleep
        nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = (1000000000 / 60)}, NULL);
    }
    return;
}

int main(int argc, char** argv) {
    int result = 0;
    SDL_Window* window = NULL;
    SDL_Surface* screen_surface = NULL;
    SDL_Surface* key_press_surface[KEY_PRESS_TOTAL];

    TRACE("start");

    memset(key_press_surface, 0, sizeof(key_press_surface));

    // Command line
    TRACE("argc=[%d]", argc);
    for (int i = 0; i < argc; i++) {
        TRACE("argv[%d]=[%s]", i, argv[i]);
    }

    TRACE("Initializing");
    result = init_SDL(&window, &screen_surface);
    if (!C_ASSERT(result == 0)) {
        TRACE("init_SDL() error");
        close_SDL(&window, key_press_surface);
        return -1;
    }

    TRACE("Loading media");
    result = load_media(key_press_surface);
    if (!C_ASSERT(result >= 0)) {
        TRACE("load_media() error");
        close_SDL(&window, key_press_surface);
        return -1;
    }

    main_loop(window, screen_surface, key_press_surface);

    TRACE("Closing");
    close_SDL(&window, key_press_surface);

    TRACE("end");
    return 0;
}
