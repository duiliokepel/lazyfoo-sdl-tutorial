#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "assert.h"
#include "embed/hello_world.bmp.h"
#include "trace.h"

struct sdl_system {
    SDL_Window* window;
    SDL_Surface* screen_surface;
};

struct sdl_data {
    SDL_Surface* image_hello_world;
};

int init_SDL(struct sdl_system* system);
void close_SDL(struct sdl_system* system);

int load_media(struct sdl_data* data);
void free_media(struct sdl_data* data);

int init_SDL(struct sdl_system* system) {
    int return_code = 0;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;

    ASSERT(system != NULL, return -1;, "Argument system must not be NULL");
    ASSERT(system->window == NULL, return -1;, "Argument system->window must be NULL before initialization");
    ASSERT(system->screen_surface == NULL, return -1;
           , "Argument system->screen_surface must be NULL before initialization");

    TRACE("Initializing SDL");
    return_code = SDL_Init(SDL_INIT_VIDEO);
    ASSERT(return_code == 0, return -1;, "SDL_Init error=[%s]", SDL_GetError());

    TRACE("Creating window");
    system->window = SDL_CreateWindow("SDL Tutorial 02 - Getting an Image on the Screen", SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    ASSERT(system->window != NULL, SDL_Quit(); return -1;, "SDL_CreateWindow error=[%s]", SDL_GetError());

    TRACE("Getting window surface");
    system->screen_surface = SDL_GetWindowSurface(system->window);
    ASSERT(system->screen_surface != NULL, close_SDL(system); return -1;
           , "SDL_GetWindowSurface error=[%s]", SDL_GetError());

    return 0;
}

void close_SDL(struct sdl_system* system) {
    ASSERT(system != NULL, return;, "Argument system must not be NULL");

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

int load_media(struct sdl_data* data) {
    int return_code = 0;
    SDL_RWops* image_RWops = NULL;

    ASSERT(data != NULL, return -1;, "Argument data must not be NULL");
    ASSERT(data->image_hello_world == NULL, return -1;
           , "Argument data->image_hello_world must be NULL before load_media");

    TRACE("Opening stream to embedded hello_world.bmp");
    image_RWops = SDL_RWFromConstMem(_embed_hello_world_bmp_start, (int)_embed_hello_world_bmp_size);
    ASSERT(image_RWops != NULL, return -1;, "SDL_RWFromConstMem error=[%s]", SDL_GetError());

    TRACE("Loading surface image_hello_world");
    data->image_hello_world = SDL_LoadBMP_RW(image_RWops, 0);
    ASSERT(data->image_hello_world != NULL, SDL_RWclose(image_RWops); return -1;
           , "SDL_LoadBMP_RW error=[%s]", SDL_GetError());

    TRACE("Closing stream to embedded hello_world.bmp");
    return_code = SDL_RWclose(image_RWops);
    ASSERT(return_code == 0, SDL_FreeSurface(data->image_hello_world); data->image_hello_world = NULL; return -1;
           , "SDL_RWclose error=[%s]", SDL_GetError());

    TRACE("Image width=[%d] height=[%d]", data->image_hello_world->w, data->image_hello_world->h);
    return 0;
}

void free_media(struct sdl_data* data) {
    ASSERT(data != NULL, return;, "Argument data must not be NULL");

    if (data->image_hello_world != NULL) {
        TRACE("Freeing surface image_hello_world");
        SDL_FreeSurface(data->image_hello_world);
        data->image_hello_world = NULL;
    }

    return;
}

int main(int argc, char** argv) {
    int return_code = 0;
    struct sdl_system system = {0};
    struct sdl_data data = {0};
    SDL_Event event_buffer;
    bool quit = false;

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
    return_code = load_media(&data);
    ASSERT(return_code == 0, free_media(&data); close_SDL(&system); return -1;, "load_media error");

    TRACE("Blitting surface to window");
    return_code = SDL_BlitSurface(data.image_hello_world, NULL, system.screen_surface, NULL);
    ASSERT(return_code == 0, free_media(&data); close_SDL(&system); return -1;
           , "SDL_BlitSurface error=[%s]", SDL_GetError());

    TRACE("Main loop start");
    while (quit == false) {
        // Update the surface
        return_code = SDL_UpdateWindowSurface(system.window);
        ASSERT(return_code == 0, free_media(&data); close_SDL(&system); return -1;
               , "SDL_UpdateWindowSurface error=[%s]", SDL_GetError());

        // Poll for currently pending events
        do {
            return_code = SDL_PollEvent(&event_buffer);
            if (return_code == 1 && event_buffer.type == SDL_QUIT) {
                TRACE("Quit");
                quit = true;
            }
        } while (return_code == 1);

        // sleep
        nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = (1000000000 / 60)}, NULL);
    }

    TRACE("Freeing media");
    free_media(&data);

    TRACE("Closing");
    close_SDL(&system);

    TRACE("end");
    return 0;
}
