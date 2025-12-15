#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "assert.h"
#include "embed/viewport.png.h"
#include "trace.h"

struct sdl_system {
    SDL_Window* window;
    SDL_Renderer* renderer;
};

struct sdl_data {
    SDL_Texture* display_texture;
    SDL_Rect top_left_viewport;
    SDL_Rect top_right_viewport;
    SDL_Rect bottom_viewport;
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
    system->window = SDL_CreateWindow("SDL Tutorial 09 - The Viewport", SDL_WINDOWPOS_UNDEFINED,
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
        SDL_RWclose(rwops);
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
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;

    if (!C_ASSERT(data != NULL)) {
        TRACE("Invalid parameter data");
        return -1;
    }
    if (!C_ASSERT(system.renderer != NULL)) {
        TRACE("Renderer must not be NULL in load_media");
        return -1;
    }

    TRACE("Loading texture viewport");
    if (!C_ASSERT(data->display_texture == NULL)) {
        TRACE("Texture must be NULL before calling load_media");
        return -1;
    }
    data->display_texture = load_texture_embedded(_embed_viewport_png_start, _embed_viewport_png_size, system.renderer);
    if (!C_ASSERT(data->display_texture != NULL)) {
        TRACE("load_texture_embedded() error");
        return -1;
    }

    // Top left corner viewport
    data->top_left_viewport.x = 0;
    data->top_left_viewport.y = 0;
    data->top_left_viewport.w = SCREEN_WIDTH / 2;
    data->top_left_viewport.h = SCREEN_HEIGHT / 2;

    // Top right viewport
    data->top_right_viewport.x = SCREEN_WIDTH / 2;
    data->top_right_viewport.y = 0;
    data->top_right_viewport.w = SCREEN_WIDTH / 2;
    data->top_right_viewport.h = SCREEN_HEIGHT / 2;

    // Bottom viewport
    data->bottom_viewport.x = 0;
    data->bottom_viewport.y = SCREEN_HEIGHT / 2;
    data->bottom_viewport.w = SCREEN_WIDTH;
    data->bottom_viewport.h = SCREEN_HEIGHT / 2;

    return 0;
}

void free_media(struct sdl_data* data) {
    if (!C_ASSERT(data != NULL)) {
        TRACE("Invalid parameter data");
        return;
    }

    if (data->display_texture != NULL) {
        TRACE("Destroying texture viewport");
        SDL_DestroyTexture(data->display_texture);
        data->display_texture = NULL;
    }
    return;
}

int main_loop(struct sdl_system system, struct sdl_data data) {
    int result = 0;
    SDL_Event eventBuffer;
    bool quit = false;

    if (!C_ASSERT(system.renderer != NULL)) {
        TRACE("Invalid renderer in main_loop");
        return -1;
    }
    if (!C_ASSERT(data.display_texture != NULL)) {
        TRACE("Invalid display_texture in main_loop");
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
        // Clear screen
        result = SDL_RenderClear(system.renderer);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_RenderClear() error=[%s]", SDL_GetError());
            return -1;
        }

        // Top left corner viewport
        result = SDL_RenderSetViewport(system.renderer, &data.top_left_viewport);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_RenderSetViewport() error=[%s]", SDL_GetError());
            return -1;
        }

        // Render texture to viewport
        result = SDL_RenderCopy(system.renderer, data.display_texture, NULL, NULL);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_RenderCopy() error=[%s]", SDL_GetError());
            return -1;
        }

        // Top right viewport
        result = SDL_RenderSetViewport(system.renderer, &data.top_right_viewport);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_RenderSetViewport() error=[%s]", SDL_GetError());
            return -1;
        }

        // Render texture to viewport
        result = SDL_RenderCopy(system.renderer, data.display_texture, NULL, NULL);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_RenderCopy() error=[%s]", SDL_GetError());
            return -1;
        }

        // Bottom viewport
        result = SDL_RenderSetViewport(system.renderer, &data.bottom_viewport);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_RenderSetViewport() error=[%s]", SDL_GetError());
            return -1;
        }

        // Render texture to viewport
        result = SDL_RenderCopy(system.renderer, data.display_texture, NULL, NULL);
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
