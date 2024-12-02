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
using namespace std; 

void resetBricks(int& liveCount, float& velY, float& velX, SDL_Rect& paddle, SDL_Rect& ball, std::vector<bool>& bricks) {
    for(int i = 0; i < COL * ROW; i++) bricks[i] = 1;
    liveCount = 3;
    paddle.x = (WIDTH / 2) - (paddle.w / 2);
    ball.y = paddle.y - (paddle.h * 4);
    velY = BALL_SPEED / 2;
    velX = 0;
    ball.x = WIDTH / 2 - (BALL_SIZE / 2);
}

void setBricks(int i, SDL_Rect& brick) {
    brick.x = (((i % COL) + 1) * SPACING) + ((i % COL) * brick.w) - (SPACING / 2);
    brick.y = brick.h * 3 + (((i % ROW) + 1) * SPACING) + ((i % ROW) * brick.h) - (SPACING / 2);
}

void write(std::string text, int x, int y, TTF_Font* font, SDL_Renderer* renderer, SDL_Color color, SDL_Rect& lives) {
    SDL_Surface* surface;
    SDL_Texture* texture;
    std::string fullText = "Lives Remaining: " + text;
    const char* t = fullText.c_str(); 
    surface = TTF_RenderText_Solid(font, +t, color);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    lives.w = surface->w;
    lives.h = surface->h;
    lives.x = x - lives.w + 100;
    lives.y = y - lives.h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &lives);
    SDL_DestroyTexture(texture);
}


