#include "InputHandler.h"  // Include your header first
#include "Event.h"
#include "structs.h"  // Include structs.h for full definition of Concepts and Game
#include "EventManager.h"

void InputHandler::onEvent(const Event& e) {
    if (e.type == "input") {
        int keyCode = e.parameters.at("keyCode").asInt;
        // std::cout << "Input event received: Key code " << keyCode << std::endl;
        if (keyCode == SDL_SCANCODE_DOWN) {
            concepts->c->setColor({255, 255, 0, 0});
        }
        else {
            concepts->c->setColor({0, 255, 0, 0});
        }
        if (keyCode == SDL_SCANCODE_UP) {
            concepts->verticalVel = concepts->thrust;
        } else if (keyCode == SDL_SCANCODE_LEFT) {
            concepts->c->move(-concepts->moveSpeed, 0);
        } else if (keyCode == SDL_SCANCODE_RIGHT) {
            concepts->c->move(concepts->moveSpeed, 0);
        } else if (keyCode == SDL_SCANCODE_C) {
            if (!concepts->held) {
                concepts->held = true;
                if (!concepts->scaling) {
                    SDL_RenderSetLogicalSize(game->renderer, 1920, 1080);
                    concepts->scaling = true;
                } else {
                    SDL_RenderSetLogicalSize(game->renderer, 0, 0);
                    concepts->scaling = false;
                }
            }
        }
    }
}
