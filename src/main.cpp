// main.cpp
#include <SDL2/SDL.h>
#include <iostream>
#include "sdl_setup.h"
#include "defs.h"
#include "Entity.h"
#include "Rectangle.h"
#include "Physics.h"
#include "Intersect.h"
#include "Timeline.h"    
#include "structs.h"     
#include <zmq.hpp>
#include "json.hpp" 
#include <thread>
#include "Threads.h"
#include "Event.h"
#include "InputHandler.h"
#include "SpawnHandler.h"
#include "CollisionHandler.h"
#include "QuitHandler.h"
#include "EventManager.h"
#include "EventHandler.h"
#include <SDL2/SDL_ttf.h>

using namespace std; 
using json = nlohmann::json;

/**
 * Runs the game.
 * 
 * Use this main function for all Homework 2 Sections except Sections 2 and 3.
 * 
 * References resources and tutorials provided by Professor Card through
 * the "CSC 481-581 HW 1-4.pdf" located beneath the "Homework 1" title in
 * the "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page. These resources can be found in the
 * https://wiki.libsdl.org/SDL2/FrontPage website.
 * 
 * \param argc the count argument
 * \param argv the string argument
 * \returns int 0 if successful, else, unsuccessful
 * 
 * @author Lillie Sharpe
 * @author Chinmay Nayak
 * @author Robbie Martin
 */

void renderPlayer(SDL_Renderer* renderer, SDL_Rect player, int x, int y, int scale, vector<int> tailX, vector<int> tailY, int tailLength) {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	player.w = scale;
	player.h = scale;

	// Gets x and y of all tail blocks and renders them
	for (int i = 0; i < tailLength; i++) {
		player.x = tailX[i];
		player.y = tailY[i];
		SDL_RenderFillRect(renderer, &player);
	}

	player.x = x;
	player.y = y;

	SDL_RenderFillRect(renderer, &player);
}

void renderFood(SDL_Renderer* renderer, SDL_Rect food) {
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_RenderFillRect(renderer, &food);
}

void renderScore(SDL_Renderer* renderer, int tailLength, int scale, int wScale, Concepts concepts) {
	SDL_Color White = { 255,255,255 };

	// Get the font used for displaying text
	TTF_Font* font = TTF_OpenFont((char*)"/usr/share/fonts/truetype/msttcorefonts/arial.ttf", 10);
	if (font == NULL) {
		cout << "Font loading error" << endl;
		return;
	}

	SDL_Surface* score = TTF_RenderText_Solid(font, (string("Score: ") + to_string(tailLength * 10)).c_str(), White);
	SDL_Texture* scoreMessage = SDL_CreateTextureFromSurface(renderer, score);
	Entity scoreRect(((scale*wScale) / 2) - (100 / 2), 0, 100, 30, {255,255,255,255}, false, 0);
	concepts.scoreRect=&scoreRect;

	SDL_RenderCopy(renderer, scoreMessage, NULL, &concepts.scoreRect->getRect());
	TTF_CloseFont(font);
}

bool checkCollision(int foodx, int foody, int playerx, int playery) {

	if (playerx == foodx && playery == foody){
		return true;
	}

	return false;
}

// Get a valid spawn for the food which is not on top of a tail or player block
pair<int, int> getFoodSpawn(vector<int> tailX, vector<int> tailY, int playerX, int playerY, int scale, int wScale, int tailLength) {
	bool valid = false;
	int x = 0;
	int y = 0;
	srand(time(0));
	x = scale * (rand() % wScale);
	y = scale * (rand() % wScale);
	valid = true;

	// Check all tail blocks and player block
	for (int i = 0; i < tailLength; i++) {

		if ((x == tailX[i] && y == tailY[i]) || (x == playerX && y == playerY)) {
			valid = false;
		}

	}

	if (!valid) {
		pair<int, int> foodLoc;
		foodLoc = make_pair(-100, -100);
		return foodLoc;
	}

	pair<int, int> foodLoc;
	foodLoc = make_pair(x, y);

	return foodLoc;
}

