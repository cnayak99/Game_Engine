// main.cpp
#include <SDL2/SDL.h>
#include <iostream>
#include "sdl_setup.h"
#include "Entity.h"
#include "Rectangle.h"

int main(int argc, char* argv[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!initializeSDL(&window, &renderer)) {
        return 1; // Initialization failed
    }

    bool quit = false;
    SDL_Event e;

    // Define rectangles for the three shapes
    // SDL_Rect staticRect = { 100, 100, 100, 100 };       // Static shape
    // SDL_Rect controllableRect = { 300, 300, 50, 50 };   // Controllable shape
    // SDL_Rect movingRect = { 500, 100, 75, 75 };         // Moving shape

    Entity staticEntity(Rectangle(100,100,100,100),{255,0,0,255});//Static Red Shape
    Entity controllableEntity(Rectangle(300,300,50,50),{0,255,0,255});//Controllable Green Shape 
    Entity movingEntity(Rectangle(100,100,100,100),{0,0,0,255});//Black Moving Shape


    int speed = 5; // Speed for the moving shape

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Move the controllable shape (placeholder logic, will be controlled later)
        // controllableRect.y += 2;
        // if (controllableRect.y > 1080) controllableRect.y = 0;

        controllableEntity.move(0,2);
        if(controllableEntity.getRect().y>1080){
            controllableEntity.move(0,-1080);
        }

        // Move the shape in a continuous pattern (horizontal oscillation)
        // movingRect.x += speed;
        // if (movingRect.x > 1820 || movingRect.x < 100) speed = -speed;
        movingEntity.move(speed, 0);
        if (movingEntity.getRect().x > 1820 || movingEntity.getRect().x < 100) {
            speed = -speed;
        }

        // Set the background color to blue and clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderClear(renderer);

        // // Render the static shape (red)
        // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        // SDL_RenderFillRect(renderer, &staticRect);

        // // Render the controllable shape (green)
        // SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        // SDL_RenderFillRect(renderer, &controllableRect);

        // // Render the moving shape (blue)
        // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black color
        // SDL_RenderFillRect(renderer, &movingRect);

        staticEntity.render(renderer);
        controllableEntity.render(renderer);
        movingEntity.render(renderer);
        
        // Present the rendered content
        SDL_RenderPresent(renderer);

        // Delay to control frame rate (optional)
        SDL_Delay(16); // ~60 frames per second
    }

    // Clean up and shut down SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
