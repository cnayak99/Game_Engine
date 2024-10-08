#include "Entity.h"

/**
 * Creates an entity object.
 * 
 * References resources and tutorials provided by Professor Card through
 * the "CSC 481-581 HW 1-4.pdf" located beneath the "Homework 1" title in
 * the "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page. These resources can be found in the
 * https://wiki.libsdl.org/SDL2/FrontPage website. Particularly based on
 * tutorials and guidance from the wiki for drawing entities.
 */
Entity::Entity(Rectangle rect, SDL_Color color)
    : velocity({0, 0}), rect(rect), color(color) {}// Initialize in the same order as declared

/**
 * Renders the entity.
 * 
 * References resources and tutorials provided by Professor Card through
 * the "CSC 481-581 HW 1-4.pdf" located beneath the "Homework 1" title in
 * the "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page. These resources can be found in the
 * https://wiki.libsdl.org/SDL2/FrontPage website. Particularly based on
 * tutorials and guidance from the wiki for drawing entities.
 */
void Entity::render(SDL_Renderer* renderer) {
    SDL_Rect sdlRect = {rect.x, rect.y, rect.width, rect.height};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &sdlRect);
}

/**
 * Moves the entity.
 * 
 * Based on physics knowledge.
 */
void Entity::move(int dx, int dy) {
    rect.x += dx;
    rect.y += dy;
}

/**
 * Sets the velocity for the entity.
 * 
 * Based on physics knowledge.
 */
void Entity::setVelocity(float vx, float vy) {
    velocity.x = vx;
    velocity.y = vy;
}

/**
 * Returns the entity.
 */
Rectangle Entity::getRect() const {
    return rect;
}

/**
 * Returns the color of the entity.
 * 
 * References resources and tutorials provided by Professor Card through
 * the "CSC 481-581 HW 1-4.pdf" located beneath the "Homework 1" title in
 * the "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page. These resources can be found in the
 * https://wiki.libsdl.org/SDL2/FrontPage website. Particularly based on
 * tutorials and guidance from the wiki for drawing entities.
 */
SDL_Color Entity::getColor() const {
    return color;
}
