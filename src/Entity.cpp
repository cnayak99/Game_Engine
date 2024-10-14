#include "Entity.h"

Entity::Entity(int x, int y, int width, int height, SDL_Color color, bool weighted)
    : velocity({0, 0}), color(color), weighted(weighted){
        rect.x = x;
        rect.y = y;
        rect.w = width;
        rect.h = height;
    }// Initialize in the same order as declared


void Entity::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void Entity::move(int dx, int dy) {
    rect.x += dx;
    rect.y += dy;
}

void Entity::setVelocity(float vx, float vy) {
    velocity.x = vx;
    velocity.y = vy;
}

void Entity::setPosition(int x, int y) {
    rect.x = x;
    rect.y = y;  
}

void Entity::setWeight(bool w) {
    weighted = w;
}

SDL_Rect Entity::getRect() const {
    return rect;
}

SDL_Color Entity::getColor() const {
    return color;
}

bool Entity::getWeight() const {
    return weighted;
}