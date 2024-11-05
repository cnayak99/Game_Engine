#ifndef SPAWNHANDLER_H
#define SPAWNHANDLER_H

#include <iostream>
#include "Event.h"
#include "EventHandler.h"  // Include EventHandler first.
#include "structs.h"  // Include Concepts and Game structs.

/**
 * SpawnHandler class to handle spawn-related events.
 */
class SpawnHandler : public EventHandler {
private:
    Concepts* concepts;  // Pointer to Concepts object
    Game* game;          // Pointer to Game object

public:
    SpawnHandler(Concepts* c, Game* g) : concepts(c), game(g) {}
    void onEvent(const Event& e) override;  // Ensure the signature matches the base class.
};

#endif // SPAWNHANDLER_H