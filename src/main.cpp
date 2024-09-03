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

    bool quit = false;
    SDL_Event e;

    // Define rectangles for the three shapes
    SDL_Rect staticRect = { 100, 100, 100, 100 };       // Static shape
    SDL_Rect controllableRect = { 300, 300, 50, 50 };   // Controllable shape
    SDL_Rect movingRect = { 500, 100, 75, 75 };         // Moving shape

    int speed = 5; // Speed for the moving shape

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Move the controllable shape (placeholder logic, will be controlled later)
        controllableRect.y += 2;
        if (controllableRect.y > 1080) controllableRect.y = 0;

        // Move the shape in a continuous pattern (horizontal oscillation)
        movingRect.x += speed;
        if (movingRect.x > 1820 || movingRect.x < 100) speed = -speed;

        // Set the background color to blue and clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderClear(renderer);

        // Render the static shape (red)
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &staticRect);

        // Render the controllable shape (green)
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &controllableRect);

        // Render the moving shape (blue)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black color
        SDL_RenderFillRect(renderer, &movingRect);

        // Present the rendered content
        SDL_RenderPresent(renderer);

        // Delay to control frame rate (optional)
        SDL_Delay(16); // ~60 frames per second
    }

    // Clean up and shut down SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
