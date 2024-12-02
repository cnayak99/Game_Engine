#include "InputHandler.h"  // Include your header first
#include "Event.h"
#include "structs.h"  // Include structs.h for full definition of Concepts and Game
#include "EventManager.h"
#include "defs.h"

void InputHandler::onEvent(const Event& e) {
    if (e.type == "input") {
        int keyCode = e.parameters.at("keyCode").asInt;
        if (keyCode == SDL_SCANCODE_R) {
            replayManager->startRecording();
        } else if (keyCode == SDL_SCANCODE_S) {
            replayManager->stopRecording();
        } else if (keyCode == SDL_SCANCODE_K) {
            if (!replayManager->isReplaying) {
                replayManager->startReplay();
            } else {
                replayManager->stopReplay();
            }
        }else if (keyCode == SDL_SCANCODE_LEFT) {
            concepts->paddle->getRect().x-=SPEED;
        } 
        else if (keyCode == SDL_SCANCODE_RIGHT) {
            concepts->paddle->getRect().x+=SPEED;
        } 

    }
}