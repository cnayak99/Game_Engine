// main.cpp
#include <SDL2/SDL.h>
#include <iostream>
#include "sdl_setup.h"

int main(int argc, char* argv[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!initializeSDL(&window, &renderer)) {
        return 1; // Initialization failed
    }

    // Set the background color to blue
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // RGBA: Blue

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Clear the screen with the blue color
        SDL_RenderClear(renderer);

        // Present the rendered content
        SDL_RenderPresent(renderer);
    }

    // Clean up and shut down SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