void gameOver(SDL_Renderer* renderer, SDL_Event event, int scale, int wScale, int tailLength, Concepts &concepts) {
	SDL_Color Red = { 255, 0, 0 };
	SDL_Color White = { 255, 255, 255 };
	SDL_Color Black = { 0, 0, 0 };

	// Get the font used for displaying text
	TTF_Font* font = TTF_OpenFont((char*)"/usr/share/fonts/truetype/msttcorefonts/arial.ttf", 10);
	if (font == NULL) {
		cout << "Font loading error" << endl;
		return;
	}

	SDL_Surface* gameover = TTF_RenderText_Solid(font, "Game Over", Red);
	SDL_Surface* retry = TTF_RenderText_Solid(font, "Press Enter to retry", White);
	SDL_Surface* score = TTF_RenderText_Solid(font, (string("Score: ") + to_string(tailLength * 10)).c_str(), Black);
	SDL_Texture* gameoverMessage = SDL_CreateTextureFromSurface(renderer, gameover);
	SDL_Texture* retryMessage = SDL_CreateTextureFromSurface(renderer, retry);
	SDL_Texture* scoreMessage = SDL_CreateTextureFromSurface(renderer, score);

	Entity gameoverRect(((scale*wScale) / 2)-(200/2), ((scale*wScale) / 2)-(100/2)-50, 200, 100,{255,255,255,255}, false, 0);
	concepts.gameoverRect=&gameoverRect;
	Entity retryRect(((scale*wScale) / 2) - ((300 / 2)), 
	(((scale*wScale) / 2) - ((50 / 2))+150), 300, 50,
	{255,255,255,255}, false, 0);
	concepts.retryRect=&retryRect;
	Entity scoreRect(((scale*wScale) / 2) - (100 / 2), 0, 100, 30, {255,255,255,255}, false, 0);
	concepts.scoreRect=&scoreRect;

	SDL_RenderCopy(renderer, gameoverMessage, NULL, &concepts.gameoverRect->getRect());
	SDL_RenderCopy(renderer, retryMessage, NULL, &concepts.retryRect->getRect());
	SDL_RenderCopy(renderer, scoreMessage, NULL, &concepts.scoreRect->getRect());

	TTF_CloseFont(font);

	// Show game over screen while space has not been pressed
	while (true) {
		SDL_RenderPresent(renderer);

		if (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT || event.type == SDL_SCANCODE_ESCAPE) {
				exit(0);
			}

			if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
				return;
			}

		}

	}

}

void youWin(SDL_Renderer* renderer, SDL_Event event, int scale, int wScale, int tailLength, Concepts &concepts) {
	SDL_Color Red = { 255, 0, 0 };
	SDL_Color White = { 255, 255, 255 };
	SDL_Color Black = { 0, 0, 0 };
	SDL_Color Yellow = { 255, 255, 0 };

	// Get the font used for displaying text
	TTF_Font* font = TTF_OpenFont((char*)"/usr/share/fonts/truetype/msttcorefonts/arial.ttf", 10);
	if (font == NULL) {
		cout << "Font loading error" << endl;
		return;
	}

	SDL_Surface* gameover = TTF_RenderText_Solid(font, "You won!", Yellow);
	SDL_Surface* retry = TTF_RenderText_Solid(font, "Press Enter to play again", White);
	SDL_Surface* score = TTF_RenderText_Solid(font, (string("Score: ") + to_string(tailLength * 10)).c_str(), Black);
	SDL_Texture* gameoverMessage = SDL_CreateTextureFromSurface(renderer, gameover);
	SDL_Texture* retryMessage = SDL_CreateTextureFromSurface(renderer, retry);
	SDL_Texture* scoreMessage = SDL_CreateTextureFromSurface(renderer, score);

	Entity gameoverRect(((scale*wScale) / 2)-(200/2), ((scale*wScale) / 2)-(100/2)-50, 200, 100,{255,255,255,255}, false, 0);
	concepts.gameoverRect=&gameoverRect;
	Entity retryRect(((scale*wScale) / 2) - ((300 / 2)), (((scale*wScale) / 2) - ((50 / 2))+150), 300, 50,
	{255,255,255,255}, false, 0);
	concepts.retryRect=&retryRect;
	Entity scoreRect(((scale*wScale) / 2) - (100 / 2), 0, 100, 30, {255,255,255,255}, false, 0);
	concepts.scoreRect=&scoreRect;

	SDL_RenderCopy(renderer, gameoverMessage, NULL, &concepts.gameoverRect->getRect());
	SDL_RenderCopy(renderer, retryMessage, NULL, &concepts.retryRect->getRect());
	SDL_RenderCopy(renderer, scoreMessage, NULL, &concepts.scoreRect->getRect());

	TTF_CloseFont(font);

	// Show victory screen while space has not been pressed
	while (true) {
		SDL_RenderPresent(renderer);

		if (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				exit(0);
			}

			if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
				return;
			}

		}

	}

}

