#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <iostream>
#include "Event.h"
#include "EventHandler.h"  // Include EventHandler first
#include "structs.h"  // Include Concepts and Game structs

// InputHandler class to handle input-related events like key presses.
class InputHandler : public EventHandler {
private:
    Concepts* concepts;  // Pointer to Concepts object
    Game* game;          // Pointer to Game object

public:
    InputHandler(Concepts* c, Game* g) : concepts(c), game(g) {}
    void onEvent(const Event& e) override;  // Ensure the signature matches the base class
};

#endif // INPUTHANDLER_H