int main(int argc, char* argv[]) {
	int frameCount, timerFPS, lastFrame, fps;
	// float velY, velX;
	// int liveCount;
	SDL_Color color;
	// SDL_Rect lives;
	// bool bricks[ROW*COL];
	std::vector<bool> bricks(ROW * COL, true);
	EventManager eventManager;

	// Create the Game object.
	Game game;
	// Set the window up.
	game.window = nullptr;
	// Set the renderer up.
	game.renderer = nullptr;
	game.font = nullptr;

	Concepts concepts;
	concepts.quit = false;
	Timeline anchor(nullptr, 1);

	if (!initializeSDL(&game.window, &game.renderer)) {
	    return 1; // If unsuccessful, initialization failed.
	}
	// if(SDL_Init(SDL_INIT_EVERYTHING) < 0) std::cout << "Failed at SDL_Init()" << std::endl;
	// if(SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) < 0) std::cout << "Failed at SDL_CreateWindowAndRenderer()" << std::endl;
	// SDL_SetWindowTitle(window, "Brick Breaker");
	TTF_Init();
	game.font = TTF_OpenFont((char*)"/usr/share/fonts/truetype/msttcorefonts/arial.ttf", 25);
	static int lastTime=0;
	color.r=color.g=color.b=255;
 	SDL_Color paddleColor = {255, 255, 255, 255}; // White color

    // Create an Entity for the paddle
    Entity paddle(0, HEIGHT - 44, WIDTH / 4, 12, paddleColor, false, 0);
	concepts.paddle = &paddle;

	SDL_Color ballColor = {255, 255, 255, 255}; // White color
    // Create an Entity for the ball
    Entity ball(0, 0, BALL_SIZE, BALL_SIZE, ballColor, false, 0);
	concepts.ball=&ball;
	// ball.w=ball.h=BALL_SIZE;
	Entity brick(0, 0, (WIDTH-(SPACING*COL))/COL, 22, {0,0,0,0}, false, 0);
	concepts.brick=&brick;

	Entity lives(0, 0, 0, 0, {0,0,0,0}, false, 0);
	concepts.lives=&lives;
	// brick.w=(WIDTH-(SPACING*COL))/COL;
	// brick.h=22;

	resetBricks(concepts.liveCount, concepts.velY, concepts.velX, concepts.paddle->getRect(), concepts.ball->getRect(), bricks);
	SDL_Event event;
	concepts.a = &anchor;
    Timeline timeThreads(&anchor, 1);
	ReplayManager replayManager(&concepts);

	InputHandler inputHandler(&concepts, &game, &eventManager, &replayManager, game.renderer);
    eventManager.registerListener("input", &inputHandler);

	CollisionHandler collisionHandler(&concepts, &game, &bricks);
    eventManager.registerListener("collision", &collisionHandler);

	SpawnHandler spawnHandler(&concepts, &game, &bricks);
    eventManager.registerListener("spawn", &spawnHandler);

	while(!concepts.quit) {
		lastFrame=SDL_GetTicks();
		if(lastFrame>=lastTime+1000) {
			lastTime=lastFrame;
			fps=frameCount;
			frameCount=0;
		}

		//handle input
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
			// If the player is pressing 'B'.
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
		

		if (!concepts.a->isPaused) {
		//Update Code
		if(concepts.state[SDL_SCANCODE_LEFT]){
			// concepts.paddle->getRect().x-=SPEED;
			int64_t currentTimestamp = timeThreads.getTimeline();
			Event inputEvent("input", currentTimestamp);
			Variant keyCode;
			keyCode.type = Variant::TYPE_INT;
			keyCode.asInt = SDL_SCANCODE_LEFT;
			inputEvent.parameters["keyCode"] = keyCode;
			eventManager.raiseEvent(inputEvent);
		}
		if(concepts.state[SDL_SCANCODE_RIGHT]){
			// concepts.paddle->getRect().x+=SPEED;
			int64_t currentTimestamp = timeThreads.getTimeline();
			Event inputEvent("input", currentTimestamp);
			Variant keyCode;
			keyCode.type = Variant::TYPE_INT;
			keyCode.asInt = SDL_SCANCODE_RIGHT;
			inputEvent.parameters["keyCode"] = keyCode;
			eventManager.raiseEvent(inputEvent);
		}
		if(concepts.liveCount<=0) {
			// resetBricks(concepts.liveCount, concepts.velY, concepts.velX, concepts.paddle->getRect(), concepts.ball->getRect(), bricks);
			 // Create the current timestamp.
			int64_t currentTimestamp = timeThreads.getTimeline();

			// Create the spawn event.
			Event spawnEvent("spawn", currentTimestamp);

			// Creates a spawn code for the bound two map 1 collision scenario.
			Variant spawnCode;
			spawnCode.type = Variant::TYPE_INT;
			spawnCode.asInt = 1;
			spawnEvent.parameters["spawnCode"] = spawnCode;

			// Raises the respawn event to the event manager.
			eventManager.raiseEvent(spawnEvent);
		}
		if(SDL_HasIntersection(&concepts.ball->getRect(), &concepts.paddle->getRect())) {
			int64_t currentTimestamp = timeThreads.getTimeline();
			// Create the collision event.
			Event collisionEvent("collision", currentTimestamp);
			// Creates a code for the collision scenario.
			Variant collisionCode;
			collisionCode.type = Variant::TYPE_INT;
			collisionCode.asInt = 99;
			collisionEvent.parameters["collisionCode"] = collisionCode;
			// Raises the collision event to the event manager.
			eventManager.raiseEvent(collisionEvent);
		}
		if(concepts.ball->getRect().y<=0) concepts.velY=-concepts.velY;
		if(concepts.ball->getRect().y+BALL_SIZE>=HEIGHT) {concepts.velY=-concepts.velY;concepts.liveCount--;}
		if(concepts.ball->getRect().x<=0 || concepts.ball->getRect().x+BALL_SIZE>=WIDTH) concepts.velX=-concepts.velX;
		concepts.ball->getRect().x+=concepts.velX;
		concepts.ball->getRect().y+=concepts.velY;
		if(concepts.paddle->getRect().x<0) concepts.paddle->getRect().x=0;
		if(concepts.paddle->getRect().x+concepts.paddle->getRect().w>WIDTH) concepts.paddle->getRect().x=WIDTH-concepts.paddle->getRect().w;

		bool reset=1;
		for(int i=0; i<COL*ROW; i++) {
			setBricks(i,concepts.brick->getRect());
			if(SDL_HasIntersection(&concepts.ball->getRect(), &concepts.brick->getRect()) && bricks[i]) {
				int64_t currentTimestamp = timeThreads.getTimeline();
				// Create the collision event.
				Event collisionEvent("collision", currentTimestamp);
				// Creates a code for the collision scenario.
				Variant collisionCode;
				collisionCode.type = Variant::TYPE_INT;
				collisionCode.asInt = i;
				collisionEvent.parameters["collisionCode"] = collisionCode;
				// Raises the collision event to the event manager.
				eventManager.raiseEvent(collisionEvent);
			}
			if(bricks[i]) reset=0;
		}
		if(reset) {
			// resetBricks(concepts.liveCount, concepts.velY, concepts.velX, concepts.paddle->getRect(), concepts.ball->getRect(), bricks);
			int64_t currentTimestamp = timeThreads.getTimeline();

			// Create the spawn event.
			Event spawnEvent("spawn", currentTimestamp);

			// Creates a spawn code for the bound two map 1 collision scenario.
			Variant spawnCode;
			spawnCode.type = Variant::TYPE_INT;
			spawnCode.asInt = 1;
			spawnEvent.parameters["spawnCode"] = spawnCode;

			// Raises the respawn event to the event manager.
			eventManager.raiseEvent(spawnEvent);
		}
		}
		// render 
		if (!replayManager.isReplaying) {

		frameCount++;
		timerFPS=SDL_GetTicks()-lastFrame;
		if(timerFPS<(1000/60)) {
			SDL_Delay((1000/60)-timerFPS);
		}

		SDL_SetRenderDrawColor(game.renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(game.renderer, &concepts.paddle->getRect());
		SDL_RenderFillRect(game.renderer, &concepts.ball->getRect());
		write(std::to_string(concepts.liveCount), WIDTH/2+FONT_SIZE/2, FONT_SIZE*1.5, game.font, game.renderer, color, concepts.lives->getRect());
		for(int i=0; i<COL*ROW; i++) {
			SDL_SetRenderDrawColor(game.renderer, 255, 0, 0, 255);
			if(i%2==0)SDL_SetRenderDrawColor(game.renderer, 0,255, 0, 255);
			if(bricks[i]) {
				setBricks(i,concepts.brick->getRect());
				SDL_RenderFillRect(game.renderer, &concepts.brick->getRect());
			}
		}
		if (replayManager.isRecording) {  // Record moving object
            replayManager.recordEntity(concepts.paddle, SDL_GetTicks64(), bricks,concepts.liveCount, 'P');
			replayManager.recordEntity(concepts.ball, SDL_GetTicks64(), bricks,concepts.liveCount,'B');
			replayManager.recordEntity(concepts.brick, SDL_GetTicks64(), bricks,concepts.liveCount,'V');

            // replayManager.recordEntity(concepts.player, SDL_GetTicks64(),tailX, tailY, 'V');
            // replayManager.recordEntity(concepts.c, SDL_GetTicks64(), 'C');
        }
		}else{
			concepts.a->pause();
            replayManager.playReplay(game.renderer);
            concepts.a->unpause();
		}

		SDL_RenderPresent(game.renderer);
		SDL_SetRenderDrawColor(game.renderer, 0x00, 0x00, 0x00, 255);
		SDL_RenderClear(game.renderer);
		eventManager.dispatchEvents();
	}
	TTF_CloseFont(game.font);
	SDL_DestroyRenderer(game.renderer);
	SDL_DestroyWindow(game.window);
	SDL_Quit();
	TTF_Quit();

	return 0;
}