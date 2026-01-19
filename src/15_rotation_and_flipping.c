#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <strings.h>
#include <time.h>

#include "assert.h"
#include "embed/SNES_F-Zero_Racers.png.h"
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
    struct sdl_texture snes_fzero_racers;
    SDL_Rect blue_falcon_clips[13];
    SDL_Rect golden_fox_clips[13];
    SDL_Rect wild_goose_clips[13];
    SDL_Rect fire_stingray_clips[13];
    SDL_Rect snail_clips[13];
};

struct car_state {
    int pos_x[5];
    int pos_y[5];
    int sprite_index[5];
    SDL_RendererFlip flip[5];
    double angle;
};

int init_SDL(struct sdl_system* system);
void close_SDL(struct sdl_system* system);

int load_texture_embedded(struct sdl_texture* texture, const void* img_data, const size_t size, SDL_Renderer* renderer,
                          const SDL_Color* color_key);
void free_texture(struct sdl_texture* texture);
int render_texture(const struct sdl_texture texture, SDL_Renderer* renderer, int x, int y, const SDL_Rect* clip,
                   const double angle, const SDL_Point* center, const SDL_RendererFlip flip);

int load_media(struct sdl_data* data, SDL_Renderer* renderer);
void free_media(struct sdl_data* data);

int get_car_heading_index(const double heading_angle, int* index, SDL_RendererFlip* flip);
int get_animation_state(const struct timespec* start_time, struct car_state* car_state);

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

    TRACE("Creating window");
    system->window = SDL_CreateWindow("SDL Tutorial 15 - Rotation and Flipping", SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    ASSERT(system->window != NULL, IMG_Quit(); SDL_Quit(); return -1;, "SDL_CreateWindow error=[%s]", SDL_GetError());

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

int load_media(struct sdl_data* data, SDL_Renderer* renderer) {
    int return_code = 0;
    SDL_Color color_key = {.r = 0x93, .g = 0xBB, .b = 0xEC, .a = 0};  // colorkey #93bbec

    ASSERT(data != NULL, return -1;, "Argument data must not be NULL");
    ASSERT(renderer != NULL, return -1;, "Argument renderer must not be NULL");

    TRACE("Loading texture SNES_F-Zero_Racers");
    return_code = load_texture_embedded(&(data->snes_fzero_racers), _embed_SNES_F_Zero_Racers_png_start,
                                        _embed_SNES_F_Zero_Racers_png_size, renderer, &color_key);
    ASSERT(return_code == 0, return -1;, "load_texture_embedded error");

    data->blue_falcon_clips[0] = (SDL_Rect){.x = 1, .y = 18, .w = 48, .h = 32};
    data->blue_falcon_clips[1] = (SDL_Rect){.x = 50, .y = 18, .w = 48, .h = 32};
    data->blue_falcon_clips[2] = (SDL_Rect){.x = 99, .y = 18, .w = 48, .h = 32};
    data->blue_falcon_clips[3] = (SDL_Rect){.x = 148, .y = 18, .w = 48, .h = 32};
    data->blue_falcon_clips[4] = (SDL_Rect){.x = 197, .y = 18, .w = 48, .h = 32};
    data->blue_falcon_clips[5] = (SDL_Rect){.x = 246, .y = 18, .w = 48, .h = 32};
    data->blue_falcon_clips[6] = (SDL_Rect){.x = 295, .y = 18, .w = 48, .h = 32};
    data->blue_falcon_clips[7] = (SDL_Rect){.x = 344, .y = 18, .w = 48, .h = 32};
    data->blue_falcon_clips[8] = (SDL_Rect){.x = 393, .y = 18, .w = 48, .h = 32};
    data->blue_falcon_clips[9] = (SDL_Rect){.x = 442, .y = 18, .w = 48, .h = 32};
    data->blue_falcon_clips[10] = (SDL_Rect){.x = 491, .y = 18, .w = 48, .h = 32};
    data->blue_falcon_clips[11] = (SDL_Rect){.x = 540, .y = 18, .w = 48, .h = 32};
    data->blue_falcon_clips[12] = (SDL_Rect){.x = 589, .y = 18, .w = 48, .h = 32};

    data->golden_fox_clips[0] = (SDL_Rect){.x = 1, .y = 324, .w = 48, .h = 32};
    data->golden_fox_clips[1] = (SDL_Rect){.x = 50, .y = 324, .w = 48, .h = 32};
    data->golden_fox_clips[2] = (SDL_Rect){.x = 99, .y = 324, .w = 48, .h = 32};
    data->golden_fox_clips[3] = (SDL_Rect){.x = 148, .y = 324, .w = 48, .h = 32};
    data->golden_fox_clips[4] = (SDL_Rect){.x = 197, .y = 324, .w = 48, .h = 32};
    data->golden_fox_clips[5] = (SDL_Rect){.x = 246, .y = 324, .w = 48, .h = 32};
    data->golden_fox_clips[6] = (SDL_Rect){.x = 295, .y = 324, .w = 48, .h = 32};
    data->golden_fox_clips[7] = (SDL_Rect){.x = 344, .y = 324, .w = 48, .h = 32};
    data->golden_fox_clips[8] = (SDL_Rect){.x = 393, .y = 324, .w = 48, .h = 32};
    data->golden_fox_clips[9] = (SDL_Rect){.x = 442, .y = 324, .w = 48, .h = 32};
    data->golden_fox_clips[10] = (SDL_Rect){.x = 491, .y = 324, .w = 48, .h = 32};
    data->golden_fox_clips[11] = (SDL_Rect){.x = 540, .y = 324, .w = 48, .h = 32};
    data->golden_fox_clips[12] = (SDL_Rect){.x = 589, .y = 324, .w = 48, .h = 32};

    data->wild_goose_clips[0] = (SDL_Rect){.x = 1, .y = 642, .w = 48, .h = 32};
    data->wild_goose_clips[1] = (SDL_Rect){.x = 50, .y = 642, .w = 48, .h = 32};
    data->wild_goose_clips[2] = (SDL_Rect){.x = 99, .y = 642, .w = 48, .h = 32};
    data->wild_goose_clips[3] = (SDL_Rect){.x = 148, .y = 642, .w = 48, .h = 32};
    data->wild_goose_clips[4] = (SDL_Rect){.x = 197, .y = 642, .w = 48, .h = 32};
    data->wild_goose_clips[5] = (SDL_Rect){.x = 246, .y = 642, .w = 48, .h = 32};
    data->wild_goose_clips[6] = (SDL_Rect){.x = 295, .y = 642, .w = 48, .h = 32};
    data->wild_goose_clips[7] = (SDL_Rect){.x = 344, .y = 642, .w = 48, .h = 32};
    data->wild_goose_clips[8] = (SDL_Rect){.x = 393, .y = 642, .w = 48, .h = 32};
    data->wild_goose_clips[9] = (SDL_Rect){.x = 442, .y = 642, .w = 48, .h = 32};
    data->wild_goose_clips[10] = (SDL_Rect){.x = 491, .y = 642, .w = 48, .h = 32};
    data->wild_goose_clips[11] = (SDL_Rect){.x = 540, .y = 642, .w = 48, .h = 32};
    data->wild_goose_clips[12] = (SDL_Rect){.x = 589, .y = 642, .w = 48, .h = 32};

    data->fire_stingray_clips[0] = (SDL_Rect){.x = 1, .y = 945, .w = 48, .h = 32};
    data->fire_stingray_clips[1] = (SDL_Rect){.x = 50, .y = 945, .w = 48, .h = 32};
    data->fire_stingray_clips[2] = (SDL_Rect){.x = 99, .y = 945, .w = 48, .h = 32};
    data->fire_stingray_clips[3] = (SDL_Rect){.x = 148, .y = 945, .w = 48, .h = 32};
    data->fire_stingray_clips[4] = (SDL_Rect){.x = 197, .y = 945, .w = 48, .h = 32};
    data->fire_stingray_clips[5] = (SDL_Rect){.x = 246, .y = 945, .w = 48, .h = 32};
    data->fire_stingray_clips[6] = (SDL_Rect){.x = 295, .y = 945, .w = 48, .h = 32};
    data->fire_stingray_clips[7] = (SDL_Rect){.x = 344, .y = 945, .w = 48, .h = 32};
    data->fire_stingray_clips[8] = (SDL_Rect){.x = 393, .y = 945, .w = 48, .h = 32};
    data->fire_stingray_clips[9] = (SDL_Rect){.x = 442, .y = 945, .w = 48, .h = 32};
    data->fire_stingray_clips[10] = (SDL_Rect){.x = 491, .y = 945, .w = 48, .h = 32};
    data->fire_stingray_clips[11] = (SDL_Rect){.x = 540, .y = 945, .w = 48, .h = 32};
    data->fire_stingray_clips[12] = (SDL_Rect){.x = 589, .y = 945, .w = 48, .h = 32};

    data->snail_clips[0] = (SDL_Rect){.x = 1, .y = 1247, .w = 48, .h = 32};
    data->snail_clips[1] = (SDL_Rect){.x = 50, .y = 1247, .w = 48, .h = 32};
    data->snail_clips[2] = (SDL_Rect){.x = 99, .y = 1247, .w = 48, .h = 32};
    data->snail_clips[3] = (SDL_Rect){.x = 148, .y = 1247, .w = 48, .h = 32};
    data->snail_clips[4] = (SDL_Rect){.x = 197, .y = 1247, .w = 48, .h = 32};
    data->snail_clips[5] = (SDL_Rect){.x = 246, .y = 1247, .w = 48, .h = 32};
    data->snail_clips[6] = (SDL_Rect){.x = 295, .y = 1247, .w = 48, .h = 32};
    data->snail_clips[7] = (SDL_Rect){.x = 344, .y = 1247, .w = 48, .h = 32};
    data->snail_clips[8] = (SDL_Rect){.x = 393, .y = 1247, .w = 48, .h = 32};
    data->snail_clips[9] = (SDL_Rect){.x = 442, .y = 1247, .w = 48, .h = 32};
    data->snail_clips[10] = (SDL_Rect){.x = 491, .y = 1247, .w = 48, .h = 32};
    data->snail_clips[11] = (SDL_Rect){.x = 540, .y = 1247, .w = 48, .h = 32};
    data->snail_clips[12] = (SDL_Rect){.x = 589, .y = 1247, .w = 48, .h = 32};

    return 0;
}

void free_media(struct sdl_data* data) {
    ASSERT(data != NULL, return;, "Argument data must not be NULL");

    TRACE("Destroying texture SNES_F-Zero_Racers");
    free_texture(&(data->snes_fzero_racers));

    memset(data->blue_falcon_clips, 0, sizeof(data->blue_falcon_clips));
    memset(data->golden_fox_clips, 0, sizeof(data->golden_fox_clips));
    memset(data->wild_goose_clips, 0, sizeof(data->wild_goose_clips));
    memset(data->fire_stingray_clips, 0, sizeof(data->fire_stingray_clips));
    memset(data->snail_clips, 0, sizeof(data->snail_clips));

    return;
}

int get_car_heading_index(const double heading_angle, int* index, SDL_RendererFlip* flip) {
    double angle;
    int index_offset;

    ASSERT(index != NULL, return -1;, "Argument index must not be NULL");
    ASSERT(flip != NULL, return -1;, "Argument flip must not be NULL");

    angle = fmod(heading_angle, 360.0);
    while (angle < 0.0) {
        angle = angle + 360.0;
    }

    if (angle < 90.0) {
        *flip = SDL_FLIP_HORIZONTAL;
        angle = 180.0 - angle;
    } else if (angle >= 270.0) {
        *flip = SDL_FLIP_HORIZONTAL;
        angle = 540.0 - angle;
    } else {
        *flip = SDL_FLIP_NONE;
    }

    // index 0 = 90
    // index 1 = 101.25
    // index 2 = 112.5
    // index 3 = 123.75
    // index 4 = 135
    // index 5 = 146.25
    // index 6 = 157.5
    // index 7 = 168.75
    // index 8 = 180
    // index 9 = 202.5
    // index 10 = 225
    // index 11 = 247.5
    // index 12 = 270

    if (angle < 180) {
        // 90 to 180 -> 11.25 degrees step
        const double step = 11.25;
        const double offset = 90.0;
        index_offset = (int)lround((angle - offset) / step);
    } else {
        // 180 to 270 -> 22.5 degrees step
        const double step = 22.5;
        const double offset = 180.0;
        index_offset = 8 + (int)lround((angle - offset) / step);
    }

    if (index_offset < 0) {
        index_offset = 0;
    }
    if (index_offset > 12) {
        index_offset = 12;
    }

    *index = index_offset;

    return 0;
}

int get_animation_state(const struct timespec* start_time, struct car_state* car_state) {
    int return_code = 0;
    int error_num = 0;

    const double duration = 6.0;
    const double radius = 160.0;
    const int center_x = 320;
    const int center_y = 240;
    const int sprite_offset_x = 24;
    const int sprite_offset_y = 16;

    struct timespec current_time;
    double seconds = 0;
    double t = 0;
    int counter = 0;

    ASSERT(start_time != NULL, return -1;, "Argument start_time must not be NULL");
    ASSERT(car_state != NULL, return -1;, "Argument car_state must not be NULL");

    // Get current time
    return_code = clock_gettime(CLOCK_MONOTONIC, &current_time);
    error_num = errno;
    ASSERT(return_code == 0, return -1;, "clock_gettime error=[%s]", strerror(error_num));

    seconds = (double)(current_time.tv_sec - start_time->tv_sec) +
              ((double)(current_time.tv_nsec - start_time->tv_nsec) / 1000000000.0);
    seconds = fmod(seconds, duration);
    t = seconds / duration;

    for (counter = 0; counter < 5; counter++) {
        double angle_offset = ((double)counter * 2.0 * M_PI) / 5.0;
        double heading_x;
        double heading_y;
        double heading_angle;

        // car position
        car_state->pos_x[counter] =
            (int)lround(radius * cos(t * 2.0 * M_PI + angle_offset) + center_x - sprite_offset_x);
        car_state->pos_y[counter] =
            (int)lround(radius * sin(t * 2.0 * M_PI + angle_offset) + center_y - sprite_offset_y);

        // car velocity heading (derivatives)
        heading_x = -sin(t * 2.0 * M_PI + angle_offset);
        heading_y = cos(t * 2.0 * M_PI + angle_offset);

        // heading angle (velocity direction)
        // math coordinates Y+ goes up, SDL coordinates Y+ goes down.
        heading_angle = atan2(-heading_y, heading_x) * (180.0 / M_PI);

        return_code =
            get_car_heading_index(heading_angle, &car_state->sprite_index[counter], &car_state->flip[counter]);
        ASSERT(return_code == 0, return -1;, "get_car_heading_index error");
    }
    car_state->angle = t * 360.0;

    return 0;
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
    int error_num = 0;
    bool quit = false;

    struct timespec start_time;
    struct car_state car_state = {0};
    const SDL_Point sprite_center = {.x = 24, .y = 16};

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

        return_code = get_animation_state(&start_time, &car_state);
        ASSERT(return_code == 0, return -1;, "get_animation_state error");

        // Render textures
        return_code = render_texture(data.snes_fzero_racers, system.renderer, car_state.pos_x[0], car_state.pos_y[0],
                                     &data.blue_falcon_clips[car_state.sprite_index[0]], 0.0, NULL, car_state.flip[0]);
        ASSERT(return_code == 0, return -1;, "render_texture error");

        return_code = render_texture(data.snes_fzero_racers, system.renderer, car_state.pos_x[1], car_state.pos_y[1],
                                     &data.golden_fox_clips[car_state.sprite_index[1]], 0.0, NULL, car_state.flip[1]);
        ASSERT(return_code == 0, return -1;, "render_texture error");

        return_code = render_texture(data.snes_fzero_racers, system.renderer, car_state.pos_x[2], car_state.pos_y[2],
                                     &data.wild_goose_clips[car_state.sprite_index[2]], 0.0, NULL, car_state.flip[2]);
        ASSERT(return_code == 0, return -1;, "render_texture error");

        return_code =
            render_texture(data.snes_fzero_racers, system.renderer, car_state.pos_x[3], car_state.pos_y[3],
                           &data.fire_stingray_clips[car_state.sprite_index[3]], 0.0, NULL, car_state.flip[3]);
        ASSERT(return_code == 0, return -1;, "render_texture error");

        return_code = render_texture(data.snes_fzero_racers, system.renderer, car_state.pos_x[4], car_state.pos_y[4],
                                     &data.snail_clips[car_state.sprite_index[4]], 0.0, NULL, car_state.flip[4]);
        ASSERT(return_code == 0, return -1;, "render_texture error");

        return_code = render_texture(data.snes_fzero_racers, system.renderer, 296, 224, &data.blue_falcon_clips[0],
                                     car_state.angle, &sprite_center, 0);
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
