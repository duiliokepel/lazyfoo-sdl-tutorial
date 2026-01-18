#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "assert.h"
#include "embed/blending_press_s.png.h"
#include "embed/blending_press_w.png.h"
#include "trace.h"

struct sdl_texture {
    SDL_Texture* texture;
    int width;
    int height;
};

struct sdl_system {
    SDL_Window* window;
    SDL_Renderer* renderer;
};

struct sdl_data {
    struct sdl_texture blending_press_w;
    struct sdl_texture blending_press_s;
};

int init_SDL(struct sdl_system* system);
void close_SDL(struct sdl_system* system);

int load_texture_embedded(struct sdl_texture* texture, const void* img_data, const size_t size, SDL_Renderer* renderer);
void free_texture(struct sdl_texture* texture);
int render_texture(const struct sdl_texture texture, SDL_Renderer* renderer, int x, int y, const SDL_Rect* clip);

int load_media(struct sdl_data* data, SDL_Renderer* renderer);
void free_media(struct sdl_data* data);

int main_loop(const struct sdl_system system, const struct sdl_data data);
int handle_events(bool* quit, uint8_t* alpha);
int main(int argc, char** argv);

int init_SDL(struct sdl_system* system) {
    int return_code = 0;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    const int img_flags = IMG_INIT_PNG;

    ASSERT(system != NULL, return -1;, "Argument system must not be NULL");
    ASSERT(system->window == NULL, return -1;, "Argument system->window must be NULL before initialization");
    ASSERT(system->renderer == NULL, return -1;, "Argument system->renderer must be NULL before initialization");

    TRACE("Initializing SDL");
    return_code = SDL_Init(SDL_INIT_VIDEO);
    ASSERT(return_code == 0, return -1;, "SDL_Init error=[%s]", SDL_GetError());

    TRACE("Initializing SDL_image");
    return_code = IMG_Init(img_flags);
    ASSERT((return_code & img_flags) == img_flags, IMG_Quit(); SDL_Quit(); return -1;
           , "IMG_Init error=[%s]", IMG_GetError());

    TRACE("Creating window");
    system->window = SDL_CreateWindow("SDL Tutorial 13 - Alpha Blending", SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    ASSERT(system->window != NULL, IMG_Quit(); SDL_Quit(); return -1;, "SDL_CreateWindow error=[%s]", SDL_GetError());

    TRACE("Creating renderer for window");
    system->renderer = SDL_CreateRenderer(system->window, -1, SDL_RENDERER_ACCELERATED);
    ASSERT(system->renderer != NULL, close_SDL(system); return -1;, "SDL_CreateRenderer error=[%s]", SDL_GetError());

    return 0;
}

void close_SDL(struct sdl_system* system) {
    ASSERT(system != NULL, return;, "Argument system must not be NULL");

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

int load_texture_embedded(struct sdl_texture* texture, const void* img_data, const size_t size,
                          SDL_Renderer* renderer) {
    SDL_RWops* rwops = NULL;
    SDL_Surface* loaded_surface = NULL;
    SDL_Texture* loaded_texture = NULL;
    int width = 0;
    int height = 0;
    int return_code = 0;

    ASSERT(texture != NULL, return -1;, "Argument texture must not be NULL");
    ASSERT(img_data != NULL, return -1;, "Argument img_data must not be NULL");
    ASSERT(size > 0, return -1;, "Argument size must be larger than 0");
    ASSERT(size <= INT_MAX, return -1;, "Argument size must not exceed maximum allowed");
    ASSERT(renderer != NULL, return -1;, "Argument renderer must not be NULL");

    TRACE("Opening stream to embedded image data");
    rwops = SDL_RWFromConstMem(img_data, (int)size);
    ASSERT(rwops != NULL, return -1;, "SDL_RWFromConstMem error=[%s]", SDL_GetError());

    TRACE("Loading surface");
    loaded_surface = IMG_Load_RW(rwops, 1);
    ASSERT(loaded_surface != NULL, return -1;, "IMG_Load_RW error=[%s]", IMG_GetError());

    width = loaded_surface->w;
    height = loaded_surface->h;
    TRACE("Image width=[%d] height=[%d]", width, height);

    TRACE("Creating texture from surface");
    loaded_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
    ASSERT(loaded_texture != NULL, SDL_FreeSurface(loaded_surface); return -1;
           , "SDL_CreateTextureFromSurface error=[%s]", SDL_GetError());

    SDL_FreeSurface(loaded_surface);

    return_code = SDL_SetTextureScaleMode(loaded_texture, SDL_ScaleModeBest);
    ASSERT(return_code == 0, SDL_DestroyTexture(loaded_texture); return -1;
           , "SDL_SetTextureScaleMode error=[%s]", SDL_GetError());

    return_code = SDL_SetTextureBlendMode(loaded_texture, SDL_BLENDMODE_BLEND);
    ASSERT(return_code == 0, SDL_DestroyTexture(loaded_texture); return -1;
           , "SDL_SetTextureBlendMode error=[%s]", SDL_GetError());

    // Fill return texture
    if (texture->texture != NULL) {
        free_texture(texture);
    }
    texture->texture = loaded_texture;
    texture->width = width;
    texture->height = height;
    return 0;
}

void free_texture(struct sdl_texture* texture) {
    ASSERT(texture != NULL, return;, "Argument texture must not be NULL");

    if (texture->texture != NULL) {
        TRACE("Destroying texture");
        SDL_DestroyTexture(texture->texture);
        texture->texture = NULL;
    }
    texture->width = 0;
    texture->height = 0;

    return;
}

int render_texture(const struct sdl_texture texture, SDL_Renderer* renderer, int x, int y, const SDL_Rect* clip) {
    int return_code = 0;
    SDL_Rect source_rect;
    SDL_Rect destination_rect;

    ASSERT(texture.texture != NULL, return -1;, "Argument texture.texture must not be NULL");
    ASSERT(renderer != NULL, return -1;, "Argument renderer must not be NULL");

    source_rect = (SDL_Rect){0, 0, texture.width, texture.height};
    destination_rect = (SDL_Rect){x, y, texture.width, texture.height};

    if (clip != NULL) {
        source_rect.x = clip->x;
        source_rect.y = clip->y;
        source_rect.w = clip->w;
        source_rect.h = clip->h;
        destination_rect.w = clip->w;
        destination_rect.h = clip->h;
    }

    return_code = SDL_RenderCopy(renderer, texture.texture, &source_rect, &destination_rect);
    ASSERT(return_code == 0, return -1;, "SDL_RenderCopy error=[%s]", SDL_GetError());

    return 0;
}

int load_media(struct sdl_data* data, SDL_Renderer* renderer) {
    int return_code = 0;
    ASSERT(data != NULL, return -1;, "Argument data must not be NULL");
    ASSERT(renderer != NULL, return -1;, "Argument renderer must not be NULL");

    TRACE("Loading texture blending_press_w");
    return_code = load_texture_embedded(&(data->blending_press_w), _embed_blending_press_w_png_start,
                                        _embed_blending_press_w_png_size, renderer);
    ASSERT(return_code == 0, return -1;, "load_texture_embedded error");

    TRACE("Loading texture blending_press_s");
    return_code = load_texture_embedded(&(data->blending_press_s), _embed_blending_press_s_png_start,
                                        _embed_blending_press_s_png_size, renderer);
    ASSERT(return_code == 0, return -1;, "load_texture_embedded error");

    return 0;
}

void free_media(struct sdl_data* data) {
    ASSERT(data != NULL, return;, "Argument data must not be NULL");

    TRACE("Destroying texture blending_press_w");
    free_texture(&(data->blending_press_w));

    TRACE("Destroying texture blending_press_s");
    free_texture(&(data->blending_press_s));

    return;
}

int handle_events(bool* quit, uint8_t* alpha) {
    SDL_Event event_buffer;
    int return_code = 0;
    int alpha_buffer;

    ASSERT(quit != NULL, return -1;, "Argument quit must not be NULL");
    ASSERT(alpha != NULL, return -1;, "Argument alpha must not be NULL");

    alpha_buffer = *alpha;

    do {
        return_code = SDL_PollEvent(&event_buffer);
        ASSERT(return_code >= 0, return -1;, "SDL_PollEvent error=[%s]", SDL_GetError());

        if (return_code == 0) {
            break;
        }
        switch (event_buffer.type) {
            case SDL_QUIT: {
                TRACE("Quit");
                *quit = true;
                break;
            }
            case SDL_KEYDOWN: {
                switch (event_buffer.key.keysym.sym) {
                    case SDLK_w: {
                        alpha_buffer += 32;
                        break;
                    }
                    case SDLK_s: {
                        alpha_buffer -= 32;
                        break;
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            default: {
                break;
            }
        }
    } while (return_code == 1);

    if (alpha_buffer > 255) {
        alpha_buffer = 255;
    } else if (alpha_buffer < 0) {
        alpha_buffer = 0;
    }

    *alpha = (uint8_t)alpha_buffer;

    return 0;
}

int main_loop(const struct sdl_system system, const struct sdl_data data) {
    int return_code = 0;
    bool quit = false;
    uint8_t alpha = 255;

    ASSERT(system.renderer != NULL, return -1;, "Argument system.renderer must not be NULL");

    // Set renderer color
    return_code = SDL_SetRenderDrawColor(system.renderer, 0x00, 0x80, 0x80, 0xFF);
    ASSERT(return_code == 0, return -1;, "SDL_SetRenderDrawColor error=[%s]", SDL_GetError());

    TRACE("Main loop start");
    while (quit == false) {
        // Clear screen
        return_code = SDL_RenderClear(system.renderer);
        ASSERT(return_code == 0, return -1;, "SDL_RenderClear error=[%s]", SDL_GetError());

        // Set texture alpha modulation
        return_code = SDL_SetTextureAlphaMod(data.blending_press_s.texture, alpha);
        ASSERT(return_code == 0, return -1;, "SDL_SetTextureAlphaMod error=[%s]", SDL_GetError());

        // Render textures
        return_code = render_texture(data.blending_press_w, system.renderer, 0, 0, NULL);
        ASSERT(return_code == 0, return -1;, "render_texture error");

        return_code = render_texture(data.blending_press_s, system.renderer, 0, 0, NULL);
        ASSERT(return_code == 0, return -1;, "render_texture error");

        // Update screen
        SDL_RenderPresent(system.renderer);

        // Poll for currently pending events
        return_code = handle_events(&quit, &alpha);
        ASSERT(return_code == 0, return -1;, "handle_events error");

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
    return_code = load_media(&data, system.renderer);
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
