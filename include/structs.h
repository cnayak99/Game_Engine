#ifndef STRUCTS_H
#define STRUCTS_H

#include <SDL2/SDL.h>

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
} Game;

#endif // STRUCTS_H