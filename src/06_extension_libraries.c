#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "assert.h"
#include "embed/png_loaded.png.h"
#include "trace.h"

int init_SDL(SDL_Window** window, SDL_Surface** screen_surface);
SDL_Surface* load_img_embedded(const void* data, const size_t size, SDL_Surface* screen_surface);
int load_media(SDL_Surface** png_image, SDL_Surface* screen_surface);
void close_SDL(SDL_Window** window, SDL_Surface** png_image);
void main_loop(SDL_Window* window, SDL_Surface* screen_surface, SDL_Surface* png_image);

int init_SDL(SDL_Window** window, SDL_Surface** screen_surface) {
    int result = 0;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    const int img_flags = IMG_INIT_PNG;

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
    *window = SDL_CreateWindow("SDL Tutorial 06 - Extension Libraries and Loading Other Image Formats",
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
                               SDL_WINDOW_SHOWN);
    if (!C_ASSERT(*window != NULL)) {
        TRACE("SDL_CreateWindow() error=[%s]", SDL_GetError());
        return -1;
    }

    TRACE("Initializing SDL_image");
    result = IMG_Init(img_flags);
    if (!C_ASSERT((result & img_flags) == img_flags)) {
        TRACE("IMG_Init() error=[%s]", IMG_GetError());
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

SDL_Surface* load_img_embedded(const void* data, const size_t size, SDL_Surface* screen_surface) {
    int result = 0;
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
    surface = IMG_Load_RW(rwops, 1);
    if (!C_ASSERT(surface != NULL)) {
        TRACE("IMG_Load_RW() error=[%s]", IMG_GetError());
        return NULL;
    }

    TRACE("Optimizing surface");
    // optimized_surface = SDL_ConvertSurface(surface, screen_surface->format, 0);
    optimized_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    if (!C_ASSERT(optimized_surface != NULL)) {
        TRACE("SDL_ConvertSurface() error=[%s]", SDL_GetError());
        return NULL;
    }
    SDL_FreeSurface(surface);

    TRACE("Setting surface blend mode");
    result = SDL_SetSurfaceBlendMode(optimized_surface, SDL_BLENDMODE_BLEND);
    if (!C_ASSERT(result == 0)) {
        TRACE("SDL_SetSurfaceBlendMode() error=[%s]", SDL_GetError());
        return NULL;
    }

    TRACE("Image width=[%d] height=[%d]", optimized_surface->w, optimized_surface->h);

    return optimized_surface;
}

int load_media(SDL_Surface** png_image, SDL_Surface* screen_surface) {
    if (!C_ASSERT(png_image != NULL)) {
        TRACE("Invalid parameter png_image");
        return -1;
    }
    if (!C_ASSERT(screen_surface != NULL)) {
        TRACE("Invalid parameter screen_surface");
        return -1;
    }

    TRACE("Loading surface png_image");
    if (!C_ASSERT(*png_image == NULL)) {
        TRACE("Surface must be NULL before calling load_media");
        return -1;
    }
    *png_image = load_img_embedded(_embed_png_loaded_png_start, _embed_png_loaded_png_size, screen_surface);
    if (!C_ASSERT(*png_image != NULL)) {
        TRACE("load_img_embedded() error");
        return -1;
    }

    return 0;
}

void close_SDL(SDL_Window** window, SDL_Surface** png_image) {
    if (!C_ASSERT(window != NULL)) {
        TRACE("Invalid parameter window");
        return;
    }
    if (!C_ASSERT(png_image != NULL)) {
        TRACE("Invalid parameter png_image");
        return;
    }

    if (*png_image != NULL) {
        TRACE("Freeing surface png_image");
        SDL_FreeSurface(*png_image);
        *png_image = NULL;
    }

    if (*window != NULL) {
        TRACE("Destroying window");
        SDL_DestroyWindow(*window);
        *window = NULL;
    }

    TRACE("Quitting SDL_image");
    IMG_Quit();

    TRACE("Quitting SDL");
    SDL_Quit();
    return;
}

void main_loop(SDL_Window* window, SDL_Surface* screen_surface, SDL_Surface* png_image) {
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
        if (seconds < 0.5) {
            double t = (seconds / 0.5);
            stretch_rect.x = (int)lround(0.0 * t + 80.0 * (1.0 - t));
            stretch_rect.y = (int)lround(0.0 * t + 60.0 * (1.0 - t));
            stretch_rect.w = (int)lround(640.0 * t + 480.0 * (1.0 - t));
            stretch_rect.h = (int)lround(480.0 * t + 360.0 * (1.0 - t));
        } else if (seconds < 1.0) {
            double t = ((seconds - 0.5) / 0.5);
            stretch_rect.x = (int)lround(80.0 * t + 0.0 * (1.0 - t));
            stretch_rect.y = (int)lround(60.0 * t + 0.0 * (1.0 - t));
            stretch_rect.w = (int)lround(480.0 * t + 640.0 * (1.0 - t));
            stretch_rect.h = (int)lround(360.0 * t + 480.0 * (1.0 - t));
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
        result = SDL_BlitScaled(png_image, NULL, screen_surface, &stretch_rect);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_BlitScaled() error=[%s]", SDL_GetError());
            return;
        }

        // Update the window surface
        result = SDL_UpdateWindowSurface(window);
        if (!C_ASSERT(result == 0)) {
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
    SDL_Surface* png_image = NULL;

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
        close_SDL(&window, &png_image);
        return -1;
    }

    TRACE("Loading media");
    result = load_media(&png_image, screen_surface);
    if (!C_ASSERT(result >= 0)) {
        TRACE("load_media() error");
        close_SDL(&window, &png_image);
        return -1;
    }

    main_loop(window, screen_surface, png_image);

    TRACE("Closing");
    close_SDL(&window, &png_image);

    TRACE("end");
    return 0;
}
