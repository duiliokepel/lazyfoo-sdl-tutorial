#define _DEFAULT_SOURCE

#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "assert.h"
#include "trace.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct sdl_system {
    SDL_Window* window;
    SDL_Renderer* renderer;
};

int init_SDL(struct sdl_system* system);
void close_SDL(struct sdl_system* system);
int main_loop(struct sdl_system system);

int init_SDL(struct sdl_system* system) {
    int result = 0;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;

    if (!C_ASSERT(system != NULL)) {
        TRACE("Invalid parameter system");
        return -1;
    }
    if (!C_ASSERT(system->window == NULL)) {
        TRACE("window must be NULL before init");
        return -1;
    }
    if (!C_ASSERT(system->renderer == NULL)) {
        TRACE("renderer must be NULL before init");
        return -1;
    }

    TRACE("Initializing SDL");
    result = SDL_Init(SDL_INIT_VIDEO);
    if (!C_ASSERT(result == 0)) {
        TRACE("SDL_Init() error=[%s]", SDL_GetError());
        return -1;
    }

    TRACE("Creating window");
    system->window = SDL_CreateWindow("SDL Tutorial 08 - Geometry Rendering", SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!C_ASSERT(system->window != NULL)) {
        TRACE("SDL_CreateWindow() error=[%s]", SDL_GetError());
        return -1;
    }

    TRACE("Creating renderer for window");
    system->renderer = SDL_CreateRenderer(system->window, -1, SDL_RENDERER_ACCELERATED);
    if (!C_ASSERT(system->renderer != NULL)) {
        TRACE("SDL_CreateRenderer() error=[%s]", SDL_GetError());
        return -1;
    }

    return 0;
}

void close_SDL(struct sdl_system* system) {
    if (!C_ASSERT(system != NULL)) {
        TRACE("Invalid parameter system");
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

    TRACE("Quitting SDL");
    SDL_Quit();
    return;
}

int main_loop(struct sdl_system system) {
    int result = 0;
    SDL_Event eventBuffer;
    bool quit = false;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    SDL_Rect fill_rect = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    SDL_Rect outline_rect = {SCREEN_WIDTH / 6, SCREEN_HEIGHT / 6, SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT * 2 / 3};
    int counter = 0;
    int mouse_x, mouse_y;
    double p0_x, p0_y;
    double p1_x, p1_y;
    const double CURSOR_RADIUS = 50.0;
    const int CURSOR_STEPS = 20;

    if (!C_ASSERT(system.renderer != NULL)) {
        TRACE("Invalid renderer in main_loop");
        return -1;
    }

    TRACE("Main loop start");
    while (quit == false) {
        // Set renderer color
        result = SDL_SetRenderDrawColor(system.renderer, 0x00, 0x80, 0x80, 0xFF);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_SetRenderDrawColor() error=[%s]", SDL_GetError());
            return -1;
        }

        // Clear screen
        result = SDL_RenderClear(system.renderer);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_RenderClear() error=[%s]", SDL_GetError());
            return -1;
        }

        // Render red filled quad
        result = SDL_SetRenderDrawColor(system.renderer, 0xFF, 0x00, 0x00, 0xFF);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_SetRenderDrawColor() error=[%s]", SDL_GetError());
            return -1;
        }
        result = SDL_RenderFillRect(system.renderer, &fill_rect);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_RenderFillRect() error=[%s]", SDL_GetError());
            return -1;
        }

        // Render green outlined quad
        result = SDL_SetRenderDrawColor(system.renderer, 0x00, 0xFF, 0x00, 0xFF);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_SetRenderDrawColor() error=[%s]", SDL_GetError());
            return -1;
        }
        result = SDL_RenderDrawRect(system.renderer, &outline_rect);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_RenderDrawRect() error=[%s]", SDL_GetError());
            return -1;
        }

        // Draw blue horizontal line
        result = SDL_SetRenderDrawColor(system.renderer, 0x00, 0x00, 0xFF, 0xFF);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_SetRenderDrawColor() error=[%s]", SDL_GetError());
            return -1;
        }
        result = SDL_RenderDrawLine(system.renderer, 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_RenderDrawLine() error=[%s]", SDL_GetError());
            return -1;
        }

        // Draw vertical line of yellow dots
        result = SDL_SetRenderDrawColor(system.renderer, 0xFF, 0xFF, 0x00, 0xFF);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_SetRenderDrawColor() error=[%s]", SDL_GetError());
            return -1;
        }
        for (counter = 0; counter < SCREEN_HEIGHT; counter += 4) {
            result = SDL_RenderDrawPoint(system.renderer, SCREEN_WIDTH / 2, counter);
            if (!C_ASSERT(result == 0)) {
                TRACE("SDL_RenderDrawPoint() error=[%s]", SDL_GetError());
                return -1;
            }
        }

        // Draw a white circle around the mouse
        result = SDL_SetRenderDrawColor(system.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        if (!C_ASSERT(result == 0)) {
            TRACE("SDL_SetRenderDrawColor() error=[%s]", SDL_GetError());
            return -1;
        }
        SDL_GetMouseState(&mouse_x, &mouse_y);
        p0_x = 0.0;
        p0_y = 1.0;
        for (counter = 1; counter <= CURSOR_STEPS; counter++) {
            int x0, y0, x1, y1;
            p1_x = sin((double)counter * 2.0 * M_PI / (double)CURSOR_STEPS);
            p1_y = cos((double)counter * 2.0 * M_PI / (double)CURSOR_STEPS);

            x0 = (int)lround((double)mouse_x + (p0_x * CURSOR_RADIUS));
            y0 = (int)lround((double)mouse_y + (p0_y * CURSOR_RADIUS));
            x1 = (int)lround((double)mouse_x + (p1_x * CURSOR_RADIUS));
            y1 = (int)lround((double)mouse_y + (p1_y * CURSOR_RADIUS));

            result = SDL_RenderDrawLine(system.renderer, x0, y0, x1, y1);
            if (!C_ASSERT(result == 0)) {
                TRACE("SDL_RenderDrawLine() error=[%s]", SDL_GetError());
                return -1;
            }

            p0_x = p1_x;
            p0_y = p1_y;
        }

        // Update screen
        SDL_RenderPresent(system.renderer);

        // Poll for currently pending events
        do {
            result = SDL_PollEvent(&eventBuffer);
            if (!C_ASSERT(result >= 0)) {
                TRACE("SDL_PollEvent() error=[%s]", SDL_GetError());
                return -1;
            }

            if (result == 0) {
                break;
            }
            switch (eventBuffer.type) {
                case SDL_QUIT: {
                    TRACE("Quit");
                    quit = true;
                    break;
                }
                default: {
                    break;
                }
            }
        } while (result == 1);

        // sleep
        nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = (1000000000 / 60)}, NULL);
    }
    return 0;
}

int main(int argc, char** argv) {
    int result = 0;
    struct sdl_system system = {0};

    TRACE("Start");

    // Command line
    TRACE("argc=[%d]", argc);
    for (int i = 0; i < argc; i++) {
        TRACE("argv[%d]=[%s]", i, argv[i]);
    }

    TRACE("Initializing");
    result = init_SDL(&system);
    if (!C_ASSERT(result == 0)) {
        TRACE("init_SDL() error");
        close_SDL(&system);
        return -1;
    }

    result = main_loop(system);
    if (!C_ASSERT(result == 0)) {
        TRACE("main_loop() error");
        close_SDL(&system);
        return -1;
    }

    TRACE("Closing");
    close_SDL(&system);

    TRACE("End");
    return 0;
}
