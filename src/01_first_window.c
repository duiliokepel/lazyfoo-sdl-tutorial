#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char** argv) {
    int result = 0;

    SDL_Window* window = NULL;
    SDL_Surface* screenSurface = NULL;
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    SDL_Event eventBuffer;
    bool quit = false;

    // Command line
    printf("argc=[%d]\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]=[%s]\n", i, argv[i]);
    }

    // Initialize SDL
    result = SDL_Init(SDL_INIT_VIDEO);
    if( result < 0 ) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        return -1;
    }

    // Create window
    window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    if( window == NULL ) {
        printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        SDL_Quit();
        return -1;
    }

    // Get window surface
    screenSurface = SDL_GetWindowSurface(window);

    // Fill the surface white
    result = SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );
    if ( result != 0 ) {
        printf( "SDL_FillRect - SDL_Error: %s\n", SDL_GetError() );
        SDL_Quit();
        return -1;
    }

    // Update the surface
    result = SDL_UpdateWindowSurface( window );
    if ( result != 0 ) {
        printf( "SDL_FillRect - SDL_Error: %s\n", SDL_GetError() );
        SDL_Quit();
        return -1;
    }

    // Main loop
    while( quit == false ) {
        // Poll for currently pending events
        result = SDL_PollEvent( &eventBuffer );
        if (result == 1) {
            if( eventBuffer.type == SDL_QUIT ) {
                quit = true;
            }
        }
    }

    // Destroy window
    SDL_DestroyWindow( window );

    // Quit SDL subsystems
    SDL_Quit();

    return 0;
}
