#ifndef STRUCTS_H
#define STRUCTS_H

#include <SDL2/SDL.h>
#include "Entity.h"
#include "Timeline.h"

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
    int moveSpeed;

    float delta;

    const Uint8* state;

    Entity *s;
    Entity *c;
    Entity *m;

    Timeline *a;

    bool quit;
} Concepts;

#endif // STRUCTS_H