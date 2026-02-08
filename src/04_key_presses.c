#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "assert.h"
#include "embed/press_default.bmp.h"
#include "embed/press_down.bmp.h"
#include "embed/press_left.bmp.h"
#include "embed/press_right.bmp.h"
#include "embed/press_up.bmp.h"
#include "trace.h"

enum key_press_surfaces {
    KEY_PRESS_DEFAULT,
    KEY_PRESS_UP,
    KEY_PRESS_DOWN,
    KEY_PRESS_LEFT,
    KEY_PRESS_RIGHT,
    KEY_PRESS_TOTAL
};

struct sdl_system {
    SDL_Window* window;
    SDL_Surface* screen_surface;
};

struct sdl_data {
    SDL_Surface* key_press_surface[KEY_PRESS_TOTAL];
};

int init_SDL(struct sdl_system* system);
void close_SDL(struct sdl_system* system);

SDL_Surface* load_bmp_embedded(const void* bmp_data, const size_t size);
int load_media(struct sdl_data* data);
void free_media(struct sdl_data* data);

int main_loop(struct sdl_system system, struct sdl_data data);
int main(int argc, char** argv);

int init_SDL(struct sdl_system* system) {
    int return_code = 0;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;

    ASSERT (system != NULL, "Argument system must not be NULL") {
        return -1;
    }
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

    TRACE("Creating window");
    system->window = SDL_CreateWindow(
        "SDL Tutorial 04 - Key Presses", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    ASSERT (system->window != NULL, "SDL_CreateWindow error=[%s]",
            SDL_GetError()) {
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
    ASSERT (system != NULL, "Argument system must not be NULL") {
        return;
    }

    if (system->screen_surface != NULL) {
        system->screen_surface = NULL;
    }

    if (system->window != NULL) {
        TRACE("Destroying window");
        SDL_DestroyWindow(system->window);
        system->window = NULL;
    }

    TRACE("Quitting SDL");
    SDL_Quit();
    return;
}

SDL_Surface* load_bmp_embedded(const void* bmp_data, const size_t size) {
    SDL_RWops* rwops = NULL;
    SDL_Surface* return_surface = NULL;

    ASSERT (bmp_data != NULL, "Argument bmp_data must not be NULL") {
        return NULL;
    }
    ASSERT (size > 0, "Argument size must be larger than 0") {
        return NULL;
    }
    ASSERT (size <= INT_MAX, "Argument size must not exceed maximum allowed") {
        return NULL;
    }

    TRACE("Opening stream to embedded");
    rwops = SDL_RWFromConstMem(bmp_data, (int)size);
    ASSERT (rwops != NULL, "SDL_RWFromConstMem error=[%s]", SDL_GetError()) {
        return NULL;
    }

    TRACE("Loading surface");
    return_surface = SDL_LoadBMP_RW(rwops, 1);
    ASSERT (return_surface != NULL, "SDL_LoadBMP_RW error=[%s]",
            SDL_GetError()) {
        return NULL;
    }

    TRACE("Image width=[%d] height=[%d]", return_surface->w, return_surface->h);
    return return_surface;
}

int load_media(struct sdl_data* data) {
    ASSERT (data != NULL, "Argument data must not be NULL") {
        return -1;
    }

    TRACE("Loading surface press_default");
    ASSERT (data->key_press_surface[KEY_PRESS_DEFAULT] == NULL,
            "Surface must be NULL before calling load_media") {
        return -1;
    }
    data->key_press_surface[KEY_PRESS_DEFAULT] = load_bmp_embedded(
        _embed_press_default_bmp_start, _embed_press_default_bmp_size);
    ASSERT (data->key_press_surface[KEY_PRESS_DEFAULT] != NULL,
            "load_bmp_embedded error") {
        return -1;
    }

    TRACE("Loading surface press_up");
    ASSERT (data->key_press_surface[KEY_PRESS_UP] == NULL,
            "Surface must be NULL before calling load_media") {
        return -1;
    }
    data->key_press_surface[KEY_PRESS_UP] =
        load_bmp_embedded(_embed_press_up_bmp_start, _embed_press_up_bmp_size);
    ASSERT (data->key_press_surface[KEY_PRESS_UP] != NULL,
            "load_bmp_embedded error") {
        return -1;
    }

    TRACE("Loading surface press_down");
    ASSERT (data->key_press_surface[KEY_PRESS_DOWN] == NULL,
            "Surface must be NULL before calling load_media") {
        return -1;
    }
    data->key_press_surface[KEY_PRESS_DOWN] = load_bmp_embedded(
        _embed_press_down_bmp_start, _embed_press_down_bmp_size);
    ASSERT (data->key_press_surface[KEY_PRESS_DOWN] != NULL,
            "load_bmp_embedded error") {
        return -1;
    }

    TRACE("Loading surface press_left");
    ASSERT (data->key_press_surface[KEY_PRESS_LEFT] == NULL,
            "Surface must be NULL before calling load_media") {
        return -1;
    }
    data->key_press_surface[KEY_PRESS_LEFT] = load_bmp_embedded(
        _embed_press_left_bmp_start, _embed_press_left_bmp_size);
    ASSERT (data->key_press_surface[KEY_PRESS_LEFT] != NULL,
            "load_bmp_embedded error") {
        return -1;
    }

    TRACE("Loading surface press_right");
    ASSERT (data->key_press_surface[KEY_PRESS_RIGHT] == NULL,
            "Surface must be NULL before calling load_media") {
        return -1;
    }
    data->key_press_surface[KEY_PRESS_RIGHT] = load_bmp_embedded(
        _embed_press_right_bmp_start, _embed_press_right_bmp_size);
    ASSERT (data->key_press_surface[KEY_PRESS_RIGHT] != NULL,
            "load_bmp_embedded error") {
        return -1;
    }

    return 0;
}

void free_media(struct sdl_data* data) {
    int counter = 0;
    ASSERT (data != NULL, "Argument data must not be NULL") {
        return;
    }

    for (counter = 0; counter < KEY_PRESS_TOTAL; counter++) {
        if (data->key_press_surface[counter] != NULL) {
            TRACE("Freeing surface %d", counter);
            SDL_FreeSurface(data->key_press_surface[counter]);
            data->key_press_surface[counter] = NULL;
        }
    }

    return;
}

int main_loop(struct sdl_system system, struct sdl_data data) {
    int return_code = 0;
    SDL_Event event_buffer;
    bool quit = false;
    int last_surface = KEY_PRESS_TOTAL;
    int current_surface = KEY_PRESS_DEFAULT;

    TRACE("Main loop start");
    while (quit == false) {
        // Update current image shown on screen
        if (current_surface != last_surface) {
            TRACE("Blitting surface %d to window", current_surface);
            ASSERT (data.key_press_surface[current_surface] != NULL,
                    "Surface missing") {
                return -1;
            }
            return_code =
                SDL_BlitSurface(data.key_press_surface[current_surface], NULL,
                                system.screen_surface, NULL);
            ASSERT (return_code == 0, "SDL_BlitSurface error=[%s]",
                    SDL_GetError()) {
                return -1;
            }
            last_surface = current_surface;
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
            if (return_code == 0) {
                break;
            }
            switch (event_buffer.type) {
                case SDL_KEYDOWN: {
                    switch (event_buffer.key.keysym.sym) {
                        case SDLK_UP: {
                            current_surface = KEY_PRESS_UP;
                            break;
                        }

                        case SDLK_DOWN: {
                            current_surface = KEY_PRESS_DOWN;
                            break;
                        }

                        case SDLK_LEFT: {
                            current_surface = KEY_PRESS_LEFT;
                            break;
                        }

                        case SDLK_RIGHT: {
                            current_surface = KEY_PRESS_RIGHT;
                            break;
                        }

                        default: {
                            current_surface = KEY_PRESS_DEFAULT;
                            break;
                        }
                    }
                    break;
                }
                case SDL_KEYUP: {
                    current_surface = KEY_PRESS_DEFAULT;
                    break;
                }
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
