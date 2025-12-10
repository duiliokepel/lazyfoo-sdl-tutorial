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
#include "embed/rendering_texture.png.h"
#include "trace.h"

struct sdl_system {
    SDL_Window* window;
    SDL_Renderer* renderer;
};

struct sdl_data {
    SDL_Texture* display_texture;
};

int init_SDL(struct sdl_system* system);
void close_SDL(struct sdl_system* system);
SDL_Texture* load_texture_embedded(const void* data, const size_t size, SDL_Renderer* renderer);
int load_media(struct sdl_data* data, struct sdl_system system);
void free_media(struct sdl_data* data);
int main_loop(struct sdl_system system, struct sdl_data data);

int init_SDL(struct sdl_system* system) {
    int result = 0;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    const int img_flags = IMG_INIT_PNG;

    if (!C_ASSERT(system != NULL)) {
        TRACE("Invalid parameter system");
        return -1;
    }
    if (!C_ASSERT(system->window == NULL)) {
        TRACE("window must be NULL before init");
        return -1;
    }
    if (!C_ASSERT(system->renderer == NULL)) {
        TRACE("renderer must be NULL before init");
        return -1;
    }

    TRACE("Initializing SDL");
    result = SDL_Init(SDL_INIT_VIDEO);
    if (!C_ASSERT(result == 0)) {
        TRACE("SDL_Init() error=[%s]", SDL_GetError());
        return -1;
    }

    TRACE("Creating window");
    system->window = SDL_CreateWindow("SDL Tutorial 07 - Texture Loading and Rendering", SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!C_ASSERT(system->window != NULL)) {
        TRACE("SDL_CreateWindow() error=[%s]", SDL_GetError());
        return -1;
    }

    TRACE("Creating renderer for window");
    system->renderer = SDL_CreateRenderer(system->window, -1, SDL_RENDERER_ACCELERATED);
    if (!C_ASSERT(system->renderer != NULL)) {
        TRACE("SDL_CreateRenderer() error=[%s]", SDL_GetError());
        return -1;
    }

    TRACE("Initializing SDL_image");
    result = IMG_Init(img_flags);
    if (!C_ASSERT((result & img_flags) == img_flags)) {
        TRACE("IMG_Init() error=[%s]", IMG_GetError());
        return -1;
    }

    return 0;
}

void close_SDL(struct sdl_system* system) {
    if (!C_ASSERT(system != NULL)) {
        TRACE("Invalid parameter system");
        return;
    }

    if (system->renderer != NULL) {
        TRACE("Destroying renderer");
        SDL_DestroyRenderer(system->renderer);
        system->renderer = NULL;
    }

    if (system->window != NULL) {
        TRACE("Destroying window");
        SDL_DestroyWindow(system->window);
        system->window = NULL;
    }

    TRACE("Quitting SDL_image");
    IMG_Quit();

    TRACE("Quitting SDL");
    SDL_Quit();
    return;
}

SDL_Texture* load_texture_embedded(const void* data, const size_t size, SDL_Renderer* renderer) {
    SDL_RWops* rwops = NULL;
    SDL_Surface* surface = NULL;
    SDL_Texture* texture = NULL;
    int result = 0;

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
    if (!C_ASSERT(renderer != NULL)) {
        TRACE("Invalid parameter renderer");
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

    TRACE("Image width=[%d] height=[%d]", surface->w, surface->h);

    TRACE("Creating texture from surface");
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!C_ASSERT(texture != NULL)) {
        TRACE("SDL_CreateTextureFromSurface() error=[%s]", SDL_GetError());
        SDL_FreeSurface(surface);
        return NULL;
    }
    SDL_FreeSurface(surface);

    result = SDL_SetTextureScaleMode(texture, SDL_ScaleModeBest);
    if (!C_ASSERT(result == 0)) {
        TRACE("SDL_SetTextureScaleMode() error=[%s]", SDL_GetError());
        SDL_DestroyTexture(texture);
        return NULL;
    }

    result = SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    if (!C_ASSERT(result == 0)) {
        TRACE("SDL_SetTextureBlendMode() error=[%s]", SDL_GetError());
        SDL_DestroyTexture(texture);
        return NULL;
    }

    return texture;
}

