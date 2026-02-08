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
    ASSERT (system->renderer == NULL,
            "Argument system->renderer must be NULL before initialization") {
        return -1;
    }

    TRACE("Initializing SDL");
    return_code = SDL_Init(SDL_INIT_VIDEO);
    ASSERT (return_code == 0, "SDL_Init error=[%s]", SDL_GetError()) {
        return -1;
    }

    TRACE("Creating window");
    system->window = SDL_CreateWindow(
        "SDL Tutorial 08 - Geometry Rendering", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    ASSERT (system->window != NULL, "SDL_CreateWindow error=[%s]",
            SDL_GetError()) {
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
    ASSERT (system != NULL, "Argument system must not be NULL") {
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
    int return_code = 0;
    SDL_Event event_buffer;
    bool quit = false;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    SDL_Rect fill_rect = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2,
                          SCREEN_HEIGHT / 2};
    SDL_Rect outline_rect = {SCREEN_WIDTH / 6, SCREEN_HEIGHT / 6,
                             SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT * 2 / 3};
    int counter = 0;
    int mouse_x = 0, mouse_y = 0;
    double p0_x, p0_y;
    double p1_x, p1_y;
    const double CURSOR_RADIUS = 50.0;
    const int CURSOR_STEPS = 20;

    ASSERT (system.renderer != NULL,
            "Argument system.renderer must not be NULL") {
        return -1;
    }

    TRACE("Main loop start");
    while (quit == false) {
        // Set renderer color
        return_code =
            SDL_SetRenderDrawColor(system.renderer, 0x00, 0x80, 0x80, 0xFF);
        ASSERT (return_code == 0, "SDL_SetRenderDrawColor error=[%s]",
                SDL_GetError()) {
            return -1;
        }

        // Clear screen
        return_code = SDL_RenderClear(system.renderer);
        ASSERT (return_code == 0, "SDL_RenderClear error=[%s]",
                SDL_GetError()) {
            return -1;
        }

        // Render red filled quad
        return_code =
            SDL_SetRenderDrawColor(system.renderer, 0xFF, 0x00, 0x00, 0xFF);
        ASSERT (return_code == 0, "SDL_SetRenderDrawColor error=[%s]",
                SDL_GetError()) {
            return -1;
        }

        return_code = SDL_RenderFillRect(system.renderer, &fill_rect);
        ASSERT (return_code == 0, "SDL_RenderFillRect error=[%s]",
                SDL_GetError()) {
            return -1;
        }

        // Render green outlined quad
        return_code =
            SDL_SetRenderDrawColor(system.renderer, 0x00, 0xFF, 0x00, 0xFF);
        ASSERT (return_code == 0, "SDL_SetRenderDrawColor error=[%s]",
                SDL_GetError()) {
            return -1;
        }

        return_code = SDL_RenderDrawRect(system.renderer, &outline_rect);
        ASSERT (return_code == 0, "SDL_RenderDrawRect error=[%s]",
                SDL_GetError()) {
            return -1;
        }

        // Draw blue horizontal line
        return_code =
            SDL_SetRenderDrawColor(system.renderer, 0x00, 0x00, 0xFF, 0xFF);
        ASSERT (return_code == 0, "SDL_SetRenderDrawColor error=[%s]",
                SDL_GetError()) {
            return -1;
        }

        return_code = SDL_RenderDrawLine(system.renderer, 0, SCREEN_HEIGHT / 2,
                                         SCREEN_WIDTH, SCREEN_HEIGHT / 2);
        ASSERT (return_code == 0, "SDL_RenderDrawLine error=[%s]",
                SDL_GetError()) {
            return -1;
        }

        // Draw vertical line of yellow dots
        return_code =
            SDL_SetRenderDrawColor(system.renderer, 0xFF, 0xFF, 0x00, 0xFF);
        ASSERT (return_code == 0, "SDL_SetRenderDrawColor error=[%s]",
                SDL_GetError()) {
            return -1;
        }

        for (counter = 0; counter < SCREEN_HEIGHT; counter += 4) {
            return_code =
                SDL_RenderDrawPoint(system.renderer, SCREEN_WIDTH / 2, counter);
            ASSERT (return_code == 0, "SDL_RenderDrawPoint error=[%s]",
                    SDL_GetError()) {
                return -1;
            }
        }

        // Draw a white circle around the mouse
        return_code =
            SDL_SetRenderDrawColor(system.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        ASSERT (return_code == 0, "SDL_SetRenderDrawColor error=[%s]",
                SDL_GetError()) {
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

            return_code = SDL_RenderDrawLine(system.renderer, x0, y0, x1, y1);
            ASSERT (return_code == 0, "SDL_RenderDrawLine error=[%s]",
                    SDL_GetError()) {
                return -1;
            }

            p0_x = p1_x;
            p0_y = p1_y;
        }

        // Update screen
        SDL_RenderPresent(system.renderer);

        // Poll for currently pending events
        do {
            return_code = SDL_PollEvent(&event_buffer);
            ASSERT (return_code >= 0, "SDL_PollEvent error=[%s]",
                    SDL_GetError()) {
                return -1;
            }

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
        nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = (1000000000 / 60)},
                  NULL);
    }
    return 0;
}

int main(int argc, char** argv) {
    int return_code = 0;
    struct sdl_system system = {0};

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

    return_code = main_loop(system);
    ASSERT (return_code == 0, "main_loop error") {
        close_SDL(&system);
        return -1;
    }

    TRACE("Closing");
    close_SDL(&system);

    TRACE("end");
    return 0;
}
