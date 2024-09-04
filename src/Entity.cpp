#include "Entity.h"

Entity::Entity(Rectangle rect, SDL_Color color)
    : velocity({0, 0}), rect(rect), color(color) {}// Initialize in the same order as declared

void Entity::render(SDL_Renderer* renderer) {
    SDL_Rect sdlRect = {rect.x, rect.y, rect.width, rect.height};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &sdlRect);
}

void Entity::move(int dx, int dy) {
    rect.x += dx;
    rect.y += dy;
}

void Entity::setVelocity(float vx, float vy) {
    velocity.x = vx;
    velocity.y = vy;
}

Rectangle Entity::getRect() const {
    return rect;
}

SDL_Color Entity::getColor() const {
    return color;
}
