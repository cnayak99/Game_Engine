#ifndef ENTITY_H
#define ENTITY_H

#include <SDL2/SDL.h>

/**
 * Creates an entity struct.
 * 
 * References resources and tutorials provided by Professor Card through
 * the "CSC 481-581 HW 1-4.pdf" located beneath the "Homework 1" title in
 * the "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page. These resources can be found in the
 * https://wiki.libsdl.org/SDL2/FrontPage website. Particularly based on
 * tutorials and guidance from the wiki for drawing entities.
 */
class Entity {
public:
    Entity(int x, int y, int width, int height, SDL_Color color, bool weighted);

    bool weighted;
    void render(SDL_Renderer* renderer);
    void move(int dx, int dy);
    void setVelocity(float vx, float vy);
    void setWeight(bool w);
    void setPosition(int x, int y);
    SDL_Rect getRect() const;
    SDL_Color getColor() const;
    bool getWeight() const;

    struct {
        float x;
        float y;
    } velocity;

private:
    SDL_Rect rect;
    SDL_Color color;
};

#endif // ENTITY_H
