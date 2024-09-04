#ifndef ENTITY_H
#define ENTITY_H

#include "Rectangle.h"
#include <SDL2/SDL.h>

class Entity {
public:
    Entity(Rectangle rect, SDL_Color color);

    void render(SDL_Renderer* renderer);
    void move(int dx, int dy);
    void setVelocity(float vx, float vy);
    Rectangle getRect() const;
    SDL_Color getColor() const;

    struct {
        float x;
        float y;
    } velocity;

private:
    Rectangle rect;
    SDL_Color color;
};

#endif // ENTITY_H
