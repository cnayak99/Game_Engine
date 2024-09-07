// main.cpp
#include <SDL2/SDL.h>
#include <iostream>
#include "sdl_setup.h"
#include "Entity.h"
#include "Rectangle.h"
#include "Physics.h"
#include "Intersect.h"

/**
 * Runs the game.
 * 
 * References resources and tutorials provided by Professor Card through
 * the "CSC 481-581 HW 1-4.pdf" located beneath the "Homework 1" title in
 * the "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page. These resources can be found in the
 * https://wiki.libsdl.org/SDL2/FrontPage website.
 * 
 * \param argc the count argument
 * \param argv the string argument
 * \returns int 0 if successful, else, unsuccessful
 * 
 * @author Lillie Sharpe
 * @author Chinmay Nayak
 * @author Robbie Martin
 */

int main(int argc, char* argv[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!initializeSDL(&window, &renderer)) {
        return 1; // Initialization failed
    }

    bool quit = false;

    SDL_Event e;

    // Create static entities and moving entity
    int windowWidth = 1920;
    int windowHeight = 1080;
    Entity staticEntity(Rectangle(0, windowHeight-500, 300, 100), {255, 0, 0, 255}); // Static Red Shape (left)
    Entity staticEntityRight(Rectangle(windowWidth - 300, windowHeight - 500, 300, 100), {255, 0, 0, 255}); // Static Red Shape (right)
    Entity controllableEntity(Rectangle(300, 300, 50, 50), {0, 255, 0, 255}); // Controllable Green Shape
    Entity movingEntity(Rectangle(300, windowHeight - 500, 300, 10), {0, 0, 0, 255}); // Black Moving Shape, matches the height    bool scaling = false;
    bool held = false;
    float gravity = 9.888f;
    Physics physics(gravity);

    int speed = 5; // Speed of the moving entity
    float verticalVel = 0.0f;
    float thrust = -9.888f;
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

        if (state[SDL_SCANCODE_UP]) {
            verticalVel = thrust;
        }

        if (state[SDL_SCANCODE_LEFT]) { // Move left
            controllableEntity.move(-moveSpeed, 0);
        }
        if (state[SDL_SCANCODE_RIGHT]) { // Move right
            controllableEntity.move(moveSpeed, 0);
        }
        bool scaling = false;
        if (state[SDL_SCANCODE_C]) {
            if (!held) {
                held = true;
                if (!scaling) {
                    SDL_RenderSetLogicalSize(renderer, 1920, 1080);
                    scaling = true;
                }
                else {
                    SDL_RenderSetLogicalSize(renderer, 0, 0);
                    scaling = false;
                }
            }
        }
        else {
            held = false;
        }
        if (state[SDL_SCANCODE_ESCAPE]) { // Exit the game
            quit = true;
        }

        // Apply gravity to the controllable entity
        verticalVel += gravity * deltaTime;
        controllableEntity.move(0, static_cast<int>(verticalVel));

        // Move the moving entity and check for oscillation
        movingEntity.move(speed, 0);

        // Keeps track of the rectangles
        Rectangle c = controllableEntity.getRect(); // Controllable entity
        Rectangle m = movingEntity.getRect();       // Moving entity
        Rectangle s = staticEntity.getRect();       // Left static entity
        Rectangle sr = staticEntityRight.getRect(); // Right static entity

        // Check for collision with staticEntity (left)
        if (hasIntersection(&m, &s) == true) {
            speed = -speed; // Reverse direction if collision detected
        }

        // Check for collision with staticEntityRight (right)
        if (hasIntersection(&m, &sr) == true) {
            speed = -speed; // Reverse direction if collision detected
        }

        // Check for collision between controllableEntity and movingEntity
        if (hasIntersection(&c, &m) == true) {
            if (intersect(&c, &m) == 2) { // Collision on top
                deltaTime = 0;
                verticalVel = 0;
                controllableEntity.move(speed, static_cast<int>(verticalVel));
                if (controllableEntity.getRect().x > 1820 || controllableEntity.getRect().x < 100) {
                    speed = -speed;
                }
            }
        }

        // Check for collision between controllableEntity and staticEntity (left)
        if (hasIntersection(&c, &s) == true) {
            if (intersect(&c, &s) == 2) { // Collision on top
                deltaTime = 0;
                verticalVel = 0;
            }
        }

        // Check for collision between controllableEntity and staticEntityRight (right)
        if (hasIntersection(&c, &sr) == true) {
            if (intersect(&c, &sr) == 2) { // Collision on top
                deltaTime = 0;
                verticalVel = 0;
            }
        }

        // Set the background color to blue and clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderClear(renderer);

        // Render the shapes
        staticEntity.render(renderer);
        staticEntityRight.render(renderer);
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
