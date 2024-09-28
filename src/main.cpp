// main.cpp
#include <SDL2/SDL.h>
#include <iostream>

#include "sdl_setup.h"
#include "Entity.h"
#include "Rectangle.h"
#include "Physics.h"
#include "Intersect.h"
#include "Timeline.h"
#include "structs.h"

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
    // Create the Game object.
    Game game;
    // Set the window up.
    game.window = nullptr;
    // Set the renderer up.
    game.renderer = nullptr;

    // Attempt to initialize the game.
    if (!initializeSDL(&game.window, &game.renderer)) {
        return 1; // If unsuccessful, initialization failed.
    }

    // Create the Concepts object.
    Concepts concepts;

    // Initialize quit to false.
    concepts.quit = false;

    // Create an event object.
    SDL_Event e;

    // Construct the anchor timeline.
    Timeline anchor(nullptr, 1);

    // Creates the static red shape and connects its address to concepts.
    Entity staticEntity(Rectangle(100,100,100,100),{255,0,0,255}, false); // Static red shape.
    concepts.s = &staticEntity;

    // Creates the controllable green shape and connects its address to concepts.
    Entity controllableEntity(Rectangle(300,300,50,50),{0,255,0,255}, true); // Controllable green shape.
    concepts.c = &controllableEntity;

    // Creates the moving black shape and connects its address to concepts.
    Entity movingEntity(Rectangle(100,100,100,100),{0,0,0,255}, false); // Black moving shape.
    concepts.m = &movingEntity;

    // Initializes scaling and held through concepts.
    concepts.scaling = false;
    concepts.held = false;

    // Initializes physics variables.
    concepts.gravity = 9.8f;
    concepts.speed = 5; // Speed of the Entity.
    concepts.verticalVel = 0.0f;
    concepts.thrust = -9.8f;

    // Sets the last time.
    int64_t lastTime = anchor.getTimeline();

    // Runs the game.
    while (!concepts.quit) {
        int64_t currentTime = anchor.getTimeline();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        printf("Time: %ld\nLast Time: %ld\nDelta: %f\n", currentTime, lastTime, deltaTime);

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                concepts.quit = true;
            }
        }
        
        const Uint8* state = SDL_GetKeyboardState(nullptr);
        int moveSpeed = 5;

        if(state[SDL_SCANCODE_UP]){
            concepts.verticalVel = concepts.thrust;
        }

        if(state[SDL_SCANCODE_LEFT]){ // Move left.
            controllableEntity.move(-moveSpeed,0);
        }
        if(state[SDL_SCANCODE_RIGHT]){// Move right.
            controllableEntity.move(moveSpeed, 0);
        }
        if (state[SDL_SCANCODE_C]) {
            if (!concepts.held) {
                concepts.held = true;
                if (!concepts.scaling) {
                    SDL_RenderSetLogicalSize(game.renderer, 1920, 1080);
                    concepts.scaling = true;
                }
                else {
                    SDL_RenderSetLogicalSize(game.renderer, 0, 0);
                    concepts.scaling = false;
                }
            }
        }
        else {
            concepts.held = false;
        }
        if (state[SDL_SCANCODE_ESCAPE]) {// Exit the game
            concepts.quit = true; 
        }

        concepts.verticalVel += concepts.gravity * deltaTime * 60;
        controllableEntity.move(0, static_cast<int>(concepts.verticalVel));
        //Apply Gravity to this object

        // Move the shape in a continuous pattern (horizontal)
        movingEntity.move(concepts.speed, 0);
        if (movingEntity.getRect().x > 1820 || movingEntity.getRect().x < 100) {
            concepts.speed = -concepts.speed;
        }

        // Keeps track of the controllable rectangle.
        Rectangle c = controllableEntity.getRect();
        // Keeps track of the moving rectangle.
        Rectangle m = movingEntity.getRect();
        // Keeps track of the static rectangle.
        Rectangle s = staticEntity.getRect();
        // Stores the address of a Rectangle entity.
        // Due to redundancy, for now, 'result' has been removed.
        // Rectangle * result;

        // Senses other shapes for collision.
        if (hasIntersection(&c, &m) == true) {
            // If there was an intersection on the top of the terrain rectangle,
            // the controllable rectangle lands on the terrain rectangle.
            if (intersect(&c, &m) == 2) {
                // Causes vertical collision.
                deltaTime = 0;
                concepts.verticalVel = 0;
                // Enables player movement mimicking the moving entity.
                controllableEntity.move(concepts.speed, static_cast<int>(concepts.verticalVel));
                if (controllableEntity.getRect().x > 1820 || controllableEntity.getRect().x < 100) {
                    concepts.speed = -concepts.speed;
                }
            }
            // More sides will be added in the future.
        }

        // Senses other shapes for collision.
        if (hasIntersection(&c, &s) == true) {
            // If there was an intersection on the top of the terrain rectangle,
            // the controllable rectangle lands on the terrain rectangle.
            if (intersect(&c, &s) == 2) {
                // Causes vertical collision.
                deltaTime = 0;
                concepts.verticalVel = 0;
            }
            // More sides will be added in the future.
        }

        // Set the background color to blue and clear the screen
        SDL_SetRenderDrawColor(game.renderer, 0, 0, 255, 255);
        SDL_RenderClear(game.renderer);

        // Render the shapes
        staticEntity.render(game.renderer);
        controllableEntity.render(game.renderer);
        movingEntity.render(game.renderer);
        

        // Present the rendered content
        SDL_RenderPresent(game.renderer);

        // ~60 frames per second
        SDL_Delay(16);
        lastTime = currentTime;
    }

    // Clean up and shut down SDL
    SDL_DestroyRenderer(game.renderer);
    SDL_DestroyWindow(game.window);
    SDL_Quit();

    return 0;
}
