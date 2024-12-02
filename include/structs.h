#ifndef STRUCTS_H
#define STRUCTS_H

#include <SDL2/SDL.h>
#include "Entity.h"
#include "Timeline.h"
#include "defs.h"
#include <SDL2/SDL_ttf.h>

class Entity;

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
    TTF_Font* font;
} Game;

/** Holds all necessary concepts. */
typedef struct {
    bool scaling;
    bool running;
    bool held;

    float gravity;
    int speed;
    float verticalVel;
    float thrust;
    int moveSpeed;
    float delta;
    int liveCount;
    float velY;
    float velX;
    	// Player position variables

	// Movement controls


    const Uint8* state;

    Entity *ball;
    Entity *paddle;
    Entity *brick;
    Entity *lives;
    Timeline *a;

    Entity *s;
    Entity *c;
    Entity *m;
    Entity *v;

    Entity *spawn;
    Entity *despawn;
    Entity* getSource;


    SDL_Rect hitBL;
    SDL_Rect hitBR;
    SDL_Rect hitTR;
    SDL_Rect hitTL;
    Entity* tileMap[MAP_WIDTH][MAP_HEIGHT];

    bool quit;
} Concepts;

#endif // STRUCTS_H