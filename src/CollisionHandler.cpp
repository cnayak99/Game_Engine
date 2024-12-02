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
void CollisionHandler::onEvent(const Event& e) {
    if (e.type == "collision") {
        int collisionCode = e.parameters.at("collisionCode").asInt;

        // Access and modify the player's rect directly
        // SDL_Rect& playerRect = concepts->c->getRect();

        if (collisionCode == 99) { // Bottom-left collision
            double rel=(concepts->paddle->getRect().x+(concepts->paddle->getRect().w/2))-(concepts->ball->getRect().x+(BALL_SIZE/2));
			double norm=rel/(concepts->paddle->getRect().w/2);
			double bounce = norm* (5*PI/12);
			concepts->velY=-BALL_SPEED*cos(bounce);
			concepts->velX=BALL_SPEED*-sin(bounce);
        }else{
            (*bricks)[collisionCode]=0;
				if(concepts->ball->getRect().x >= concepts->brick->getRect().x) {concepts->velX=-concepts->velX; concepts->ball->getRect().x-=20;}
				if(concepts->ball->getRect().x <= concepts->brick->getRect().x) {concepts->velX=-concepts->velX; concepts->ball->getRect().x+=20;}
				if(concepts->ball->getRect().y <= concepts->brick->getRect().y) {concepts->velY=-concepts->velY; concepts->ball->getRect().y-=20;}
				if(concepts->ball->getRect().y >= concepts->brick->getRect().y) {concepts->velY=-concepts->velY; concepts->ball->getRect().y+=20;}
			
        }

    }
}
