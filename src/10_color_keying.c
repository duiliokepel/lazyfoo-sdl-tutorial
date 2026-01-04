#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "assert.h"
#include "embed/earth_background.png.h"
#include "embed/space_shuttle_colorkey.png.h"
#include "trace.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    struct sdl_texture background_texture;
    struct sdl_texture colorkey_texture;
};

int init_SDL(struct sdl_system* system);
void close_SDL(struct sdl_system* system);

int load_texture_embedded(struct sdl_texture* texture, const void* img_data, const size_t size, SDL_Renderer* renderer);
void free_texture(struct sdl_texture* texture);
int render_texture(const struct sdl_texture texture, SDL_Renderer* renderer, int x, int y);

int load_media(struct sdl_data* data, SDL_Renderer* renderer);
void free_media(struct sdl_data* data);

int get_animation_state(struct timespec* start_time, int* pos_x, int* pos_y);

int main_loop(const struct sdl_system system, const struct sdl_data data);
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
    system->window = SDL_CreateWindow("SDL Tutorial 10 - Color Keying", SDL_WINDOWPOS_UNDEFINED,
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

    // Color key image with cyan
    TRACE("Setting surface color key");
    return_code = SDL_SetColorKey(loaded_surface, SDL_TRUE, SDL_MapRGB(loaded_surface->format, 0, 0xFF, 0xFF));
    ASSERT(return_code == 0, SDL_FreeSurface(loaded_surface); return -1;, "SDL_SetColorKey error=[%s]", SDL_GetError());

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

int render_texture(const struct sdl_texture texture, SDL_Renderer* renderer, int x, int y) {
    int return_code = 0;
    SDL_Rect destination_rect;

    ASSERT(texture.texture != NULL, return -1;, "Argument texture.texture must not be NULL");
    ASSERT(renderer != NULL, return -1;, "Argument renderer must not be NULL");

    destination_rect = (SDL_Rect){x, y, texture.width, texture.height};

    return_code = SDL_RenderCopy(renderer, texture.texture, NULL, &destination_rect);
    ASSERT(return_code == 0, return -1;, "SDL_RenderCopy error=[%s]", SDL_GetError());

    return 0;
}

int load_media(struct sdl_data* data, SDL_Renderer* renderer) {
    int return_code = 0;
    ASSERT(data != NULL, return -1;, "Argument data must not be NULL");
    ASSERT(renderer != NULL, return -1;, "Argument renderer must not be NULL");

    TRACE("Loading texture background_texture");
    return_code = load_texture_embedded(&(data->background_texture), _embed_earth_background_png_start,
                                        _embed_earth_background_png_size, renderer);
    ASSERT(return_code == 0, return -1;, "load_texture_embedded error");

    TRACE("Loading texture colorkey_texture");
    return_code = load_texture_embedded(&(data->colorkey_texture), _embed_space_shuttle_colorkey_png_start,
                                        _embed_space_shuttle_colorkey_png_size, renderer);
    ASSERT(return_code == 0, return -1;, "load_texture_embedded error");

    return 0;
}

void free_media(struct sdl_data* data) {
    ASSERT(data != NULL, return;, "Argument data must not be NULL");

    TRACE("Destroying texture background_texture");
    free_texture(&(data->background_texture));

    TRACE("Destroying texture colorkey_texture");
    free_texture(&(data->colorkey_texture));
    return;
}

int get_animation_state(struct timespec* start_time, int* pos_x, int* pos_y) {
    int return_code = 0;
    int error_num = 0;
    struct timespec current_time;
    double seconds = 0;
    double t = 0;

    const double duration = 40.0;
    const int x0 = 30;
    const int y0 = 160;
    const int x1 = 330;
    const int y1 = 100;

    ASSERT(start_time != NULL, return -1;, "Argument start_time must not be NULL");
    ASSERT(pos_x != NULL, return -1;, "Argument pos_x must not be NULL");
    ASSERT(pos_y != NULL, return -1;, "Argument pos_y must not be NULL");

    // Get current time
    return_code = clock_gettime(CLOCK_MONOTONIC, &current_time);
    error_num = errno;
    ASSERT(return_code == 0, return -1;, "clock_gettime error=[%s]", strerror(error_num));

    seconds = (double)(current_time.tv_sec - start_time->tv_sec) +
              ((double)(current_time.tv_nsec - start_time->tv_nsec) / 1000000000.0);
    seconds = fmod(seconds, duration);

    t = (seconds / duration) * 2 * M_PI;
    *pos_x = (int)lround(((double)x1 - (double)x0) * (-(cos(t) - 1.0) / 2.0) + ((double)x0));
    *pos_y = (int)lround(((double)y1 - (double)y0) * (-(cos(t) - 1.0) / 2.0) + ((double)y0));

    return 0;
}

int main_loop(const struct sdl_system system, const struct sdl_data data) {
    int return_code = 0;
    SDL_Event event_buffer;
    bool quit = false;
    int pos_x = 0;
    int pos_y = 0;
    struct timespec start_time;
    int error_num = 0;

    ASSERT(system.renderer != NULL, return -1;, "Argument system.renderer must not be NULL");

    return_code = clock_gettime(CLOCK_MONOTONIC, &start_time);
    error_num = errno;
    ASSERT(return_code == 0, return -1;, "clock_gettime error=[%s]", strerror(error_num));

    // Set renderer color
    return_code = SDL_SetRenderDrawColor(system.renderer, 0x00, 0x80, 0x80, 0xFF);
    ASSERT(return_code == 0, return -1;, "SDL_SetRenderDrawColor error=[%s]", SDL_GetError());

    TRACE("Main loop start");
    while (quit == false) {
        // Clear screen
        return_code = SDL_RenderClear(system.renderer);
        ASSERT(return_code == 0, return -1;, "SDL_RenderClear error=[%s]", SDL_GetError());

        // Render background_texture to screen
        return_code = render_texture(data.background_texture, system.renderer, 0, 0);
        ASSERT(return_code == 0, return -1;, "render_texture error");

        return_code = get_animation_state(&start_time, &pos_x, &pos_y);
        ASSERT(return_code == 0, return -1;, "get_animation_state error");

        // Render colorkey_texture to screen
        return_code = render_texture(data.colorkey_texture, system.renderer, pos_x, pos_y);
        ASSERT(return_code == 0, return -1;, "render_texture error");

        // Update screen
        SDL_RenderPresent(system.renderer);

        // Poll for currently pending events
        do {
            return_code = SDL_PollEvent(&event_buffer);
            ASSERT(return_code >= 0, return -1;, "SDL_PollEvent error=[%s]", SDL_GetError());

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
