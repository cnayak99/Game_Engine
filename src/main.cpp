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


json parseUpdatedPositions(const std::string& updatedPositions) {
    return json::parse(updatedPositions); // Parse the JSON string into a JSON object
}

json variantToJson(const Variant& variant) {
    json j;
    switch (variant.type) {
        case Variant::TYPE_INT:
            j["type"] = "int";
            j["value"] = variant.asInt;
            break;
        case Variant::TYPE_FLOAT:
            j["type"] = "float";
            j["value"] = variant.asFloat;
            break;
        case Variant::TYPE_STRING:
            j["type"] = "string";
            j["value"] = variant.asString;
            break;
    }
    return j;
}

// Helper function to convert Event to JSON
json eventToJson(const Event& event) {
    json j;
    j["type"] = event.type;
    j["timestamp"] = event.timestamp;

    // Convert each parameter in the event's parameters map
    for (const auto& param : event.parameters) {
        j["parameters"][param.first] = variantToJson(param.second);
    }
    return j;
}

Variant jsonToVariant(const json& j) {
    Variant v;
    
    std::string typeStr = j["type"];
    
    if (typeStr == "int") {
        v.type = Variant::TYPE_INT;
        v.asInt = j["value"];
        
    } else if (typeStr == "float") {
        v.type = Variant::TYPE_FLOAT;
        v.asFloat = j["value"];
        
    } else if (typeStr == "string") {
        v.type = Variant::TYPE_STRING;
        v.asString = j["value"].get<std::string>().c_str();
        
    }
    
    return v;
}

// Helper function to convert JSON back into an Event
Event jsonToEvent(const json& j) {
    
    int64_t timestamp = j["timestamp"];
    
    Event event(j["type"], timestamp);
    
    for (const auto& param : j["parameters"].items()) {
        event.parameters[param.key()] = jsonToVariant(param.value());
        
    }
    
   return event; 
}

void printPositions(const json& positions) {
    for (const auto& position : positions) {
        std::string clientId = position["clientId"];
        std::string clientAddr = position["clientAddr"];
        int x = position["position"]["x"];
        int y = position["position"]["y"];
        cout<< "ClientAddr: "<<clientAddr<< " X: "<< x<<" Y: "<< y<<endl;
    }
}


std::mutex knownAddressesMutex;
/*
This code has been referred from the zeromq official wesite
Links:
1) https://zguide.zeromq.org/docs/chapter3/
2) https://zguide.zeromq.org/docs/chapter5/
3) http://wiki.zeromq.org/tutorials:dealer-and-router
*/

void broadcastPosition(std::unordered_map<std::string, zmq::socket_t>& dealerSockets, const std::string& positionData) {
    std::lock_guard<std::mutex> lock(knownAddressesMutex); // Lock for thread safety
    for (auto& [addr, socket] : dealerSockets) {
        zmq::message_t msg(positionData.size());
        memcpy(msg.data(), positionData.c_str(), positionData.size());

        // Send the message
        socket.send(std::move(msg), zmq::send_flags::none);
    }
}
/*
This code has been referred from the zeromq official wesite
Links:
1) https://zguide.zeromq.org/docs/chapter3/
2) https://zguide.zeromq.org/docs/chapter5/
3) http://wiki.zeromq.org/tutorials:dealer-and-router
*/