int main(int argc, char* argv[]) {

    EventManager eventManager;

    // Create the Game object.
    Game game;
    // Set the window up.
    game.window = nullptr;
    // Set the renderer up.
    game.renderer = nullptr;

    Concepts concepts;
	ReplayManager replayManager(&concepts);
    concepts.quit = false;

    Timeline anchor(nullptr, 1);

	// Init everything so we have everything
	if (!initializeSDL(&game.window, &game.renderer)) {
        return 1; // If unsuccessful, initialization failed.
    }

	// Init TTF and check for any errors
	if (TTF_Init() < 0) {
		cout << "Error: " << TTF_GetError() << endl;
	}

	InputHandler inputHandler(&concepts, &game, &eventManager, &replayManager, game.renderer);
    eventManager.registerListener("input", &inputHandler);

	SpawnHandler spawnHandler(&concepts, &game);
    // Registers the spawn event handler with the event manager.
    eventManager.registerListener("spawn", &spawnHandler);

	QuitHandler quitHandler(&concepts, &game);
    eventManager.registerListener("quit", &quitHandler);

	SDL_Event event;

	// This is the player rectangle, set all values to 0
    Entity player(0, 0, 0, 0,{173, 216, 230, 255}, false, 0); // Purple moving shape.
    concepts.player = &player;


    concepts.tailLength=0;
	// Vectors for storage of tail block positions
	vector<int> tailX;
	vector<int> tailY;

	// Player position variables
	concepts.x = 0;
	concepts.y = 0;
	concepts.prevX = 0;
	concepts.prevY = 0;

	// Movement controls
	concepts.up = false;
	concepts.down = false;
	concepts.right = false;
	concepts.left = false;

	concepts.inputThisFrame = false;
	concepts.redo = false;

	// Food rectangle
	Entity food(0, 0, SCALE, SCALE,{173, 216, 230, 255}, false, 0); 
    concepts.food = &food;
	
	pair<int, int> foodLoc = getFoodSpawn(tailX, tailY, concepts.x, concepts.y, SCALE, WSCALE, concepts.tailLength);
	concepts.food->getRect().x = foodLoc.first;
	concepts.food->getRect().y = foodLoc.second;

    concepts.a = &anchor;
    Timeline timeThreads(&anchor, 1);
	// Show the window with these settings and apply a renderer to it

	float time = SDL_GetTicks() / 100;

	// Main game loop, this constantly runs and keeps everything updated
	while (!concepts.quit) { 
		float newTime = SDL_GetTicks() / 75; //This value (75) is the speed at which the blocks are updated
		float delta = newTime - time;
		time = newTime;
		concepts.inputThisFrame = false;

		// Check win condition, tail needs to fill all tiles
		if (concepts.tailLength >= 575) {
			int64_t currentTimestamp = timeThreads.getTimeline();
                // Create the spawn event.
                Event spawnEvent("spawn", currentTimestamp);
                // Creates a spawn code for the death zone collision scenario.
                Variant spawnCode;
                spawnCode.type = Variant::TYPE_INT;
                spawnCode.asInt = 1;
                spawnEvent.parameters["spawnCode"] = spawnCode;

                eventManager.raiseEvent(spawnEvent);

			// youWin(game.renderer, event, SCALE, WSCALE, concepts.tailLength,concepts);
			concepts.x = 0;
			concepts.y = 0;
			concepts.up = false;
			concepts.left = false;
			concepts.right = false;
			concepts.down = false;
			tailX.clear();
			tailY.clear();
			concepts.tailLength = 0;
			concepts.redo = false;
			foodLoc = getFoodSpawn(tailX, tailY, concepts.x, concepts.y, SCALE, WSCALE, concepts.tailLength);

			if (concepts.food->getRect().x == -100 && concepts.food->getRect().y == -100) {
				concepts.redo = true;
			}

			concepts.food->getRect().x = foodLoc.first;
			concepts.food->getRect().y = foodLoc.second;
		}

		// Controls
        concepts.state = SDL_GetKeyboardState(nullptr);
		if (SDL_PollEvent(&event)) {

			// Simply exit the program when told to
			if (event.type == SDL_QUIT || concepts.state[SDL_SCANCODE_ESCAPE]) {
				concepts.quit = true;
			}
            if (concepts.state[SDL_SCANCODE_P]) { // Unpause game.
                if (concepts.a->isPaused) {
                    concepts.a->unpause();
                }
            }
            if (concepts.state[SDL_SCANCODE_O]) { // Pause game.
                if (!concepts.a->isPaused) {
                    concepts.a->pause();
                }
            }
			if(concepts.state[SDL_SCANCODE_B]){ // Set tic to 0.5 (which is marked with 3).
				timeThreads.setTicks(3);
				printf("Tics set to 0.5.\n");
			}

			// If the player is pressing 'N'.
			if(concepts.state[SDL_SCANCODE_N]){ // Set tic to 1.
				timeThreads.setTicks(1);
				printf("Tics set to 1.\n");
			}

			// If the player is pressing 'M'.
			if(concepts.state[SDL_SCANCODE_M]){ // Set tic to 2.
				timeThreads.setTicks(2);
				printf("Tics set to 2.\n");
			}
			        // **Record Button (R)**
        if (concepts.state[SDL_SCANCODE_R]) {
            int64_t currentTimestamp = timeThreads.getTimeline();
            Event recordEvent("input", currentTimestamp);
            Variant keyCode;
            keyCode.type = Variant::TYPE_INT;
            keyCode.asInt = SDL_SCANCODE_R;  // Key for Record
            recordEvent.parameters["keyCode"] = keyCode;
            eventManager.raiseEvent(recordEvent);  // Raise Record Event
        }

        // **Stop Button (S)**
        if (concepts.state[SDL_SCANCODE_S]) {
            int64_t currentTimestamp = timeThreads.getTimeline();
            Event stopEvent("input", currentTimestamp);
            Variant keyCode;
            keyCode.type = Variant::TYPE_INT;
            keyCode.asInt = SDL_SCANCODE_S;  // Key for Stop
            stopEvent.parameters["keyCode"] = keyCode;
            eventManager.raiseEvent(stopEvent);  // Raise Stop Event
        }

        // **Replay Button (K)**
        if (concepts.state[SDL_SCANCODE_K]) {
            int64_t currentTimestamp = timeThreads.getTimeline();
            Event replayEvent("input", currentTimestamp);
            Variant keyCode;
            keyCode.type = Variant::TYPE_INT;
            keyCode.asInt = SDL_SCANCODE_K;  // Key for Replay
            replayEvent.parameters["keyCode"] = keyCode;
            eventManager.raiseEvent(replayEvent);  // Raise Replay Event
        }
		}

        if (!concepts.inputThisFrame) {
            if (!concepts.down && concepts.state[SDL_SCANCODE_UP]) {
				int64_t currentTimestamp = timeThreads.getTimeline();
				Event inputEvent("input", currentTimestamp);
				Variant keyCode;
				keyCode.type = Variant::TYPE_INT;
				keyCode.asInt = SDL_SCANCODE_UP;
				inputEvent.parameters["keyCode"] = keyCode;
				eventManager.raiseEvent(inputEvent);
			}
            else if (!concepts.right && concepts.state[SDL_SCANCODE_LEFT]) {
				int64_t currentTimestamp = timeThreads.getTimeline();
				Event inputEvent("input", currentTimestamp);
				Variant keyCode;
				keyCode.type = Variant::TYPE_INT;
				keyCode.asInt = SDL_SCANCODE_LEFT;
				inputEvent.parameters["keyCode"] = keyCode;
				eventManager.raiseEvent(inputEvent);
            }
            else if (!concepts.up && concepts.state[SDL_SCANCODE_DOWN]) {
				int64_t currentTimestamp = timeThreads.getTimeline();
				Event inputEvent("input", currentTimestamp);
				Variant keyCode;
				keyCode.type = Variant::TYPE_INT;
				keyCode.asInt = SDL_SCANCODE_DOWN;
				inputEvent.parameters["keyCode"] = keyCode;
				eventManager.raiseEvent(inputEvent);
            }
            else if (!concepts.left && concepts.state[SDL_SCANCODE_RIGHT]) {
				int64_t currentTimestamp = timeThreads.getTimeline();
				Event inputEvent("input", currentTimestamp);
				Variant keyCode;
				keyCode.type = Variant::TYPE_INT;
				keyCode.asInt = SDL_SCANCODE_RIGHT;
				inputEvent.parameters["keyCode"] = keyCode;
				eventManager.raiseEvent(inputEvent);
            }
        }
        
        if (!concepts.a->isPaused) {

			// The previous position of the player block
			concepts.prevX = concepts.x;
			concepts.prevY = concepts.y;

			if (concepts.up) {
				concepts.y -= delta * SCALE;
			}
			else if (concepts.left) {
				concepts.x -= delta * SCALE;
			}
			else if (concepts.right) {
				concepts.x += delta * SCALE;
			}
			else if (concepts.down) {
				concepts.y += delta * SCALE;
			}

			if (concepts.redo == true) {
				concepts.redo = false;
				foodLoc = getFoodSpawn(tailX, tailY, concepts.x, concepts.y, SCALE, WSCALE, concepts.tailLength);
				concepts.food->getRect().x = foodLoc.first;
				concepts.food->getRect().y = foodLoc.second;

				if (concepts.food->getRect().x == -100 && concepts.food->getRect().y == -100) {
					concepts.redo = true;
				}

			}

			// Collision detection, has played collided with food?
			if (checkCollision(concepts.food->getRect().x, concepts.food->getRect().y, concepts.x, concepts.y)) {

				// Spawn new food after it has been eaten
				foodLoc = getFoodSpawn(tailX, tailY, concepts.x, concepts.y, SCALE, WSCALE, concepts.tailLength);
				concepts.food->getRect().x = foodLoc.first;
				concepts.food->getRect().y = foodLoc.second;

				if (concepts.food->getRect().x == -100 && concepts.food->getRect().y == -100) {
					concepts.redo = true;
				}

				concepts.tailLength++;
			}

			// Only runs in the frames where the player block has moved
			if (delta * SCALE == 24) {

				// Update tail size and position
				if (tailX.size() != concepts.tailLength) {
					tailX.push_back(concepts.prevX);
					tailY.push_back(concepts.prevY);
				}

				//Loop through every tail block, move all blocks to the nearest block in front
				//This updates the blocks from end (farthest from player block) to the start (nearest to player block)
				for (int i = 0; i < concepts.tailLength; i++) {

					if (i > 0) {
						tailX[i - 1] = tailX[i];
						tailY[i - 1] = tailY[i];
					}

				}

				if (concepts.tailLength > 0) {
					tailX[concepts.tailLength - 1] = concepts.prevX;
					tailY[concepts.tailLength - 1] = concepts.prevY;
				}

			}
		
			// Game over if player has collided with a tail block, also reset everything
			for (int i = 0; i < concepts.tailLength; i++) {

				if (concepts.x == tailX[i] && concepts.y == tailY[i]) {
					gameOver(game.renderer, event, SCALE, WSCALE, concepts.tailLength, concepts);
					int64_t currentTimestamp = timeThreads.getTimeline();  // Get current time from Timeline
					Event quitEvent("quit", currentTimestamp);
					eventManager.raiseEvent(quitEvent);
					concepts.x = 0;
					concepts.y = 0;
					concepts.up = false;
					concepts.left = false;
					concepts.right = false;
					concepts.down = false;
					tailX.clear();
					tailY.clear();
					concepts.tailLength = 0;
					concepts.redo = false;

					foodLoc = getFoodSpawn(tailX, tailY, concepts.x, concepts.y, SCALE, WSCALE, concepts.tailLength);
					if (concepts.food->getRect().x == -100 && concepts.food->getRect().y == -100) {
						concepts.redo = true;
					}

					concepts.food->getRect().x = foodLoc.first;
					concepts.food->getRect().y = foodLoc.second;
				}

			}

			// Game over if player out of bounds, also resets the game state
			if (concepts.x < 0 || concepts.y < 0 || concepts.x > SCALE * WSCALE - SCALE || concepts.y > SCALE * WSCALE - SCALE) {
				gameOver(game.renderer, event, SCALE, WSCALE, concepts.tailLength, concepts);
				int64_t currentTimestamp = timeThreads.getTimeline();  // Get current time from Timeline
				Event quitEvent("quit", currentTimestamp);
				eventManager.raiseEvent(quitEvent);
				concepts.x = 0;
				concepts.y = 0;
				concepts.up = false;
				concepts.left = false;
				concepts.right = false;
				concepts.down = false;
				tailX.clear();
				tailY.clear();
				concepts.tailLength = 0;
				concepts.redo = false;
				foodLoc = getFoodSpawn(tailX, tailY, concepts.x, concepts.y, SCALE, WSCALE, concepts.tailLength);
				concepts.food->getRect().x = foodLoc.first;
				concepts.food->getRect().y = foodLoc.second;

				if (concepts.food->getRect().x == -100 && concepts.food->getRect().y == -100) {
					concepts.redo = true;
				}

			}
			
    	}

		if (!replayManager.isReplaying) {
		// Render everything
		renderFood(game.renderer, concepts.food->getRect());
		renderPlayer(game.renderer, concepts.player->getRect(), concepts.x, concepts.y, SCALE, tailX, tailY, concepts.tailLength);
		renderScore(game.renderer, concepts.tailLength, SCALE, WSCALE, concepts);
		if (replayManager.isRecording) {  // Record moving object
            replayManager.recordEntity(concepts.food, SDL_GetTicks64(),{},{}, 'F');
            replayManager.recordEntity(concepts.player, SDL_GetTicks64(),tailX, tailY, 'V');
            // replayManager.recordEntity(concepts.c, SDL_GetTicks64(), 'C');
        }
		SDL_RenderDrawLine(game.renderer, 0, 0, 0, 24 * 24);
		SDL_RenderDrawLine(game.renderer, 0, 24*24, 24 * 24, 24 * 24);
		SDL_RenderDrawLine(game.renderer, 24*24, 24 * 24, 24*24, 0);
		SDL_RenderDrawLine(game.renderer, 24*24, 0, 0, 0);

		}else{
			concepts.a->pause();
            replayManager.playReplay(game.renderer);
            concepts.a->unpause();
		}
		// Put everything on screen
		// Nothing is actually put on screen until this is called
		SDL_RenderPresent(game.renderer);

		// Choose a color and fill the entire window with it, this resets everything before the next frame
		// This also give us the background color
		SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
		SDL_RenderClear(game.renderer);

		eventManager.dispatchEvents();
	}

	SDL_DestroyWindow(game.window);

	TTF_Quit();

	SDL_Quit();

	return 0;
}