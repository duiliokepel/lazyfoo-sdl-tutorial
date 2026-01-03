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

struct sdl_system {
    SDL_Window* window;
    SDL_Surface* screen_surface;
};

struct sdl_data {
    SDL_Surface* stretch_surface;
};

int init_SDL(struct sdl_system* system);
void close_SDL(struct sdl_system* system);

SDL_Surface* load_bmp_embedded(const void* bmp_data, const size_t size, SDL_Surface* screen_surface);
int load_media(struct sdl_data* data, SDL_Surface* screen_surface);
void free_media(struct sdl_data* data);

int main_loop(const struct sdl_system system, const struct sdl_data data);
int main(int argc, char** argv);

int init_SDL(struct sdl_system* system) {
    int return_code = 0;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;

    ASSERT(system != NULL, return -1;, "Argument system must not be NULL");
    ASSERT(system->window == NULL, return -1;, "Argument system->window must be NULL before initialization");
    ASSERT(system->screen_surface == NULL, return -1;
           , "Argument system->screen_surface must be NULL before initialization");

    TRACE("Initializing SDL");
    return_code = SDL_Init(SDL_INIT_VIDEO);
    ASSERT(return_code == 0, return -1;, "SDL_Init error=[%s]", SDL_GetError());

    TRACE("Creating window");
    system->window =
        SDL_CreateWindow("SDL Tutorial 05 - Optimized Surface Loading and Soft Stretching", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    ASSERT(system->window != NULL, SDL_Quit(); return -1;, "SDL_CreateWindow error=[%s]", SDL_GetError());

    TRACE("Getting window surface");
    system->screen_surface = SDL_GetWindowSurface(system->window);
    ASSERT(system->screen_surface != NULL, close_SDL(system); return -1;
           , "SDL_GetWindowSurface error=[%s]", SDL_GetError());

    return 0;
}

void close_SDL(struct sdl_system* system) {
    ASSERT(system != NULL, return;, "Argument system must not be NULL");

    if (system->screen_surface != NULL) {
        system->screen_surface = NULL;
    }

    if (system->window != NULL) {
        TRACE("Destroying window");
        SDL_DestroyWindow(system->window);
        system->window = NULL;
    }

    TRACE("Quitting SDL");
    SDL_Quit();
    return;
}

SDL_Surface* load_bmp_embedded(const void* bmp_data, const size_t size, SDL_Surface* screen_surface) {
    SDL_RWops* rwops = NULL;
    SDL_Surface* surface = NULL;
    SDL_Surface* optimized_surface = NULL;

    ASSERT(bmp_data != NULL, return NULL;, "Argument bmp_data must not be NULL");
    ASSERT(size > 0, return NULL;, "Argument size must be larger than 0");
    ASSERT(size <= INT_MAX, return NULL;, "Argument size must not exceed maximum allowed");
    ASSERT(screen_surface != NULL, return NULL;, "Argument screen_surface must not be NULL");

    TRACE("Opening stream to embedded");
    rwops = SDL_RWFromConstMem(bmp_data, (int)size);
    ASSERT(rwops != NULL, return NULL;, "SDL_RWFromConstMem error=[%s]", SDL_GetError());

    TRACE("Loading surface");
    surface = SDL_LoadBMP_RW(rwops, 1);
    ASSERT(surface != NULL, return NULL;, "SDL_LoadBMP_RW error=[%s]", SDL_GetError());

    TRACE("Optimizing surface");
    optimized_surface = SDL_ConvertSurface(surface, screen_surface->format, 0);
    ASSERT(optimized_surface != NULL, SDL_FreeSurface(surface); return NULL;
           , "SDL_ConvertSurface error=[%s]", SDL_GetError());

    SDL_FreeSurface(surface);
    TRACE("Image width=[%d] height=[%d]", optimized_surface->w, optimized_surface->h);

    return optimized_surface;
}

int load_media(struct sdl_data* data, SDL_Surface* screen_surface) {
    ASSERT(data != NULL, return -1;, "Argument data must not be NULL");
    ASSERT(screen_surface != NULL, return -1;, "Argument screen_surface must not be NULL");

    TRACE("Loading surface stretch_surface");
    ASSERT(data->stretch_surface == NULL, return -1;, "Surface must be NULL before calling load_media");
    data->stretch_surface =
        load_bmp_embedded(_embed_stretching_to_window_bmp_start, _embed_stretching_to_window_bmp_size, screen_surface);
    ASSERT(data->stretch_surface != NULL, return -1;, "load_bmp_embedded error");

    return 0;
}

void free_media(struct sdl_data* data) {
    ASSERT(data != NULL, return;, "Argument data must not be NULL");

    if (data->stretch_surface != NULL) {
        TRACE("Freeing surface stretch_surface");
        SDL_FreeSurface(data->stretch_surface);
        data->stretch_surface = NULL;
    }

    return;
}

int main_loop(const struct sdl_system system, const struct sdl_data data) {
    int return_code = 0;
    int error_num = 0;
    SDL_Event event_buffer;
    bool quit = false;
    SDL_Rect stretch_rect = {0, 0, 640, 480};
    struct timespec start_time, current_time;

    return_code = clock_gettime(CLOCK_MONOTONIC, &start_time);
    error_num = errno;
    ASSERT(return_code == 0, return -1;, "clock_gettime error=[%s]", strerror(error_num));

    TRACE("Main loop start");
    while (quit == false) {
        double seconds = 0;

        // Calculate rect size based on time
        return_code = clock_gettime(CLOCK_MONOTONIC, &current_time);
        error_num = errno;
        ASSERT(return_code == 0, return -1;, "clock_gettime error=[%s]", strerror(error_num));

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
            return_code = clock_gettime(CLOCK_MONOTONIC, &start_time);
            error_num = errno;
            ASSERT(return_code == 0, return -1;, "clock_gettime error=[%s]", strerror(error_num));
            continue;
        }

        // Fill the surface with color
        return_code =
            SDL_FillRect(system.screen_surface, NULL, SDL_MapRGB(system.screen_surface->format, 0x00, 0x80, 0x80));
        ASSERT(return_code == 0, return -1;, "SDL_FillRect error=[%s]", SDL_GetError());

        // Blit surface to window
        return_code = SDL_BlitScaled(data.stretch_surface, NULL, system.screen_surface, &stretch_rect);
        ASSERT(return_code == 0, return -1;, "SDL_BlitScaled error=[%s]", SDL_GetError());

        // Update the window surface
        return_code = SDL_UpdateWindowSurface(system.window);
        ASSERT(return_code == 0, return -1;, "SDL_UpdateWindowSurface error=[%s]", SDL_GetError());

        // Poll for currently pending events
        do {
            return_code = SDL_PollEvent(&event_buffer);
            if (return_code == 0) {
                break;
            }
            switch (event_buffer.type) {
                case SDL_QUIT: {
                    TRACE("Quit");
                    quit = true;
                    break;
                }
                default: {
                    break;
                }
            }
        } while (return_code == 1);

        // sleep
        nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = (1000000000 / 60)}, NULL);
    }
    return 0;
}

int main(int argc, char** argv) {
    int return_code = 0;
    struct sdl_system system = {0};
    struct sdl_data data = {0};

    TRACE("start");

    // Command line
    TRACE("argc=[%d]", argc);
    for (int i = 0; i < argc; i++) {
        TRACE("argv[%d]=[%s]", i, argv[i]);
    }

    TRACE("Initializing");
    return_code = init_SDL(&system);
    ASSERT(return_code == 0, close_SDL(&system); return -1;, "init_SDL error");

    TRACE("Loading media");
    return_code = load_media(&data, system.screen_surface);
    ASSERT(return_code == 0, free_media(&data); close_SDL(&system); return -1;, "load_media error");

    return_code = main_loop(system, data);
    ASSERT(return_code == 0, free_media(&data); close_SDL(&system); return -1;, "main_loop error");

    TRACE("Freeing media");
    free_media(&data);

    TRACE("Closing");
    close_SDL(&system);

    TRACE("end");
    return 0;
}
