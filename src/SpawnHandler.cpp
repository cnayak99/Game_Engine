#include "SpawnHandler.h"  // Include your header first
#include "Event.h"
#include "structs.h"  // Include structs.h for full definition of Concepts and Game
#include "EventManager.h"
#include "defs.h"

/**
 * Handles the event of despawning and respawning the player.
 * 
 * @param e the event to be processed
 */
void SpawnHandler::onEvent(const Event& e) {
    // If the event is a spawn event; else, do nothing.
    if (e.type == "spawn") {
        // Receives the spawn code as a parameter.
        int spawnCode = e.parameters.at("spawnCode").asInt;

        // Gives a system statement confirming that the event was received.
        // std::cout << "Spawn Event Received: Despawn and Respawn."<< std::endl;

        // If the spawn code is 5, the player must despawn and respawn due to a death zone.
        if (spawnCode == 5) {
            // Respawns the player at the spawn object.
            concepts->c->setPosition(concepts->spawn->getRect().x, concepts->spawn->getRect().y);
        } else if (spawnCode == 1) { // Spawn code 1, respawn due to bound one map 1 collision.
            // Respawns the player.
            concepts->c->setPosition(SCREEN_WIDTH - concepts->c->getRect().x - TILE_SIZE, concepts->c->getRect().y);
            if (concepts->c->getRect().x < 0) {
                concepts->c->setPosition(0, concepts->c->getRect().y);
            }
        } else if (spawnCode == 2) { // Spawn code 2, respawn due to bound one map 2 collision.
            // Respawns the player.
            concepts->c->setPosition(SCREEN_WIDTH - concepts->c->getRect().x - TILE_SIZE, concepts->c->getRect().y);
            if (concepts->c->getRect().x > 1856) {
                concepts->c->setPosition(1856, concepts->c->getRect().y);
            }
        } else if (spawnCode == 3) { // Spawn code 3, respawn due to bound two map 1 collision.
            // Respawns the player.
            concepts->c->setPosition(SCREEN_WIDTH - concepts->c->getRect().x - TILE_SIZE, concepts->c->getRect().y);
            if (concepts->c->getRect().x < 0) {
                concepts->c->setPosition(0, concepts->c->getRect().y);
            }
        } else if (spawnCode == 4) { // Spawn code 4, respawn due to bound two map 2 collision.
            // Respawns the player.
            concepts->c->setPosition(SCREEN_WIDTH - concepts->c->getRect().x - TILE_SIZE, concepts->c->getRect().y);
            if (concepts->c->getRect().x > 1856) {
                concepts->c->setPosition(1856, concepts->c->getRect().y);
            }
        }

        // Gives a system statement confirming that the event was completed.
        // std::cout << "Spawn Event Completed: Despawn and Respawn."<< std::endl;
    }
}
