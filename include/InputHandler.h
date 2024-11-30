#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <iostream>
#include "Event.h"
#include "EventHandler.h"  // Include EventHandler first
#include "structs.h"  // Include Concepts and Game structs
#include "EventManager.h"
#include "ReplayManager.h"

// InputHandler class to handle input-related events like key presses.
class InputHandler : public EventHandler {
private:
    Concepts* concepts;  // Pointer to Concepts object
    Game* game;          // Pointer to Game object
    EventManager* eventManager;  // Reference to EventManager
    ReplayManager* replayManager;
    SDL_Renderer* renderer;

public:
    InputHandler(Concepts* c, Game* g, EventManager* em)
        : concepts(c), game(g), eventManager(em){}
    // InputHandler(Concepts* c, Game* g, EventManager* em, ReplayManager* rm, SDL_Renderer* r)
    //     : concepts(c), game(g), eventManager(em), replayManager(rm), renderer(r) {}
    
    void onEvent(const Event& e) override;  // Ensure the signature matches the base class
};

#endif // INPUTHANDLER_H
