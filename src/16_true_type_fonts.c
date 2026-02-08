#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <strings.h>
#include <time.h>

#include "assert.h"
#include "embed/fonts/NotoSans-Regular.ttf.h"
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
    TTF_Font* notosans_regular;
    struct sdl_texture hello_world_texture;
};

int init_SDL(struct sdl_system* system);
void close_SDL(struct sdl_system* system);

int load_texture_embedded(struct sdl_texture* texture, const void* img_data, const size_t size, SDL_Renderer* renderer,
                          const SDL_Color* color_key);
void free_texture(struct sdl_texture* texture);
int render_texture(const struct sdl_texture texture, SDL_Renderer* renderer, int x, int y, const SDL_Rect* clip,
                   const double angle, const SDL_Point* center, const SDL_RendererFlip flip);

int load_font_embedded(TTF_Font** font, const int ptsize, const void* ttf_data, const size_t size);
void free_font(TTF_Font** font);

int create_text_texture(struct sdl_texture* texture, const char* text, TTF_Font* font, const SDL_Color color,
                        SDL_Renderer* renderer);

int load_media(struct sdl_data* data, SDL_Renderer* renderer);
void free_media(struct sdl_data* data);

int handle_events(bool* quit);
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

    TRACE("Initializing SDL_ttf");
    return_code = TTF_Init();
    ASSERT(return_code == 0, TTF_Quit(); IMG_Quit(); SDL_Quit(); return -1;, "TTF_Init error=[%s]", TTF_GetError());

    TRACE("Creating window");
    system->window = SDL_CreateWindow("SDL Tutorial 16 - True Type Fonts", SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    ASSERT(system->window != NULL, TTF_Quit(); IMG_Quit(); SDL_Quit(); return -1;
           , "SDL_CreateWindow error=[%s]", SDL_GetError());

    TRACE("Creating renderer for window");
    system->renderer = SDL_CreateRenderer(system->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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

    TRACE("Quitting SDL_ttf");
    TTF_Quit();

    TRACE("Quitting SDL_image");
    IMG_Quit();

    TRACE("Quitting SDL");
    SDL_Quit();
    return;
}

int load_texture_embedded(struct sdl_texture* texture, const void* img_data, const size_t size, SDL_Renderer* renderer,
                          const SDL_Color* color_key) {
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
    if (color_key != NULL) {
        TRACE("Setting surface color_key=[%02X%02X%02X]", color_key->r, color_key->g, color_key->b);
        return_code = SDL_SetColorKey(loaded_surface, SDL_TRUE,
                                      SDL_MapRGB(loaded_surface->format, color_key->r, color_key->g, color_key->b));
        ASSERT(return_code == 0, SDL_FreeSurface(loaded_surface); return -1;
               , "SDL_SetColorKey error=[%s]", SDL_GetError());
    }

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

int render_texture(const struct sdl_texture texture, SDL_Renderer* renderer, int x, int y, const SDL_Rect* clip,
                   const double angle, const SDL_Point* center, const SDL_RendererFlip flip) {
    int return_code = 0;
    SDL_Rect source_rect;
    SDL_Rect destination_rect;

    ASSERT(texture.texture != NULL, return -1;, "Argument texture.texture must not be NULL");
    ASSERT(renderer != NULL, return -1;, "Argument renderer must not be NULL");
    ASSERT(texture.width > 0, return -1;, "Argument texture.width must be greater than 0");
    ASSERT(texture.height > 0, return -1;, "Argument texture.height must be greater than 0");

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

    return_code = SDL_RenderCopyEx(renderer, texture.texture, &source_rect, &destination_rect, angle, center, flip);
    ASSERT(return_code == 0, return -1;, "SDL_RenderCopyEx error=[%s]", SDL_GetError());

    return 0;
}

int load_font_embedded(TTF_Font** font, const int ptsize, const void* ttf_data, const size_t size) {
    SDL_RWops* rwops = NULL;
    TTF_Font* loaded_font = NULL;
    // int return_code = 0;

    ASSERT(font != NULL, return -1;, "Argument font must not be NULL");
    ASSERT(ptsize > 0, return -1;, "Argument ptsize must be larger than 0");
    ASSERT(ttf_data != NULL, return -1;, "Argument ttf_data must not be NULL");
    ASSERT(size > 0, return -1;, "Argument size must be larger than 0");
    ASSERT(size <= INT_MAX, return -1;, "Argument size must not exceed maximum allowed");

    TRACE("Opening stream to embedded font data");
    rwops = SDL_RWFromConstMem(ttf_data, (int)size);
    ASSERT(rwops != NULL, return -1;, "SDL_RWFromConstMem error=[%s]", SDL_GetError());

    TRACE("Loading font");
    loaded_font = TTF_OpenFontRW(rwops, 1, ptsize);
    ASSERT(loaded_font != NULL, return -1;, "TTF_OpenFontRW error=[%s]", TTF_GetError());

    // Fill return font
    if (*font != NULL) {
        free_font(font);
    }
    *font = loaded_font;

    return 0;
}

void free_font(TTF_Font** font) {
    ASSERT(font != NULL, return;, "Argument font must not be NULL");

    if (*font != NULL) {
        TRACE("Destroying font");
        TTF_CloseFont(*font);
        *font = NULL;
    }

    return;
}

int create_text_texture(struct sdl_texture* texture, const char* text, TTF_Font* font, const SDL_Color color,
                        SDL_Renderer* renderer) {
    SDL_Surface* loaded_surface = NULL;
    SDL_Texture* loaded_texture = NULL;
    int width = 0;
    int height = 0;
    int return_code = 0;

    ASSERT(texture != NULL, return -1;, "Argument texture must not be NULL");
    ASSERT(text != NULL, return -1;, "Argument text must not be NULL");
    ASSERT(font != NULL, return -1;, "Argument font must not be NULL");
    ASSERT(renderer != NULL, return -1;, "Argument renderer must not be NULL");

    TRACE("Loading surface");
    loaded_surface = TTF_RenderText_Blended(font, text, color);
    ASSERT(loaded_surface != NULL, return -1;, "TTF_RenderText_Blended error=[%s]", TTF_GetError());

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

int load_media(struct sdl_data* data, SDL_Renderer* renderer) {
    int return_code = 0;

    ASSERT(data != NULL, return -1;, "Argument data must not be NULL");
    ASSERT(renderer != NULL, return -1;, "Argument renderer must not be NULL");

    TRACE("Loading font NotoSans-Regular");
    return_code = load_font_embedded(&(data->notosans_regular), 96, _embed_fonts_NotoSans_Regular_ttf_start,
                                     _embed_fonts_NotoSans_Regular_ttf_size);
    ASSERT(return_code == 0, return -1;, "load_font_embedded error");

    return_code = create_text_texture(&(data->hello_world_texture), "Hello, World!", data->notosans_regular,
                                      (SDL_Color){0x00, 0x00, 0x00, 0xFF}, renderer);
    ASSERT(return_code == 0, return -1;, "create_text_texture error");

    return 0;
}

void free_media(struct sdl_data* data) {
    ASSERT(data != NULL, return;, "Argument data must not be NULL");

    TRACE("Destroying texture hello_world_texture");
    free_texture(&(data->hello_world_texture));

    TRACE("Destroying font notosans_regular");
    free_font(&(data->notosans_regular));

    return;
}

int handle_events(bool* quit) {
    SDL_Event event_buffer;
    int return_code = 0;

    ASSERT(quit != NULL, return -1;, "Argument quit must not be NULL");

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
                break;
            }
            default: {
                break;
            }
        }
    } while (return_code == 1);

    return 0;
}

