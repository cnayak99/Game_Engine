#ifndef COLLISIONHANDLER_H
#define COLLISIONHANDLER_H

#include <iostream>
#include "Event.h"
#include "EventHandler.h"  // Include EventHandler first.
#include "structs.h"  // Include Concepts and Game structs.

/**
 * CollisionHandler class to handle spawn-related events.
 */
class CollisionHandler : public EventHandler {
private:
    Concepts* concepts;  // Pointer to Concepts object
    Game* game;          // Pointer to Game object

public:
    CollisionHandler(Concepts* c, Game* g) : concepts(c), game(g) {}
    void onEvent(const Event& e) override;  // Ensure the signature matches the base class.
};

#endif // COLLISIONHANDLER_H