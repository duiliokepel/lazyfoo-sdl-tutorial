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
    SDL_Texture* viewport_texture;
    SDL_Rect top_left_viewport;
    SDL_Rect top_right_viewport;
    SDL_Rect bottom_viewport;
};

int init_SDL(struct sdl_system* system);
void close_SDL(struct sdl_system* system);

SDL_Texture* load_texture_embedded(const void* img_data, const size_t size,
                                   SDL_Renderer* renderer);
int load_media(struct sdl_data* data, SDL_Renderer* renderer);
void free_media(struct sdl_data* data);

int main_loop(const struct sdl_system system, const struct sdl_data data);
int main(int argc, char** argv);

int init_SDL(struct sdl_system* system) {
    int return_code = 0;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    const int img_flags = IMG_INIT_PNG;

    ASSERT (system != NULL, "Argument system must not be NULL") {
        return -1;
    }
    ASSERT (system->window == NULL,
            "Argument system->window must be NULL before initialization") {
        return -1;
    }
    ASSERT (system->renderer == NULL,
            "Argument system->renderer must be NULL before initialization") {
        return -1;
    }

    TRACE("Initializing SDL");
    return_code = SDL_Init(SDL_INIT_VIDEO);
    ASSERT (return_code == 0, "SDL_Init error=[%s]", SDL_GetError()) {
        return -1;
    }

    TRACE("Initializing SDL_image");
    return_code = IMG_Init(img_flags);
    ASSERT ((return_code & img_flags) == img_flags, "IMG_Init error=[%s]",
            IMG_GetError()) {
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    TRACE("Creating window");
    system->window = SDL_CreateWindow(
        "SDL Tutorial 09 - The Viewport", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    ASSERT (system->window != NULL, "SDL_CreateWindow error=[%s]",
            SDL_GetError()) {
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    TRACE("Creating renderer for window");
    system->renderer =
        SDL_CreateRenderer(system->window, -1, SDL_RENDERER_ACCELERATED);
    ASSERT (system->renderer != NULL, "SDL_CreateRenderer error=[%s]",
            SDL_GetError()) {
        close_SDL(system);
        return -1;
    }

    return 0;
}

void close_SDL(struct sdl_system* system) {
    ASSERT (system != NULL, "Argument system must not be NULL") {
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

SDL_Texture* load_texture_embedded(const void* img_data, const size_t size,
                                   SDL_Renderer* renderer) {
    SDL_RWops* rwops = NULL;
    SDL_Surface* surface = NULL;
    SDL_Texture* texture = NULL;
    int return_code = 0;

    ASSERT (img_data != NULL, "Argument img_data must not be NULL") {
        return NULL;
    }
    ASSERT (size > 0, "Argument size must be larger than 0") {
        return NULL;
    }
    ASSERT (size <= INT_MAX, "Argument size must not exceed maximum allowed") {
        return NULL;
    }
    ASSERT (renderer != NULL, "Argument renderer must not be NULL") {
        return NULL;
    }

    TRACE("Opening stream to embedded image data");
    rwops = SDL_RWFromConstMem(img_data, (int)size);
    ASSERT (rwops != NULL, "SDL_RWFromConstMem error=[%s]", SDL_GetError()) {
        return NULL;
    }

    TRACE("Loading surface");
    surface = IMG_Load_RW(rwops, 1);
    ASSERT (surface != NULL, "IMG_Load_RW error=[%s]", IMG_GetError()) {
        return NULL;
    }

    TRACE("Image width=[%d] height=[%d]", surface->w, surface->h);

    TRACE("Creating texture from surface");
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    ASSERT (texture != NULL, "SDL_CreateTextureFromSurface error=[%s]",
            SDL_GetError()) {
        SDL_FreeSurface(surface);
        return NULL;
    }

    SDL_FreeSurface(surface);

    return_code = SDL_SetTextureScaleMode(texture, SDL_ScaleModeBest);
    ASSERT (return_code == 0, "SDL_SetTextureScaleMode error=[%s]",
            SDL_GetError()) {
        SDL_DestroyTexture(texture);
        return NULL;
    }

    return_code = SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    ASSERT (return_code == 0, "SDL_SetTextureBlendMode error=[%s]",
            SDL_GetError()) {
        SDL_DestroyTexture(texture);
        return NULL;
    }

    return texture;
}

int load_media(struct sdl_data* data, SDL_Renderer* renderer) {
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;

    ASSERT (data != NULL, "Argument data must not be NULL") {
        return -1;
    }
    ASSERT (renderer != NULL, "Argument renderer must not be NULL") {
        return -1;
    }

    TRACE("Loading texture viewport");
    ASSERT (data->viewport_texture == NULL,
            "Surface must be NULL before calling load_media") {
        return -1;
    }
    data->viewport_texture = load_texture_embedded(
        _embed_viewport_png_start, _embed_viewport_png_size, renderer);
    ASSERT (data->viewport_texture != NULL, "load_texture_embedded error") {
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
    ASSERT (data != NULL, "Argument data must not be NULL") {
        return;
    }

    if (data->viewport_texture != NULL) {
        TRACE("Destroying texture viewport");
        SDL_DestroyTexture(data->viewport_texture);
        data->viewport_texture = NULL;
    }
    return;
}

int main_loop(const struct sdl_system system, const struct sdl_data data) {
    int return_code = 0;
    SDL_Event event_buffer;
    bool quit = false;

    ASSERT (system.renderer != NULL,
            "Argument system.renderer must not be NULL") {
        return -1;
    }
    ASSERT (data.viewport_texture != NULL,
            "Argument data.viewport_texture must not be NULL") {
        return -1;
    }

    // Set renderer color
    return_code =
        SDL_SetRenderDrawColor(system.renderer, 0x00, 0x80, 0x80, 0xFF);
    ASSERT (return_code == 0, "SDL_SetRenderDrawColor error=[%s]",
            SDL_GetError()) {
        return -1;
    }

    TRACE("Main loop start");
    while (quit == false) {
        // Clear screen
        return_code = SDL_RenderClear(system.renderer);
        ASSERT (return_code == 0, "SDL_RenderClear error=[%s]",
                SDL_GetError()) {
            return -1;
        }

        // Top left corner viewport
        return_code =
            SDL_RenderSetViewport(system.renderer, &data.top_left_viewport);
        ASSERT (return_code == 0, "SDL_RenderSetViewport error=[%s]",
                SDL_GetError()) {
            return -1;
        }

        // Render texture to viewport
        return_code =
            SDL_RenderCopy(system.renderer, data.viewport_texture, NULL, NULL);
        ASSERT (return_code == 0, "SDL_RenderCopy error=[%s]", SDL_GetError()) {
            return -1;
        }

        // Top right viewport
        return_code =
            SDL_RenderSetViewport(system.renderer, &data.top_right_viewport);
        ASSERT (return_code == 0, "SDL_RenderSetViewport error=[%s]",
                SDL_GetError()) {
            return -1;
        }

        // Render texture to viewport
        return_code =
            SDL_RenderCopy(system.renderer, data.viewport_texture, NULL, NULL);
        ASSERT (return_code == 0, "SDL_RenderCopy error=[%s]", SDL_GetError()) {
            return -1;
        }

        // Bottom viewport
        return_code =
            SDL_RenderSetViewport(system.renderer, &data.bottom_viewport);
        ASSERT (return_code == 0, "SDL_RenderSetViewport error=[%s]",
                SDL_GetError()) {
            return -1;
        }

        // Render texture to viewport
        return_code =
            SDL_RenderCopy(system.renderer, data.viewport_texture, NULL, NULL);
        ASSERT (return_code == 0, "SDL_RenderCopy error=[%s]", SDL_GetError()) {
            return -1;
        }

        // Update screen
        SDL_RenderPresent(system.renderer);

        // Poll for currently pending events
        do {
            return_code = SDL_PollEvent(&event_buffer);
            ASSERT (return_code >= 0, "SDL_PollEvent error=[%s]",
                    SDL_GetError()) {
                return -1;
            }

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
        nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = (1000000000 / 60)},
                  NULL);
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
    ASSERT (return_code == 0, "init_SDL error") {
        close_SDL(&system);
        return -1;
    }

    TRACE("Loading media");
    return_code = load_media(&data, system.renderer);
    ASSERT (return_code == 0, "load_media error") {
        free_media(&data);
        close_SDL(&system);
        return -1;
    }

    return_code = main_loop(system, data);
    ASSERT (return_code == 0, "main_loop error") {
        free_media(&data);
        close_SDL(&system);
        return -1;
    }

    TRACE("Freeing media");
    free_media(&data);

    TRACE("Closing");
    close_SDL(&system);

    TRACE("end");
    return 0;
}
