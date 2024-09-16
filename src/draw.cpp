// draw.cpp
#include <SDL2/SDL.h>
#include <iostream>
#include "sdl_setup.h"
#include "draw.h"

void prepScene(Game *game) {
        // Set the background color to blue and clear the screen
        SDL_SetRenderDrawColor(game->renderer, 96, 128, 255, 255);
        SDL_RenderClear(game->renderer);
}

void presentScene(Game *game) {
        // Present the rendered content
        SDL_RenderPresent(game->renderer);
}