int main_loop(const struct sdl_system system, const struct sdl_data data) {
    int return_code = 0;
    bool quit = false;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    SDL_Rect draw_rect;

    ASSERT(system.renderer != NULL, return -1;, "Argument system.renderer must not be NULL");
    ASSERT(data.hello_world_texture.texture != NULL, return -1;
           , "Argument data.hello_world_texture.texture must not be NULL");
    ASSERT(data.hello_world_texture.width > 0, return -1;
           , "Argument data.hello_world_texture.width must be greater than 0");
    ASSERT(data.hello_world_texture.height > 0, return -1;
           , "Argument data.hello_world_texture.height must be greater than 0");

    draw_rect.x = (SCREEN_WIDTH / 2) - (data.hello_world_texture.width / 2);
    draw_rect.y = (SCREEN_HEIGHT / 2) - (data.hello_world_texture.height / 2);
    draw_rect.w = data.hello_world_texture.width;
    draw_rect.h = data.hello_world_texture.height;

    TRACE("Main loop start");
    while (quit == false) {
        // Set renderer color
        return_code = SDL_SetRenderDrawColor(system.renderer, 0x00, 0x80, 0x80, 0xFF);
        ASSERT(return_code == 0, return -1;, "SDL_SetRenderDrawColor error=[%s]", SDL_GetError());

        // Clear screen
        return_code = SDL_RenderClear(system.renderer);
        ASSERT(return_code == 0, return -1;, "SDL_RenderClear error=[%s]", SDL_GetError());

        // Render green rectangle
        return_code = SDL_SetRenderDrawColor(system.renderer, 0x00, 0xFF, 0x00, 0xFF);
        ASSERT(return_code == 0, return -1;, "SDL_SetRenderDrawColor error=[%s]", SDL_GetError());
        return_code = SDL_RenderDrawRect(system.renderer, &draw_rect);
        ASSERT(return_code == 0, return -1;, "SDL_RenderDrawRect error=[%s]", SDL_GetError());

        // Render textures
        return_code = render_texture(data.hello_world_texture, system.renderer, draw_rect.x, draw_rect.y, NULL, 0.0,
                                     NULL, SDL_FLIP_NONE);
        ASSERT(return_code == 0, return -1;, "render_texture error");

        // Update screen
        SDL_RenderPresent(system.renderer);

        // Poll for currently pending events
        return_code = handle_events(&quit);
        ASSERT(return_code == 0, return -1;, "handle_events error");

        // sleep
        // nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = (1000000000 / 60)}, NULL);
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
    ASSERT(return_code == 0, return -1;, "init_SDL error");

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
