#include "CollisionHandler.h"  // Include your header first
#include "Event.h"
#include "structs.h"  // Include structs.h for full definition of Concepts and Game
#include "EventManager.h"
#include "defs.h"

/**
 * Handles collision events.
 * 
 * @param e the event to be processed
 */
// void CollisionHandler::onEvent(const Event& e) {
//     // If the event is a collision event; else, do nothing.
//     if (e.type == "collision") {
//         // Receives the collision code as a parameter.
//         int collisionCode = e.parameters.at("collisionCode").asInt;

//         // Gives a system statement confirming that the event was received.
//         // std::cout << "Collision Event Received."<< std::endl;

//         // Checks the collision code for the correct scenario.
//         if (collisionCode == 1) { // If the collision code is 1.
//             // Set vertical collision and push player out of the entity.
//             concepts->delta = 0;
//             concepts->verticalVel = 0;
//             concepts->c->move(0, -(concepts->c->getRect().y + concepts->c->getRect().h - concepts->hitBL.y));
//         } else if (collisionCode == 2) { // If the collision code is 2.
//             // Set vertical collision and push player out of the entity.
//             concepts->delta = 0;
//             concepts->verticalVel = 0;
//             concepts->c->move(0, -(concepts->c->getRect().y + concepts->c->getRect().h - concepts->hitBR.y));
//         } else if (collisionCode == 3) { // If the collision code is 3.
//             // Causes vertical collision.
//             concepts->delta = 0;
//             concepts->verticalVel = 0;
//             concepts->c->move(0, -(concepts->c->getRect().y + concepts->c->getRect().h - concepts->s->getRect().y));
//         } else if (collisionCode == 4) { // If the collision code is 4.
//             // Causes vertical collision.
//             concepts->delta = 0;
//             concepts->verticalVel = 0;
//             // Enables player movement mimicking the moving entity.
//             concepts->c->move(0, - (concepts->c->getRect().y + concepts->c->getRect().h - concepts->m->getRect().y));
//         } else if (collisionCode == 5) { // If the collision code is 5.
//             // Causes vertical collision.
//             concepts->delta = 0;
//             concepts->verticalVel = 0;
//             // Enables player movement mimicking the moving entity.
//             concepts->c->move(0, - (concepts->c->getRect().y + concepts->c->getRect().h - concepts->v->getRect().y));
//         }

//         // Gives a system statement confirming that the event was completed.
//         // std::cout << "Collision Event Completed."<< std::endl;
//     }
// }

void CollisionHandler::onEvent(const Event& e) {
    if (e.type == "collision") {
        int collisionCode = e.parameters.at("collisionCode").asInt;

        // Access and modify the player's rect directly
        SDL_Rect& playerRect = concepts->c->getRect();

        if (collisionCode == 1) { // Bottom-left collision
            playerRect.y = concepts->hitBL.y - playerRect.h;
        } else if (collisionCode == 2) { // Bottom-right collision
            playerRect.y = concepts->hitBR.y - playerRect.h;
        } else if (collisionCode == 3) { // Top-left collision
            playerRect.y = concepts->hitTL.y + TILE_SIZE; // Example adjustment
        } else if (collisionCode == 4) { // Top-right collision
            playerRect.y = concepts->hitTR.y + TILE_SIZE; // Example adjustment
        }
    }
}