int load_media(struct sdl_data* data, struct sdl_system system) {
    if (!C_ASSERT(data != NULL)) {
        TRACE("Invalid parameter data");
        return -1;
    }
    if (!C_ASSERT(system.renderer != NULL)) {
        TRACE("Renderer must not be NULL in load_media");
        return -1;
    }

    TRACE("Loading texture rendering_texture");
    if (!C_ASSERT(data->display_texture == NULL)) {
        TRACE("Texture must be NULL before calling load_media");
        return -1;
    }
    data->display_texture =
        load_texture_embedded(_embed_rendering_texture_png_start, _embed_rendering_texture_png_size, system.renderer);
    if (!C_ASSERT(data->display_texture != NULL)) {
        TRACE("load_texture_embedded() error");
        return -1;
    }

    return 0;
}

void free_media(struct sdl_data* data) {
    if (!C_ASSERT(data != NULL)) {
        TRACE("Invalid parameter data");
        return;
    }

    if (data->display_texture != NULL) {
        TRACE("Destroying texture rendering_texture");
        SDL_DestroyTexture(data->display_texture);
        data->display_texture = NULL;
    }
    return;
}

int main_loop(struct sdl_system system, struct sdl_data data) {
    int result = 0;
    SDL_Event eventBuffer;
    bool quit = false;
    SDL_Rect stretch_rect = {0, 0, 640, 480};
    struct timespec start_time, current_time;

    if (!C_ASSERT(system.renderer != NULL)) {
        TRACE("Invalid renderer in main_loop");
        return -1;
    }
    if (!C_ASSERT(data.display_texture != NULL)) {
        TRACE("Invalid display_texture in main_loop");
        return -1;
    }

    result = clock_gettime(CLOCK_MONOTONIC, &start_time);
    if (!C_ASSERT(result == 0)) {
        int error = errno;
        TRACE("clock_gettime() error=[%s]", strerror(error));
        return -1;
    }

    // Set renderer color
    result = SDL_SetRenderDrawColor(system.renderer, 0x00, 0x80, 0x80, 0xFF);
    if (!C_ASSERT(result == 0)) {
        TRACE("SDL_SetRenderDrawColor() error=[%s]", SDL_GetError());
        return -1;
    }

    TRACE("Main loop start");
    while (quit == false) {
        double seconds = 0;

        // Calculate rect size based on time
        result = clock_gettime(CLOCK_MONOTONIC, &current_time);
        if (!C_ASSERT(result == 0)) {
            int error = errno;
            TRACE("clock_gettime() error=[%s]", strerror(error));
            return -1;
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
                return -1;
            }
            continue;
        }

        // Clear screen
        result = SDL_RenderClear(system.renderer);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_RenderClear() error=[%s]", SDL_GetError());
            return -1;
        }

        // Render texture to screen
        result = SDL_RenderCopy(system.renderer, data.display_texture, NULL, &stretch_rect);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_RenderCopy() error=[%s]", SDL_GetError());
            return -1;
        }

        // Update screen
        SDL_RenderPresent(system.renderer);

        // Poll for currently pending events
        do {
            result = SDL_PollEvent(&eventBuffer);
            if (!C_ASSERT(result >= 0)) {
                TRACE("SDL_PollEvent() error=[%s]", SDL_GetError());
                return -1;
            }

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
    return 0;
}

int main(int argc, char** argv) {
    int result = 0;
    struct sdl_system system = {0};
    struct sdl_data data = {0};

    TRACE("Start");

    // Command line
    TRACE("argc=[%d]", argc);
    for (int i = 0; i < argc; i++) {
        TRACE("argv[%d]=[%s]", i, argv[i]);
    }

    TRACE("Initializing");
    result = init_SDL(&system);
    if (!C_ASSERT(result == 0)) {
        TRACE("init_SDL() error");
        close_SDL(&system);
        return -1;
    }

    TRACE("Loading media");
    result = load_media(&data, system);
    if (!C_ASSERT(result == 0)) {
        TRACE("load_media() error");
        free_media(&data);
        close_SDL(&system);
        return -1;
    }

    result = main_loop(system, data);
    if (!C_ASSERT(result == 0)) {
        TRACE("main_loop() error");
        free_media(&data);
        close_SDL(&system);
        return -1;
    }

    TRACE("Freeing media");
    free_media(&data);

    TRACE("Closing");
    close_SDL(&system);

    TRACE("End");
    return 0;
}
