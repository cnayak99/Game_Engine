#ifndef SPAWNHANDLER_H
#define SPAWNHANDLER_H

#include <iostream>
#include "Event.h"
#include "EventHandler.h"  // Include EventHandler first.
#include "structs.h"  // Include Concepts and Game structs.
#include <SDL2/SDL_ttf.h>
/**
 * SpawnHandler class to handle spawn-related events.
 */
class SpawnHandler : public EventHandler {
private:
    Concepts* concepts;  // Pointer to Concepts object
    Game* game;          // Pointer to Game object

public:
    void youWin(SDL_Renderer* renderer, SDL_Event event, int scale, int wScale, int tailLength, Concepts &concepts);
    SpawnHandler(Concepts* c, Game* g) : concepts(c), game(g) {}
    void onEvent(const Event& e) override;  // Ensure the signature matches the base class.
};

#endif // SPAWNHANDLER_H