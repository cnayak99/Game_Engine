// main.cpp
#include <SDL2/SDL.h>
#include <iostream>
#include "sdl_setup.h"
#include "Entity.h"
#include "Rectangle.h"
#include "Physics.h"
#include "Intersect.h"

int main(int argc, char* argv[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!initializeSDL(&window, &renderer)) {
        return 1; // Initialization failed
    }

    bool quit = false;

    SDL_Event e;

    Entity staticEntity(Rectangle(100,100,100,100),{255,0,0,255});//Static Red Shape
    Entity controllableEntity(Rectangle(300,300,50,50),{0,255,0,255});//Controllable Green Shape 
    Entity movingEntity(Rectangle(100,100,100,100),{0,0,0,255});//Black Moving Shape

    float gravity = 9.8f;
    Physics physics(gravity);

    int speed = 5; // Speed of the Entity
    float verticalVel =0.0f;
    float thrust = -9.8f;
    Uint32 lastTime = SDL_GetTicks();

    while (!quit) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
        
        const Uint8* state = SDL_GetKeyboardState(nullptr);
        int moveSpeed = 5;

        if(state[SDL_SCANCODE_UP]){
            // controllableEntity.move(0, -moveSpeed);
            verticalVel = thrust;
        }

        if(state[SDL_SCANCODE_LEFT]){//Move Right
            controllableEntity.move(-moveSpeed,0);
        }
        if(state[SDL_SCANCODE_RIGHT]){//Move Right
            controllableEntity.move(moveSpeed, 0);
        }
        if (state[SDL_SCANCODE_ESCAPE]) {// Exit the game
            quit = true; 
        }

        verticalVel += gravity * deltaTime;
        controllableEntity.move(0, static_cast<int>(verticalVel));
        //Apply Gravity to this object

        // Move the shape in a continuous pattern (horizontal)
        movingEntity.move(speed, 0);
        if (movingEntity.getRect().x > 1820 || movingEntity.getRect().x < 100) {
            speed = -speed;
        }

        Rectangle c = controllableEntity.getRect();
        Rectangle m = movingEntity.getRect();
        Rectangle s = staticEntity.getRect();
        Rectangle * result;

        // Attempt to sense other shapes for collision. (Test! May or may not work.)
        if (hasIntersection(&c, &m) == true) {
            if (intersect(&c, &m, result) == 1) {
                deltaTime = 0;
                verticalVel = 0;
            }
            // Add more sides for this one if successful.
        }

        // Attempt to sense other shapes for collision. (Test! May or may not work.)
        if (hasIntersection(&c, &s) == true) {
            if (intersect(&c, &s, result) == 1) {
                deltaTime = 0;
                verticalVel = 0;
            }
        }

        // Set the background color to blue and clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderClear(renderer);

        // Render the shapes
        staticEntity.render(renderer);
        controllableEntity.render(renderer);
        movingEntity.render(renderer);
        

        // Present the rendered content
        SDL_RenderPresent(renderer);

        // ~60 frames per second
        SDL_Delay(16);
    }

    // Clean up and shut down SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