// Function to listen for updates and establish connections
void listenForUpdates(zmq::socket_t& subscriberSocket, std::unordered_map<std::string, zmq::socket_t>& dealerSockets, zmq::context_t& context, const std::string& clientAddress, std::unordered_map<std::string, SDL_Rect>& entityPositions, std::mutex& positionMutex, EventManager& eventManager, std::string& clientId) {
    while (true) {
        zmq::message_t update;
        subscriberSocket.recv(update, zmq::recv_flags::none);
        std::string updateStr(static_cast<char*>(update.data()), update.size());

        try {
            auto jsonData = json::parse(updateStr);
            // Check for disconnect notification
            if (jsonData.contains("eventType")&& jsonData.contains("clientId")) {
                std::string receivedClientId = jsonData["clientId"];
                if(receivedClientId == clientId){
                Event receivedEvent = jsonToEvent(jsonData);
                   
                std::cout << "Received Event: " << receivedEvent.type 
                            << ", Timestamp: " << receivedEvent.timestamp 
                            << ", Spawn Code: " << receivedEvent.parameters["spawnCode"].asInt << std::endl;

                int64_t currentTimestamp = receivedEvent.timestamp;
                // Create the spawn event.
                Event spawnEvent("spawn", currentTimestamp);
                // Creates a spawn code for the death zone collision scenario.
                Variant spawnCode;
                spawnCode.type = Variant::TYPE_INT;
                spawnCode.asInt = receivedEvent.parameters["spawnCode"].asInt;
                spawnEvent.parameters["spawnCode"] = spawnCode;

                eventManager.raiseEvent(spawnEvent);
                cout<<"Spawn Event is Raised in the listen function"<<endl;
                continue;
            }else continue;
            }   
            // Check for disconnect notification
            if (jsonData.contains("disconnectedClientId")) {
                std::string disconnectedClientId = jsonData["disconnectedClientId"];
                // Remove from active clients
                std::lock_guard<std::mutex> lock(positionMutex);
                entityPositions.erase(disconnectedClientId);
                continue;
            }

            std::cout << "Received updated client addresses:" << std::endl;
            for (auto& [id, addr] : jsonData.items()) {
                std::cout << id << ": " << addr << std::endl;

                // Ensure addr is treated as a string
                std::string addrStr = addr.get<std::string>();

                if (addrStr == clientAddress) {
                    // Skip connecting to itself
                    continue;
                }

                std::lock_guard<std::mutex> lock(knownAddressesMutex);
                if (dealerSockets.find(addrStr) == dealerSockets.end() && jsonData.size() > 1) {
                    zmq::socket_t dealerSocket(context, ZMQ_DEALER);
                    dealerSocket.setsockopt(ZMQ_IDENTITY, clientAddress.c_str(), clientAddress.size());
                    dealerSocket.connect(addrStr);
                    dealerSockets[addrStr] = std::move(dealerSocket);

                    // Send a connect message with client address
                    std::string connectMessage = "CONNECT:" + clientAddress;
                    zmq::message_t connectMsg(connectMessage.size());
                    memcpy(connectMsg.data(), connectMessage.c_str(), connectMessage.size());
                    dealerSockets[addrStr].send(std::move(connectMsg), zmq::send_flags::none);

                    // Wait for acknowledgment
                    zmq::message_t ack;
                    dealerSockets[addrStr].recv(ack, zmq::recv_flags::none);
                    std::string ackStr(static_cast<char*>(ack.data()), ack.size());
                    if (ackStr == "ACK") {
                        std::cout << "Received acknowledgment from peer at " << addrStr << std::endl;
                    }
                }
            }
        } catch (const json::parse_error& e) {
            std::cerr << "Parse error: " << e.what() << std::endl;
        }
    }
}

/*
This code has been referred from the zeromq official wesite
Links:
1) https://zguide.zeromq.org/docs/chapter3/
2) https://zguide.zeromq.org/docs/chapter5/
3) http://wiki.zeromq.org/tutorials:dealer-and-router
*/
// Function to handle incoming messages from peers
std::unordered_map<std::string, SDL_Rect> entityPositions;
std::mutex positionMutex;

void handleIncomingMessages(zmq::socket_t& routerSocket, std::unordered_map<std::string, std::string>& identityToAddressMap) {
    while (true) {
        zmq::message_t identity;
        zmq::message_t message;

        // Receive identity frame
        routerSocket.recv(identity, zmq::recv_flags::none);
        std::string identityStr(static_cast<char*>(identity.data()), identity.size());

        // Receive message frame
        routerSocket.recv(message, zmq::recv_flags::none);
        std::string msgStr(static_cast<char*>(message.data()), message.size());

        if (msgStr.rfind("CONNECT:", 0) == 0) {
            std::string senderAddress = msgStr.substr(8);
            identityToAddressMap[identityStr] = senderAddress;
            std::cout << "Received connection request from " << senderAddress << ": " << msgStr << std::endl;

            zmq::message_t ackMsg("ACK", 3);
            routerSocket.send(identity, zmq::send_flags::sndmore);
            routerSocket.send(ackMsg, zmq::send_flags::none);
        } else {
            auto it = identityToAddressMap.find(identityStr);
            if (it != identityToAddressMap.end()) {
                try {
                    auto positionData = nlohmann::json::parse(msgStr);
                    std::string clientId = positionData["clientId"];
                    float x = positionData["x"];
                    float y = positionData["y"];

                    // Update entity position in a thread-safe manner
                    std::lock_guard<std::mutex> lock(positionMutex);
                    entityPositions[clientId] = {static_cast<int>(x), static_cast<int>(y), 50, 50}; // Assuming width and height are 50

                } catch (const nlohmann::json::parse_error& e) {
                    std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Error accessing coordinates: " << e.what() << std::endl;
                }
            } else {
                std::cout << "Received message from Unknown: " << msgStr << std::endl;
            }
        }
    }
}

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

