#ifndef ENTITY_H
#define ENTITY_H

#include "Rectangle.h"
#include <SDL2/SDL.h>

class Entity {
public:
    Entity(const Rectangle& rect, SDL_Color color);
    void move(int dx, int dy);
    void render(SDL_Renderer* renderer);
    SDL_Rect getRect() const;

private:
    Rectangle rect;
    SDL_Color color;
};

#endif
