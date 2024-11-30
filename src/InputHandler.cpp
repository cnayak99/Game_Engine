#include "InputHandler.h"  // Include your header first
#include "Event.h"
#include "structs.h"  // Include structs.h for full definition of Concepts and Game
#include "EventManager.h"

void InputHandler::onEvent(const Event& e) {
    if (e.type == "input") {
        int keyCode = e.parameters.at("keyCode").asInt;
        if (keyCode == SDL_SCANCODE_UP) {
            concepts->up = true;
            concepts->left = false;
            concepts->right = false;
            concepts->down = false;
            concepts->inputThisFrame = true;
        }else if (keyCode == SDL_SCANCODE_LEFT) {
            concepts->up = false;
            concepts->left = true;
            concepts->right = false;
            concepts->down = false;
            concepts->inputThisFrame = true;
        } 
        else if (keyCode == SDL_SCANCODE_RIGHT) {
                concepts->up = false;
                concepts->left = false;
                concepts->right = true;
                concepts->down = false;
                concepts->inputThisFrame = true;
        } else{
            concepts->up = false;
            concepts->left = false;
            concepts->right = false;
            concepts->down = true;
            concepts->inputThisFrame = true;
        }
    }
}