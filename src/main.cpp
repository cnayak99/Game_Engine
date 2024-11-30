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

void renderScore(SDL_Renderer* renderer, int tailLength, int scale, int wScale) {
	SDL_Color Black = { 0, 0, 0 };

	// Get the font used for displaying text
	TTF_Font* font = TTF_OpenFont((char*)"/usr/share/fonts/truetype/msttcorefonts/arial.ttf", 10);
	if (font == NULL) {
		cout << "Font loading error" << endl;
		return;
	}

	SDL_Surface* score = TTF_RenderText_Solid(font, (string("Score: ") + to_string(tailLength * 10)).c_str(), Black);
	SDL_Texture* scoreMessage = SDL_CreateTextureFromSurface(renderer, score);
	SDL_Rect scoreRect;
	scoreRect.w = 100;
	scoreRect.h = 30;
	scoreRect.x = ((scale*wScale) / 2) - (scoreRect.w / 2);
	scoreRect.y = 0;
	SDL_RenderCopy(renderer, scoreMessage, NULL, &scoreRect);

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

void gameOver(SDL_Renderer* renderer, SDL_Event event, int scale, int wScale, int tailLength) {
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
	SDL_Rect gameoverRect;
	SDL_Rect retryRect;
	SDL_Rect scoreRect;
	gameoverRect.w = 200;
	gameoverRect.h = 100;
	gameoverRect.x = ((scale*wScale) / 2)-(gameoverRect.w/2);
	gameoverRect.y = ((scale*wScale) / 2)-(gameoverRect.h/2)-50;
	retryRect.w = 300;
	retryRect.h = 50;
	retryRect.x = ((scale*wScale) / 2) - ((retryRect.w / 2));
	retryRect.y = (((scale*wScale) / 2) - ((retryRect.h / 2))+150);
	scoreRect.w = 100;
	scoreRect.h = 30;
	scoreRect.x = ((scale*wScale) / 2) - (scoreRect.w / 2);
	scoreRect.y = 0;
	SDL_RenderCopy(renderer, gameoverMessage, NULL, &gameoverRect);
	SDL_RenderCopy(renderer, retryMessage, NULL, &retryRect);
	SDL_RenderCopy(renderer, scoreMessage, NULL, &scoreRect);

	TTF_CloseFont(font);

	// Show game over screen while space has not been pressed
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

void youWin(SDL_Renderer* renderer, SDL_Event event, int scale, int wScale, int tailLength) {
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
	SDL_Rect gameoverRect;
	SDL_Rect retryRect;
	SDL_Rect scoreRect;
	gameoverRect.w = 200;
	gameoverRect.h = 100;
	gameoverRect.x = ((scale*wScale) / 2) - (gameoverRect.w / 2);
	gameoverRect.y = ((scale*wScale) / 2) - (gameoverRect.h / 2) - 50;
	retryRect.w = 300;
	retryRect.h = 50;
	retryRect.x = ((scale*wScale) / 2) - ((retryRect.w / 2));
	retryRect.y = (((scale*wScale) / 2) - ((retryRect.h / 2)) + 150);
	scoreRect.w = 100;
	scoreRect.h = 30;
	scoreRect.x = ((scale*wScale) / 2) - (scoreRect.w / 2);
	scoreRect.y = 0;
	SDL_RenderCopy(renderer, gameoverMessage, NULL, &gameoverRect);
	SDL_RenderCopy(renderer, retryMessage, NULL, &retryRect);
	SDL_RenderCopy(renderer, scoreMessage, NULL, &scoreRect);

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

	SDL_Event event;

	// This is the player rectangle, set all values to 0
    Entity player(0, 0, 0, 0,{173, 216, 230, 255}, false, 0); // Purple moving shape.
    concepts.player = &player;

	// SDL_Rect player;
	// player.x = 0;
	// player.y = 0;
	// player.h = 0;
	// player.w = 0;

	// tailLength is incremented every time the snake eats food
	// int tailLength = 0;
    concepts.tailLength=0;
	// Vectors for storage of tail block positions
	vector<int> tailX;
	vector<int> tailY;

	// Size of tiles
	// int scale = 24;
	// int wScale = 24;

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
	// SDL_Rect food;
	// food.w = SCALE;
	// food.h = SCALE;
	// food.x = 0;
	// food.y = 0;
	
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
			youWin(game.renderer, event, SCALE, WSCALE, concepts.tailLength);
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

			

		}

        if (!concepts.inputThisFrame) {
            if (!concepts.down && concepts.state[SDL_SCANCODE_UP]) {
                concepts.up = true;
                concepts.left = false;
                concepts.right = false;
                concepts.down = false;
                concepts.inputThisFrame = true;
            }
            else if (!concepts.right && concepts.state[SDL_SCANCODE_LEFT]) {
                concepts.up = false;
                concepts.left = true;
                concepts.right = false;
                concepts.down = false;
                concepts.inputThisFrame = true;
            }
            else if (!concepts.up && concepts.state[SDL_SCANCODE_DOWN]) {
                concepts.up = false;
                concepts.left = false;
                concepts.right = false;
                concepts.down = true;
               concepts.inputThisFrame = true;
            }
            else if (!concepts.left && concepts.state[SDL_SCANCODE_RIGHT]) {
                concepts.up = false;
                concepts.left = false;
                concepts.right = true;
                concepts.down = false;
                concepts.inputThisFrame = true;
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
				gameOver(game.renderer, event, SCALE, WSCALE, concepts.tailLength);
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
			gameOver(game.renderer, event, SCALE, WSCALE, concepts.tailLength);
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

		// Render everything
		renderFood(game.renderer, concepts.food->getRect());
		renderPlayer(game.renderer, player.getRect(), concepts.x, concepts.y, SCALE, tailX, tailY, concepts.tailLength);
		renderScore(game.renderer, concepts.tailLength, SCALE, WSCALE);

		SDL_RenderDrawLine(game.renderer, 0, 0, 0, 24 * 24);
		SDL_RenderDrawLine(game.renderer, 0, 24*24, 24 * 24, 24 * 24);
		SDL_RenderDrawLine(game.renderer, 24*24, 24 * 24, 24*24, 0);
		SDL_RenderDrawLine(game.renderer, 24*24, 0, 0, 0);

		// Put everything on screen
		// Nothing is actually put on screen until this is called
		SDL_RenderPresent(game.renderer);

		// Choose a color and fill the entire window with it, this resets everything before the next frame
		// This also give us the background color
		SDL_SetRenderDrawColor(game.renderer, 105, 105, 105, 255);
		SDL_RenderClear(game.renderer);
	}

	SDL_DestroyWindow(game.window);

	TTF_Quit();

	SDL_Quit();

	return 0;
}