#ifndef ENTITY_H
#define ENTITY_H

#include <SDL2/SDL.h>
#include "Event.h"
#include "structs.h"
/**
 * Creates an entity struct.
 * 
 * References resources and tutorials provided by Professor Card through
 * the "CSC 481-581 HW 1-4.pdf" located beneath the "Homework 1" title in
 * the "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page. These resources can be found in the
 * https://wiki.libsdl.org/SDL2/FrontPage website. Particularly based on
 * tutorials and guidance from the wiki for drawing entities.
 * 
 * The "Special" field determines whether or not the Entity is normal,
 * a spawn point, or a death zone. If it is 0, the Entity is normal. If
 * it is 1, the Entity is a spawn point. If it is 2, the Entity is a
 * death zone.
 */
class Entity {
public:
    Entity(int x, int y, int width, int height, SDL_Color color, bool weighted, int special);

    SDL_Color color;
    bool weighted;
    int special;
    void render(SDL_Renderer* renderer);
    void move(int dx, int dy);
    void setVelocity(float vx, float vy);
    void setWeight(bool w);
    void setColor(SDL_Color color);
    void setPosition(int x, int y);
    SDL_Rect getRect() const;
    SDL_Color getColor() const;
    bool getWeight() const;
    int getSpecial() const;

    struct {
        float x;
        float y;
    } velocity;

private:
    SDL_Rect rect;
};

#endif // ENTITY_H
