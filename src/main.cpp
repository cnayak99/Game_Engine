// main.cpp
#include <SDL2/SDL.h>
#include <iostream>
#include "sdl_setup.h"
#include "Entity.h"
#include "Rectangle.h"
#include "Physics.h"
#include "Intersect.h"
#include "Time.h"
#include "../../CSC481_Team13_P1/include/Physics.h"

/** Constructs the Time object. Tics can be 1, half, or double.*/
Time timeline = Time(Time.anchor, 1); // How to construct anchor address?

/**
 * Creates a Time struct, which keeps track of and monitors a
 * timeline within the game.
 * 
 * This statement is heavily inspired from the example delta time
 * calculation displayed by Professor Card on page 17 in the
 * "S24 05 Timelines.pptx" PowerPoint that is available on the
 * "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page through the "Lecture Notes" link beneath the
 * "General Information and Discussions" subtitle.
 */
int64_t lastT = timeline.getTime();

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
    /**
     * This code section is heavily inspired from the example delta time
     * calculation displayed by Professor Card on page 17 in the
     * "S24 05 Timelines.pptx" PowerPoint that is available on the
     * "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
     * Moodle page through the "Lecture Notes" link beneath the
     * "General Information and Discussions" subtitle.
     * 
     * Start of inspired code.
     */
    int64_t currentT = timeline.getTime();
    int64_t deltaF = currentT - lastT;
    lastT = currentT;
    /** End of inspired code. */

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!initializeSDL(&window, &renderer)) {
        return 1; // Initialization failed
    }

    bool quit = false;

    SDL_Event e;

    Entity staticEntity(Rectangle(100,100,100,100),{255,0,0,255}, false);//Static Red Shape
    Entity controllableEntity(Rectangle(300,300,50,50),{0,255,0,255}, true);//Controllable Green Shape 
    Entity movingEntity(Rectangle(100,100,100,100),{0,0,0,255}, true);//Black Moving Shape
    bool scaling = false;
    bool held = false;
    float gravity = 9.8f;
    // Physics physics(gravity);

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
                verticalVel = 0;
                // Enables player movement mimicking the moving entity.
                controllableEntity.move(speed, static_cast<int>(verticalVel));
                if (controllableEntity.getRect().x > 1820 || controllableEntity.getRect().x < 100) {
                    speed = -speed;
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
                verticalVel = 0;
            }
            // More sides will be added in the future.
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
