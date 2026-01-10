#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <limits.h>
#include <stdbool.h>
#include <strings.h>
#include <time.h>

#include "assert.h"
#include "embed/color_modulation.png.h"
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
    struct sdl_texture color_modulation;
};

int init_SDL(struct sdl_system* system);
void close_SDL(struct sdl_system* system);

int load_texture_embedded(struct sdl_texture* texture, const void* img_data, const size_t size, SDL_Renderer* renderer);
void free_texture(struct sdl_texture* texture);
int render_texture(const struct sdl_texture texture, SDL_Renderer* renderer, int x, int y, const SDL_Rect* clip);

int load_media(struct sdl_data* data, SDL_Renderer* renderer);
void free_media(struct sdl_data* data);

int main_loop(const struct sdl_system system, const struct sdl_data data);
int handle_events(bool* quit, uint8_t* red, uint8_t* green, uint8_t* blue);
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
    system->window = SDL_CreateWindow("SDL Tutorial 12 - Color Modulation", SDL_WINDOWPOS_UNDEFINED,
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

    TRACE("Loading texture color_modulation");
    return_code = load_texture_embedded(&(data->color_modulation), _embed_color_modulation_png_start,
                                        _embed_color_modulation_png_size, renderer);
    ASSERT(return_code == 0, return -1;, "load_texture_embedded error");

    return 0;
}

void free_media(struct sdl_data* data) {
    ASSERT(data != NULL, return;, "Argument data must not be NULL");

    TRACE("Destroying texture color_modulation");
    free_texture(&(data->color_modulation));

    return;
}

int handle_events(bool* quit, uint8_t* red, uint8_t* green, uint8_t* blue) {
    SDL_Event event_buffer;
    int return_code = 0;
    int red_buffer, green_buffer, blue_buffer;

    ASSERT(quit != NULL, return -1;, "Argument quit must not be NULL");
    ASSERT(red != NULL, return -1;, "Argument red must not be NULL");
    ASSERT(green != NULL, return -1;, "Argument green must not be NULL");
    ASSERT(blue != NULL, return -1;, "Argument blue must not be NULL");

    red_buffer = *red;
    green_buffer = *green;
    blue_buffer = *blue;

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
                    case SDLK_q: {
                        red_buffer += 32;
                        break;
                    }
                    case SDLK_a: {
                        red_buffer -= 32;
                        break;
                    }
                    case SDLK_w: {
                        green_buffer += 32;
                        break;
                    }
                    case SDLK_s: {
                        green_buffer -= 32;
                        break;
                    }
                    case SDLK_e: {
                        blue_buffer += 32;
                        break;
                    }
                    case SDLK_d: {
                        blue_buffer -= 32;
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

    if (red_buffer > 255) {
        red_buffer = 255;
    } else if (red_buffer < 0) {
        red_buffer = 0;
    }
    if (green_buffer > 255) {
        green_buffer = 255;
    } else if (green_buffer < 0) {
        green_buffer = 0;
    }
    if (blue_buffer > 255) {
        blue_buffer = 255;
    } else if (blue_buffer < 0) {
        blue_buffer = 0;
    }

    *red = (uint8_t)red_buffer;
    *green = (uint8_t)green_buffer;
    *blue = (uint8_t)blue_buffer;

    return 0;
}

int main_loop(const struct sdl_system system, const struct sdl_data data) {
    int return_code = 0;
    bool quit = false;
    uint8_t red = 255;
    uint8_t green = 255;
    uint8_t blue = 255;

    ASSERT(system.renderer != NULL, return -1;, "Argument system.renderer must not be NULL");

    // Set renderer color
    return_code = SDL_SetRenderDrawColor(system.renderer, 0x00, 0x80, 0x80, 0xFF);
    ASSERT(return_code == 0, return -1;, "SDL_SetRenderDrawColor error=[%s]", SDL_GetError());

    TRACE("Main loop start");
    while (quit == false) {
        // Clear screen
        return_code = SDL_RenderClear(system.renderer);
        ASSERT(return_code == 0, return -1;, "SDL_RenderClear error=[%s]", SDL_GetError());

        // Set color modulation
        return_code = SDL_SetTextureColorMod(data.color_modulation.texture, red, green, blue);
        ASSERT(return_code == 0, return -1;, "SDL_SetTextureColorMod error=[%s]", SDL_GetError());

        // Render texture
        return_code = render_texture(data.color_modulation, system.renderer, 0, 0, NULL);
        ASSERT(return_code == 0, return -1;, "render_texture error");

        // Update screen
        SDL_RenderPresent(system.renderer);

        // Poll for currently pending events
        return_code = handle_events(&quit, &red, &green, &blue);
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
