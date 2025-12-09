#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "assert.h"
#include "embed/stretching_to_window.bmp.h"
#include "trace.h"

int init_SDL(SDL_Window** window, SDL_Surface** screen_surface);
SDL_Surface* load_bmp_embedded(const void* data, const size_t size, SDL_Surface* screen_surface);
int load_media(SDL_Surface** stretch_surface, SDL_Surface* screen_surface);
void close_SDL(SDL_Window** window, SDL_Surface** stretch_surface);
void main_loop(SDL_Window* window, SDL_Surface* screen_surface, SDL_Surface* stretch_surface);

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
    *window =
        SDL_CreateWindow("SDL Tutorial 05 - Optimized Surface Loading and Soft Stretching", SDL_WINDOWPOS_UNDEFINED,
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

SDL_Surface* load_bmp_embedded(const void* data, const size_t size, SDL_Surface* screen_surface) {
    SDL_RWops* rwops = NULL;
    SDL_Surface* surface = NULL;
    SDL_Surface* optimized_surface = NULL;

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
    if (!C_ASSERT(screen_surface != NULL)) {
        TRACE("Invalid parameter screen_surface");
        return NULL;
    }

    TRACE("Opening stream to embedded image data");
    rwops = SDL_RWFromConstMem(data, (int)size);
    if (!C_ASSERT(rwops != NULL)) {
        TRACE("SDL_RWFromConstMem() error=[%s]", SDL_GetError());
        return NULL;
    }

    TRACE("Loading surface");
    surface = SDL_LoadBMP_RW(rwops, 1);
    if (!C_ASSERT(surface != NULL)) {
        TRACE("SDL_LoadBMP_RW() error=[%s]", SDL_GetError());
        return NULL;
    }

    TRACE("Optimizing surface");
    optimized_surface = SDL_ConvertSurface(surface, screen_surface->format, 0);
    if (!C_ASSERT(optimized_surface != NULL)) {
        TRACE("SDL_ConvertSurface() error=[%s]", SDL_GetError());
        return NULL;
    }

    SDL_FreeSurface(surface);
    TRACE("Image width=[%d] height=[%d]", optimized_surface->w, optimized_surface->h);

    return optimized_surface;
}

int load_media(SDL_Surface** stretch_surface, SDL_Surface* screen_surface) {
    if (!C_ASSERT(stretch_surface != NULL)) {
        TRACE("Invalid parameter stretch_surface");
        return -1;
    }
    if (!C_ASSERT(screen_surface != NULL)) {
        TRACE("Invalid parameter screen_surface");
        return -1;
    }

    TRACE("Loading surface stretch_surface");
    if (!C_ASSERT(*stretch_surface == NULL)) {
        TRACE("Surface must be NULL before calling load_media");
        return -1;
    }
    *stretch_surface =
        load_bmp_embedded(_embed_stretching_to_window_bmp_start, _embed_stretching_to_window_bmp_size, screen_surface);
    if (!C_ASSERT(*stretch_surface != NULL)) {
        TRACE("load_bmp_embedded() error");
        return -1;
    }

    return 0;
}

void close_SDL(SDL_Window** window, SDL_Surface** stretch_surface) {
    if (!C_ASSERT(window != NULL)) {
        TRACE("Invalid parameter window");
        return;
    }
    if (!C_ASSERT(stretch_surface != NULL)) {
        TRACE("Invalid parameter stretch_surface");
        return;
    }

    if (*stretch_surface != NULL) {
        TRACE("Freeing surface stretch_surface");
        SDL_FreeSurface(*stretch_surface);
        *stretch_surface = NULL;
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

void main_loop(SDL_Window* window, SDL_Surface* screen_surface, SDL_Surface* stretch_surface) {
    int result = 0;

    SDL_Event eventBuffer;
    bool quit = false;
    SDL_Rect stretch_rect = {0, 0, 640, 480};
    struct timespec start_time, current_time;

    result = clock_gettime(CLOCK_MONOTONIC, &start_time);
    if (!C_ASSERT(result == 0)) {
        int error = errno;
        TRACE("clock_gettime() error=[%s]", strerror(error));
        return;
    }

    TRACE("Main loop start");
    while (quit == false) {
        double seconds = 0;

        // Calculate rect size based on time
        result = clock_gettime(CLOCK_MONOTONIC, &current_time);
        if (!C_ASSERT(result == 0)) {
            int error = errno;
            TRACE("clock_gettime() error=[%s]", strerror(error));
            return;
        }
        seconds = (double)(current_time.tv_sec - start_time.tv_sec) +
                  ((double)(current_time.tv_nsec - start_time.tv_nsec) / 1000000000.0);
        if (seconds < 1.0) {
            stretch_rect.x = 256;
            stretch_rect.y = 192;
            stretch_rect.w = 128;
            stretch_rect.h = 96;
        } else if (seconds < 6.0) {
            seconds = seconds - 1.0;
            stretch_rect.x = (int)lround(0.0 * (seconds / 5.0) + 256.0 * (1.0 - (seconds / 5.0)));
            stretch_rect.y = (int)lround(0.0 * (seconds / 5.0) + 192.0 * (1.0 - (seconds / 5.0)));
            stretch_rect.w = (int)lround(640.0 * (seconds / 5.0) + 128.0 * (1.0 - (seconds / 5.0)));
            stretch_rect.h = (int)lround(480.0 * (seconds / 5.0) + 96.0 * (1.0 - (seconds / 5.0)));
        } else if (seconds < 7.0) {
            stretch_rect.x = 0;
            stretch_rect.y = 0;
            stretch_rect.w = 640;
            stretch_rect.h = 480;
        } else if (seconds < 12.0) {
            seconds = seconds - 7.0;
            stretch_rect.x = (int)lround(256.0 * (seconds / 5.0) + 0.0 * (1.0 - (seconds / 5.0)));
            stretch_rect.y = (int)lround(192.0 * (seconds / 5.0) + 0.0 * (1.0 - (seconds / 5.0)));
            stretch_rect.w = (int)lround(128.0 * (seconds / 5.0) + 640.0 * (1.0 - (seconds / 5.0)));
            stretch_rect.h = (int)lround(96.0 * (seconds / 5.0) + 480.0 * (1.0 - (seconds / 5.0)));
        } else {
            result = clock_gettime(CLOCK_MONOTONIC, &start_time);
            if (!C_ASSERT(result == 0)) {
                int error = errno;
                TRACE("clock_gettime() error=[%s]", strerror(error));
                return;
            }
            continue;
        }

        // Fill the surface with color
        result = SDL_FillRect(screen_surface, NULL, SDL_MapRGB(screen_surface->format, 0x00, 0x80, 0x80));
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_FillRect() error=[%s]", SDL_GetError());
            return;
        }

        // Blit surface to window
        result = SDL_BlitScaled(stretch_surface, NULL, screen_surface, &stretch_rect);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_BlitScaled() error=[%s]", SDL_GetError());
            return;
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
    SDL_Surface* stretch_surface = NULL;

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
        close_SDL(&window, &stretch_surface);
        return -1;
    }

    TRACE("Loading media");
    result = load_media(&stretch_surface, screen_surface);
    if (!C_ASSERT(result >= 0)) {
        TRACE("load_media() error");
        close_SDL(&window, &stretch_surface);
        return -1;
    }

    main_loop(window, screen_surface, stretch_surface);

    TRACE("Closing");
    close_SDL(&window, &stretch_surface);

    TRACE("end");
    return 0;
}
