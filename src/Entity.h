#ifndef ENTITY_H
#define ENTITY_H

#include <SDL2/SDL.h>
#include "Rectangle.h"

class Entity {
public:
    Entity(Rectangle rect, SDL_Color color, bool weighted);

    void render(SDL_Renderer* renderer);
    void move(int dx, int dy);
    void setVelocity(float vx, float vy);
    void setWeight(bool w);
    Rectangle getRect() const;
    SDL_Color getColor() const;

    struct {
        float x;
        float y;
    } velocity;

    bool weighted;
private:
    Rectangle rect;
    SDL_Color color;
};

#endif // ENTITY_H
