#ifndef STRUCTS_H
#define STRUCTS_H

#include <SDL2/SDL.h>
#include "Entity.h"

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
} Game;

/** Holds all necessary concepts. */
typedef struct {
    bool scaling;
    bool held;

    float gravity;
    int speed;
    float verticalVel;
    float thrust;

    Entity *s;
    Entity *c;
    Entity *m;

    bool quit;
} Concepts;

#endif // STRUCTS_H