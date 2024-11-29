#ifndef STRUCTS_H
#define STRUCTS_H

#include <SDL2/SDL.h>
#include "Entity.h"
#include "Timeline.h"
#include "defs.h"

class Entity;

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
    int tailLength;
    float delta;

    	// Player position variables
	int x;
	int y;
	int prevX;
	int prevY;

	// Movement controls
	bool up;
	bool down;
	bool right ;
	bool left;

	bool inputThisFrame;
	bool redo;

    const Uint8* state;

    Entity *food;
    Entity *snake;
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