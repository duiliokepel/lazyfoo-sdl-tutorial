#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#include "assert.h"
#include "embed/png_loaded.png.h"
#include "trace.h"

struct sdl_system {
    SDL_Window* window;
    SDL_Surface* screen_surface;
};

struct sdl_data {
    SDL_Surface* png_image;
};

int init_SDL(struct sdl_system* system);
void close_SDL(struct sdl_system* system);

SDL_Surface* load_img_embedded(const void* img_data, const size_t size);
int load_media(struct sdl_data* data);
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
    ASSERT (system->screen_surface == NULL,
            "Argument system->screen_surface must be NULL before "
            "initialization") {
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
        "SDL Tutorial 06 - Extension Libraries and Loading Other Image Formats",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
        SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    ASSERT (system->window != NULL, "SDL_CreateWindow error=[%s]",
            SDL_GetError()) {
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    TRACE("Getting window surface");
    system->screen_surface = SDL_GetWindowSurface(system->window);
    ASSERT (system->screen_surface != NULL, "SDL_GetWindowSurface error=[%s]",
            SDL_GetError()) {
        close_SDL(system);
        return -1;
    }

    return 0;
}

void close_SDL(struct sdl_system* system) {
    ASSERT (system != NULL, "Argument system must not be NULL") { return; }

    if (system->screen_surface != NULL) { system->screen_surface = NULL; }

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

SDL_Surface* load_img_embedded(const void* img_data, const size_t size) {
    int return_code = 0;
    SDL_RWops* rwops = NULL;
    SDL_Surface* surface = NULL;
    SDL_Surface* optimized_surface = NULL;

    ASSERT (img_data != NULL, "Argument img_data must not be NULL") {
        return NULL;
    }
    ASSERT (size > 0, "Argument size must be larger than 0") { return NULL; }
    ASSERT (size <= INT_MAX, "Argument size must not exceed maximum allowed") {
        return NULL;
    }

    TRACE("Opening stream to embedded");
    rwops = SDL_RWFromConstMem(img_data, (int)size);
    ASSERT (rwops != NULL, "SDL_RWFromConstMem error=[%s]", SDL_GetError()) {
        return NULL;
    }

    TRACE("Loading surface");
    surface = IMG_Load_RW(rwops, 1);
    ASSERT (surface != NULL, "IMG_Load_RW error=[%s]", IMG_GetError()) {
        return NULL;
    }

    TRACE("Optimizing surface");
    optimized_surface =
        SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    ASSERT (optimized_surface != NULL, "SDL_ConvertSurfaceFormat error=[%s]",
            SDL_GetError()) {
        SDL_FreeSurface(surface);
        return NULL;
    }

    SDL_FreeSurface(surface);

    TRACE("Setting surface blend mode");
    return_code =
        SDL_SetSurfaceBlendMode(optimized_surface, SDL_BLENDMODE_BLEND);
    ASSERT (return_code == 0, "SDL_SetSurfaceBlendMode error=[%s]",
            SDL_GetError()) {
        SDL_FreeSurface(optimized_surface);
        return NULL;
    }

    TRACE("Image width=[%d] height=[%d]", optimized_surface->w,
          optimized_surface->h);

    return optimized_surface;
}

int load_media(struct sdl_data* data) {
    ASSERT (data != NULL, "Argument data must not be NULL") { return -1; }

    TRACE("Loading surface png_image");
    ASSERT (data->png_image == NULL,
            "Surface must be NULL before calling load_media") {
        return -1;
    }
    data->png_image = load_img_embedded(_embed_png_loaded_png_start,
                                        _embed_png_loaded_png_size);
    ASSERT (data->png_image != NULL, "load_img_embedded error") { return -1; }

    return 0;
}

void free_media(struct sdl_data* data) {
    ASSERT (data != NULL, "Argument data must not be NULL") { return; }

    if (data->png_image != NULL) {
        TRACE("Freeing surface png_image");
        SDL_FreeSurface(data->png_image);
        data->png_image = NULL;
    }

    return;
}

int main_loop(const struct sdl_system system, const struct sdl_data data) {
    int return_code = 0;
    SDL_Event event_buffer;
    bool quit = false;
    SDL_Rect stretch_rect = {0, 0, 640, 480};
    struct timespec start_time, current_time;

    return_code = clock_gettime(CLOCK_MONOTONIC, &start_time);
    ASSERT (return_code == 0, "clock_gettime error=[%s]",
            strerror(g_assert_errno)) {
        return -1;
    }

    TRACE("Main loop start");
    while (quit == false) {
        double seconds = 0;

        // Calculate rect size based on time
        return_code = clock_gettime(CLOCK_MONOTONIC, &current_time);
        ASSERT (return_code == 0, "clock_gettime error=[%s]",
                strerror(g_assert_errno)) {
            return -1;
        }

        seconds = (double)(current_time.tv_sec - start_time.tv_sec) +
                  ((double)(current_time.tv_nsec - start_time.tv_nsec) /
                   1000000000.0);
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
            return_code = clock_gettime(CLOCK_MONOTONIC, &start_time);
            ASSERT (return_code == 0, "clock_gettime error=[%s]",
                    strerror(g_assert_errno)) {
                return -1;
            }
            continue;
        }

        // Fill the surface with color
        return_code = SDL_FillRect(
            system.screen_surface, NULL,
            SDL_MapRGB(system.screen_surface->format, 0x00, 0x80, 0x80));
        ASSERT (return_code == 0, "SDL_FillRect error=[%s]", SDL_GetError()) {
            return -1;
        }

        // Blit surface to window
        return_code = SDL_BlitScaled(data.png_image, NULL,
                                     system.screen_surface, &stretch_rect);
        ASSERT (return_code == 0, "SDL_BlitScaled error=[%s]", SDL_GetError()) {
            return -1;
        }

        // Update the window surface
        return_code = SDL_UpdateWindowSurface(system.window);
        ASSERT (return_code == 0, "SDL_UpdateWindowSurface error=[%s]",
                SDL_GetError()) {
            return -1;
        }

        // Poll for currently pending events
        do {
            return_code = SDL_PollEvent(&event_buffer);
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
    return_code = load_media(&data);
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
