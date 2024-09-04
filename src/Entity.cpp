#include "Entity.h"

Entity::Entity(const Rectangle& rect, SDL_Color color):rect(rect),color(color){}

void Entity::render(SDL_Renderer* renderer) {
    SDL_Rect sdlRect = {rect.x, rect.y, rect.width, rect.height};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &sdlRect);
}

void Entity::move(int dx, int dy){
    rect.x +=dx;
    rect.y+=dy;
}

SDL_Rect Entity::getRect() const {
    return {rect.x, rect.y, rect.width, rect.height};
}

