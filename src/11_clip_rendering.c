#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <limits.h>
#include <stdbool.h>
#include <time.h>

#include "assert.h"
#include "embed/sprite_sheet.png.h"
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
    struct sdl_texture sprite_sheet;
    SDL_Rect sprite_clips[4];
};

int init_SDL(struct sdl_system* system);
void close_SDL(struct sdl_system* system);

int load_texture_embedded(struct sdl_texture* texture, const void* img_data,
                          const size_t size, SDL_Renderer* renderer);
void free_texture(struct sdl_texture* texture);
int render_texture(const struct sdl_texture texture, SDL_Renderer* renderer,
                   int x, int y, const SDL_Rect* clip);

int load_media(struct sdl_data* data, SDL_Renderer* renderer);
void free_media(struct sdl_data* data);

int main_loop(const struct sdl_system system, const struct sdl_data data);
int main(int argc, char** argv);

int init_SDL(struct sdl_system* system) {
    int return_code = 0;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    const int img_flags = IMG_INIT_PNG;

    ASSERT (system != NULL, "Argument system must not be NULL") { return -1; }
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
    system->window =
        SDL_CreateWindow("SDL Tutorial 11 - Clip Rendering and Sprite Sheets",
                         SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                         SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
    ASSERT (system != NULL, "Argument system must not be NULL") { return; }

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

int load_texture_embedded(struct sdl_texture* texture, const void* img_data,
                          const size_t size, SDL_Renderer* renderer) {
    SDL_RWops* rwops = NULL;
    SDL_Surface* loaded_surface = NULL;
    SDL_Texture* loaded_texture = NULL;
    int width = 0;
    int height = 0;
    int return_code = 0;

    ASSERT (texture != NULL, "Argument texture must not be NULL") { return -1; }
    ASSERT (img_data != NULL, "Argument img_data must not be NULL") {
        return -1;
    }
    ASSERT (size > 0, "Argument size must be larger than 0") { return -1; }
    ASSERT (size <= INT_MAX, "Argument size must not exceed maximum allowed") {
        return -1;
    }
    ASSERT (renderer != NULL, "Argument renderer must not be NULL") {
        return -1;
    }

    TRACE("Opening stream to embedded image data");
    rwops = SDL_RWFromConstMem(img_data, (int)size);
    ASSERT (rwops != NULL, "SDL_RWFromConstMem error=[%s]", SDL_GetError()) {
        return -1;
    }

    TRACE("Loading surface");
    loaded_surface = IMG_Load_RW(rwops, 1);
    ASSERT (loaded_surface != NULL, "IMG_Load_RW error=[%s]", IMG_GetError()) {
        return -1;
    }

    width = loaded_surface->w;
    height = loaded_surface->h;
    TRACE("Image width=[%d] height=[%d]", width, height);

    // Color key image with cyan
    TRACE("Setting surface color key");
    return_code =
        SDL_SetColorKey(loaded_surface, SDL_TRUE,
                        SDL_MapRGB(loaded_surface->format, 0, 0xFF, 0xFF));
    ASSERT (return_code == 0, "SDL_SetColorKey error=[%s]", SDL_GetError()) {
        SDL_FreeSurface(loaded_surface);
        return -1;
    }

    TRACE("Creating texture from surface");
    loaded_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
    ASSERT (loaded_texture != NULL, "SDL_CreateTextureFromSurface error=[%s]",
            SDL_GetError()) {
        SDL_FreeSurface(loaded_surface);
        return -1;
    }

    SDL_FreeSurface(loaded_surface);

    return_code = SDL_SetTextureScaleMode(loaded_texture, SDL_ScaleModeBest);
    ASSERT (return_code == 0, "SDL_SetTextureScaleMode error=[%s]",
            SDL_GetError()) {
        SDL_DestroyTexture(loaded_texture);
        return -1;
    }

    return_code = SDL_SetTextureBlendMode(loaded_texture, SDL_BLENDMODE_BLEND);
    ASSERT (return_code == 0, "SDL_SetTextureBlendMode error=[%s]",
            SDL_GetError()) {
        SDL_DestroyTexture(loaded_texture);
        return -1;
    }

    // Fill return texture
    if (texture->texture != NULL) { free_texture(texture); }
    texture->texture = loaded_texture;
    texture->width = width;
    texture->height = height;
    return 0;
}

void free_texture(struct sdl_texture* texture) {
    ASSERT (texture != NULL, "Argument texture must not be NULL") { return; }

    if (texture->texture != NULL) {
        TRACE("Destroying texture");
        SDL_DestroyTexture(texture->texture);
        texture->texture = NULL;
    }
    texture->width = 0;
    texture->height = 0;

    return;
}

int render_texture(const struct sdl_texture texture, SDL_Renderer* renderer,
                   int x, int y, const SDL_Rect* clip) {
    int return_code = 0;
    SDL_Rect source_rect;
    SDL_Rect destination_rect;

    ASSERT (texture.texture != NULL,
            "Argument texture.texture must not be NULL") {
        return -1;
    }
    ASSERT (renderer != NULL, "Argument renderer must not be NULL") {
        return -1;
    }

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

    return_code = SDL_RenderCopy(renderer, texture.texture, &source_rect,
                                 &destination_rect);
    ASSERT (return_code == 0, "SDL_RenderCopy error=[%s]", SDL_GetError()) {
        return -1;
    }

    return 0;
}

int load_media(struct sdl_data* data, SDL_Renderer* renderer) {
    int return_code = 0;
    ASSERT (data != NULL, "Argument data must not be NULL") { return -1; }
    ASSERT (renderer != NULL, "Argument renderer must not be NULL") {
        return -1;
    }

    TRACE("Loading texture sprite_sheet");
    return_code = load_texture_embedded(&(data->sprite_sheet),
                                        _embed_sprite_sheet_png_start,
                                        _embed_sprite_sheet_png_size, renderer);
    ASSERT (return_code == 0, "load_texture_embedded error") { return -1; }

    // Sprite 1
    data->sprite_clips[0].x = 0;
    data->sprite_clips[0].y = 0;
    data->sprite_clips[0].w = 100;
    data->sprite_clips[0].h = 100;

    // Sprite 2
    data->sprite_clips[1].x = 100;
    data->sprite_clips[1].y = 0;
    data->sprite_clips[1].w = 100;
    data->sprite_clips[1].h = 100;

    // Sprite 3
    data->sprite_clips[2].x = 0;
    data->sprite_clips[2].y = 100;
    data->sprite_clips[2].w = 100;
    data->sprite_clips[2].h = 100;

    // Sprite 4
    data->sprite_clips[3].x = 100;
    data->sprite_clips[3].y = 100;
    data->sprite_clips[3].w = 100;
    data->sprite_clips[3].h = 100;

    return 0;
}

void free_media(struct sdl_data* data) {
    ASSERT (data != NULL, "Argument data must not be NULL") { return; }

    TRACE("Destroying texture sprite_sheet");
    free_texture(&(data->sprite_sheet));
    memset(&(data->sprite_clips), 0, sizeof(data->sprite_clips));

    return;
}

int main_loop(const struct sdl_system system, const struct sdl_data data) {
    int return_code = 0;
    SDL_Event event_buffer;
    bool quit = false;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;

    ASSERT (system.renderer != NULL,
            "Argument system.renderer must not be NULL") {
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

        // Render sprite 1
        return_code = render_texture(data.sprite_sheet, system.renderer, 0, 0,
                                     &data.sprite_clips[0]);
        ASSERT (return_code == 0, "render_texture error") { return -1; }

        // Render sprite 2
        return_code =
            render_texture(data.sprite_sheet, system.renderer,
                           SCREEN_WIDTH - 100, 0, &data.sprite_clips[1]);
        ASSERT (return_code == 0, "render_texture error") { return -1; }

        // Render sprite 3
        return_code =
            render_texture(data.sprite_sheet, system.renderer, 0,
                           SCREEN_HEIGHT - 100, &data.sprite_clips[2]);
        ASSERT (return_code == 0, "render_texture error") { return -1; }

        // Render sprite 4
        return_code = render_texture(data.sprite_sheet, system.renderer,
                                     SCREEN_WIDTH - 100, SCREEN_HEIGHT - 100,
                                     &data.sprite_clips[3]);
        ASSERT (return_code == 0, "render_texture error") { return -1; }

        // Update screen
        SDL_RenderPresent(system.renderer);

        // Poll for currently pending events
        do {
            return_code = SDL_PollEvent(&event_buffer);
            ASSERT (return_code >= 0, "SDL_PollEvent error=[%s]",
                    SDL_GetError()) {
                return -1;
            }

            if (return_code == 0) { break; }
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
    for (int i = 0; i < argc; i++) { TRACE("argv[%d]=[%s]", i, argv[i]); }

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