void gameOver(SDL_Renderer* renderer, SDL_Event event, int scale, int wScale, Concepts &concepts) {
	SDL_Color Red = { 255, 0, 0 };
	SDL_Color White = { 255, 255, 255 };
	SDL_Color Black = { 0, 0, 0 };

	// Get the font used for displaying text
	TTF_Font* font = TTF_OpenFont((char*)"arial.ttf", 10);
	if (font == NULL) {
		cout << "Font loading error" << endl;
		return;
	}

	SDL_Surface* gameover = TTF_RenderText_Solid(font, "Game Over", Red);
	SDL_Surface* retry = TTF_RenderText_Solid(font, "Press Enter to retry", White);
	// SDL_Surface* score = TTF_RenderText_Solid(font, (string("Score: ") + to_string(tailLength * 10)).c_str(), Black);
	SDL_Texture* gameoverMessage = SDL_CreateTextureFromSurface(renderer, gameover);
	SDL_Texture* retryMessage = SDL_CreateTextureFromSurface(renderer, retry);
	// SDL_Texture* scoreMessage = SDL_CreateTextureFromSurface(renderer, score);

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
	// SDL_RenderCopy(renderer, scoreMessage, NULL, &concepts.scoreRect->getRect());

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
void youWin(SDL_Renderer* renderer, SDL_Event event, int scale, int wScale, Concepts &concepts) {
	SDL_Color Red = { 255, 0, 0 };
	SDL_Color White = { 255, 255, 255 };
	SDL_Color Black = { 0, 0, 0 };
	SDL_Color Yellow = { 255, 255, 0 };

	// Get the font used for displaying text
	TTF_Font* font = TTF_OpenFont((char*)"arial.ttf", 10);
	if (font == NULL) {
		cout << "Font loading error" << endl;
		return;
	}

	SDL_Surface* gameover = TTF_RenderText_Solid(font, "You won!", Yellow);
	SDL_Surface* retry = TTF_RenderText_Solid(font, "Press Enter to play again", White);
	// SDL_Surface* score = TTF_RenderText_Solid(font, (string("Score: ") + to_string(tailLength * 10)).c_str(), Black);
	SDL_Texture* gameoverMessage = SDL_CreateTextureFromSurface(renderer, gameover);
	SDL_Texture* retryMessage = SDL_CreateTextureFromSurface(renderer, retry);
	// SDL_Texture* scoreMessage = SDL_CreateTextureFromSurface(renderer, score);

	Entity gameoverRect(((scale*wScale) / 2)-(200/2), ((scale*wScale) / 2)-(100/2)-50, 200, 100,{255,255,255,255}, false, 0);
	concepts.gameoverRect=&gameoverRect;
	Entity retryRect(((scale*wScale) / 2) - ((300 / 2)), (((scale*wScale) / 2) - ((50 / 2))+150), 300, 50,
	{255,255,255,255}, false, 0);
	concepts.retryRect=&retryRect;
	Entity scoreRect(((scale*wScale) / 2) - (100 / 2), 0, 100, 30, {255,255,255,255}, false, 0);
	concepts.scoreRect=&scoreRect;

	SDL_RenderCopy(renderer, gameoverMessage, NULL, &concepts.gameoverRect->getRect());
	SDL_RenderCopy(renderer, retryMessage, NULL, &concepts.retryRect->getRect());
	// SDL_RenderCopy(renderer, scoreMessage, NULL, &concepts.scoreRect->getRect());

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
	srand(static_cast<unsigned int>(time(0)));
    int randomNum = rand() % 10000; // Generate a random number between 0 and 9999
    std::string clientId = "client" + std::to_string(randomNum);  // Random client ID
    int clientPort = 5560 + randomNum; // Unique port based on client ID
    std::string clientAddress = "tcp://127.0.0.1:" + std::to_string(clientPort);
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

	// Init TTF and check for any errors
	if (TTF_Init() < 0) {
		cout << "Error: " << TTF_GetError() << endl;
	}
	game.font = TTF_OpenFont((char*)"arial.ttf", 25);    
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

	// Create an Entity for the paddle
    Entity paddle(0, HEIGHT - 44, WIDTH / 4, 12, {255, 255, 255, 255}, false, 0);
	concepts.paddle = &paddle;

    // Create an Entity for the ball
    Entity ball(0, 0, BALL_SIZE, BALL_SIZE, {255, 255, 255, 255}, false, 0);
	concepts.ball=&ball;

	Entity brick(0, 0, (WIDTH-(SPACING*COL))/COL, 22, {0,0,0,0}, false, 0);
	concepts.brick=&brick;

	Entity lives(0, 0, 0, 0, {0,0,0,0}, false, 0);
	concepts.lives=&lives;

	resetBricks(concepts.liveCount, concepts.velY, concepts.velX, concepts.paddle->getRect(), concepts.ball->getRect(), bricks);
	
    int64_t lastTime = anchor.getTimeline();

	while(!concepts.quit) {
		int64_t currentTime = anchor.getTimeline();
        // Calculates delta time.
        float deltaTime = 0;
        if (timeThreads.getTicks() != 3) {
            deltaTime = ((currentTime - lastTime) / timeThreads.getTicks()) / 1000.0f;
        } else {
            deltaTime = ((currentTime - lastTime) * 2) / 1000.0f;
        }
        // Stores delta time in concepts.
        concepts.delta = deltaTime;

		// lastFrame=SDL_GetTicks();
		// if(lastFrame>=lastTime+1000) {
		// 	lastTime=lastFrame;
		// 	fps=frameCount;
		// 	frameCount=0;
		// }

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
			gameOver(game.renderer, event, SCALE, WSCALE, concepts);

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
			youWin(game.renderer, event, SCALE, WSCALE, concepts);			
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

		SDL_SetRenderDrawColor(game.renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(game.renderer, &concepts.paddle->getRect());
		SDL_RenderFillRect(game.renderer, &concepts.ball->getRect());
		write(std::to_string(concepts.liveCount), WIDTH/2+FONT_SIZE/2, FONT_SIZE*1.5, game.font, game.renderer, {255,255,255,255}, concepts.lives->getRect());
		for(int i=0; i<COL*ROW; i++) {
			SDL_SetRenderDrawColor(game.renderer, 255, 215, 0, 255);
			if(i%2==0)SDL_SetRenderDrawColor(game.renderer, 0,128, 128, 255);
			if(bricks[i]) {
				setBricks(i,concepts.brick->getRect());
				SDL_RenderFillRect(game.renderer, &concepts.brick->getRect());
			}
		}
		if (replayManager.isRecording) {  // Record moving object
            replayManager.recordEntity(concepts.paddle, SDL_GetTicks64(), bricks,concepts.liveCount, 'P');
			replayManager.recordEntity(concepts.ball, SDL_GetTicks64(), bricks,concepts.liveCount,'B');
			replayManager.recordEntity(concepts.brick, SDL_GetTicks64(), bricks,concepts.liveCount,'V');
        }
		}else{
			concepts.a->pause();
            replayManager.playReplay(game.renderer);
            concepts.a->unpause();
		}

		SDL_RenderPresent(game.renderer);
		SDL_SetRenderDrawColor(game.renderer, 0x00, 0x00, 0x00, 255);
		SDL_RenderClear(game.renderer);
        SDL_Delay(16);
        lastTime = currentTime;
		eventManager.dispatchEvents();
	}
	TTF_CloseFont(game.font);
	SDL_DestroyRenderer(game.renderer);
	SDL_DestroyWindow(game.window);
	SDL_Quit();
	TTF_Quit();

	return 0;
}