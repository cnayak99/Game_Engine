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
int main(int argc, char* argv[]) {
    srand(static_cast<unsigned int>(time(0)));
    int randomNum = rand() % 10000; // Generate a random number between 0 and 9999
    std::string clientId = "client" + std::to_string(randomNum);  // Random client ID
    int clientPort = 5560 + randomNum; // Unique port based on client ID
    std::string clientAddress = "tcp://127.0.0.1:" + std::to_string(clientPort);
    
    EventManager eventManager;

    // Create the Game object.
    Game game;
    // Set the window up.
    game.window = nullptr;
    // Set the renderer up.
    game.renderer = nullptr;

    // Attempt to initialize the game.
    if (!initializeSDL(&game.window, &game.renderer)) {
        return 1; // If unsuccessful, initialization failed.
    }

    // Initialize ZeroMQ context and sockets
    zmq::context_t context(1);
    zmq::socket_t receiver(context, ZMQ_REQ);
    receiver.connect("tcp://127.0.0.1:5555"); // For sending position updates

    zmq::socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect("tcp://127.0.0.1:5556"); // For receiving position updates
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0); // Subscribe to all messages

    zmq::socket_t routerSocket(context, ZMQ_ROUTER);
    routerSocket.bind(clientAddress);

    // Map to store DEALER sockets by address
    std::unordered_map<std::string, zmq::socket_t> dealerSockets;

    // Map to track identities and their corresponding addresses
    std::unordered_map<std::string, std::string> identityToAddressMap;

    // Start thread for listening for server updates
    std::thread updateListener(
        listenForUpdates,
        std::ref(subscriber),
        std::ref(dealerSockets),
        std::ref(context),
        std::cref(clientAddress),
        std::ref(entityPositions), // Pass by reference
        std::ref(positionMutex),    // Pass by reference
        std::ref(eventManager),
        std::ref(clientId)
    );
    updateListener.detach();

    // Start thread to handle incoming messages
    std::thread incomingHandler(handleIncomingMessages,std::ref(routerSocket),std::ref(identityToAddressMap));
    incomingHandler.detach();

    // Create the Concepts object.
    Concepts concepts;
    ReplayManager replayManager(&concepts);  // Create an instance of ReplayManager

    // Initialize quit to false.
    concepts.quit = false;

    //Register Events here
    InputHandler inputHandler(&concepts, &game, &eventManager, &replayManager, game.renderer);
    eventManager.registerListener("input", &inputHandler);

    // Initializes the spawn event handler.
    SpawnHandler spawnHandler(&concepts, &game);
    // Registers the spawn event handler with the event manager.
    eventManager.registerListener("spawn", &spawnHandler);

    // Initializes the collision event handler.
    CollisionHandler collisionHandler(&concepts, &game);
    // Registers the collision event handler with the event manager.
    eventManager.registerListener("collision", &collisionHandler);

    QuitHandler quitHandler(&concepts, &receiver, clientId);
    eventManager.registerListener("quit", &quitHandler);
    // Create an event object.
    SDL_Event e;

    // Construct the anchor timeline.
    Timeline anchor(nullptr, 1);

    int terrain01[MAP_HEIGHT][MAP_WIDTH] = 
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1}};

    int terrain02[MAP_HEIGHT][MAP_WIDTH] = 
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2}};

    Entity * tileMap[MAP_WIDTH][MAP_HEIGHT];

    // Keeps track of the current map tileset.
    int map = 1;

    for (int j = 0; j < MAP_HEIGHT; j++) {
        for (int i = 0; i < MAP_WIDTH; i++) {
            switch (terrain01[j][i])
            {
            case 0:
                concepts.tileMap[i][j] = nullptr;
                break;
            case 1:
                concepts.tileMap[i][j] = new Entity(TILE_SIZE * i, TILE_SIZE * j, TILE_SIZE, TILE_SIZE,{255,0,0,255}, false, 0);
                break;
            case 2:
                concepts.tileMap[i][j] = new Entity(TILE_SIZE * i, TILE_SIZE * j, TILE_SIZE, TILE_SIZE,{200,150,100,255}, false, 0);
                break;
            default:
                break;
            }
        }
    }

    // Creates the spawn shape and connects its address to concepts.
    Entity spawnEntity(635, 320, 64, 64,{0,0,255,255}, false, 1); // Invisible.
    concepts.spawn = &spawnEntity;

    // Creates the static red shape and connects its address to concepts.
    Entity staticEntity(128, 128, 64, 64,{255,0,0,255}, false, 0); // Static red shape.
    concepts.s = &staticEntity;

    // Creates the controllable green shape and connects its address to concepts.
    Entity controllableEntity(concepts.spawn->getRect().x, concepts.spawn->getRect().y, 64, 64,{0,255,0,255}, true, 0); // Controllable green shape.
    concepts.c = &controllableEntity;
    // PlayerHandler* playerEvents = new PlayerHandler(&controllableEntity);

    // Creates the moving black shape and connects its address to concepts.
    Entity movingEntity(1000, 400, 64, 64,{128, 128, 128, 255}, false, 0); // Black moving shape.
    concepts.m = &movingEntity;

    // Creates the moving purple shape and connects its address to concepts.
    Entity movingVertEntity(1100, 200, 64, 64,{173, 216, 230, 255}, false, 0); // Purple moving shape.
    concepts.v = &movingVertEntity;

    // Creates the despawn shape and connects its address to concepts.
    Entity despawnEntity(0, 32, 1920, 64,{0,0,255,255}, false, 2); // Invisible.
    concepts.despawn = &despawnEntity;

    // Creates the first bound shape.
    Entity boundEntityOne(-64, 0, 64, 1080,{0,0,255,0}, false, 3); // Invisible.

    // Creates the second bound shape.
    Entity boundEntityTwo(1920, 0, 64, 1080,{0,0,255,0}, false, 3); // Invisible.

    // Initializes scaling and held through concepts.
    concepts.scaling = false;
    concepts.held = false;

    // Initializes physics variables.
    concepts.gravity = 9.8f;
    concepts.speed = 5; // Speed of the Entity.
    concepts.verticalVel = 0.0f;
    concepts.thrust = -9.8f;
    bool latch = false; // Boolean used in scrolling mechanism
    bool swap = false; // Boolean used in changing the scrolling mechanism

    // Stores the variable that determines whether or not the game is paused.
    concepts.a = &anchor;

    // Create a timeline to run threads.
    Timeline timeThreads(&anchor, 1); // Set tic to whatever is desired.

    // Sets the last time.
    int64_t lastTime = anchor.getTimeline();

    // Runs the game.
    while (!concepts.quit) {
        // Gets the current time.
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

        // Checks if the user is quitting.
        while (SDL_PollEvent(&e) != 0) {
            // If the user is quitting, quit the game.
            if (e.type == SDL_QUIT) {
                concepts.quit = true;
            }
        }
        
        // Stores the keyboard state in concepts.
        concepts.state = SDL_GetKeyboardState(nullptr);

        // Stores the move speed in concepts.
        concepts.moveSpeed = 5;

        // If the player is pressing 'O'.
        if (concepts.state[SDL_SCANCODE_O]) { // Pause game.
            if (!concepts.a->isPaused) {
                concepts.a->pause();
            }
        }

        // If the player is pressing 'P'.
        if (concepts.state[SDL_SCANCODE_P]) { // Unpause game.
            if (concepts.a->isPaused) {
                concepts.a->unpause();
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

        if (!concepts.a->isPaused) {

            // Run threads.
            startThreads(&timeThreads, &concepts, &game, receiver, clientId, eventManager);

            // Keeps track of the controllable rectangle.
            SDL_Rect cRect = concepts.c->getRect();
            // Keeps track of the static rectangle.
            SDL_Rect sRect = concepts.s->getRect();
            // Keeps track of the moving rectangle.
            SDL_Rect mRect = concepts.m->getRect();
            // Keeps track of the vertically moving rectangle.
            SDL_Rect vRect = concepts.v->getRect();

            // Keeps track of the spawn rectangle.
            SDL_Rect spawnRect = concepts.spawn->getRect();
            // Keeps track of the despawn rectangle.
            SDL_Rect despawnRect = concepts.despawn->getRect();

            // Keeps track of the spawn rectangle.
            SDL_Rect boundOne = boundEntityOne.getRect();
            // Keeps track of the despawn rectangle.
            SDL_Rect boundTwo = boundEntityTwo.getRect();

            // The top left corner's map coordinates
            int mapPlayerTL[2] = {floor(cRect.x / TILE_SIZE), floor(cRect.y / TILE_SIZE)};
            // The top right corner's map coordinates
            int mapPlayerTR[2] = {floor((cRect.x + cRect.w) / TILE_SIZE), floor(cRect.y / TILE_SIZE)};
            // The bottom left corner's map coordinates
            int mapPlayerBL[2] = {floor(cRect.x / TILE_SIZE), floor((cRect.y + cRect.h) / TILE_SIZE)};
            // The bottom right corner's map coordinates
            int mapPlayerBR[2] = {floor((cRect.x + cRect.w) / TILE_SIZE), floor((cRect.y + cRect.h) / TILE_SIZE)};
            // Rectangle of the entity (if any) at the bottom-left corner

            // Get the coordinates of the bottom corners relative to the size of the map in tiles (if the player is within the game bounds)
            if (cRect.x + cRect.w >= 0 && cRect.x < SCREEN_WIDTH && cRect.y + cRect.h >= 0 && cRect.y < SCREEN_HEIGHT) {
                // If the entity exists at the coordinates...
                if (cRect.x >= 0 && concepts.tileMap[mapPlayerBL[0]][mapPlayerBL[1]] != nullptr) {
                    // Store the rectangle of the captured entity
                    SDL_Rect hitBL = concepts.tileMap[mapPlayerBL[0]][mapPlayerBL[1]]->getRect();
                    concepts.hitBL = hitBL;
                    // Check if the target is intersecting
                    if (hasIntersection(&cRect, &hitBL) == true) {
                        // Create the current timestamp.
                        int64_t currentTimestamp = timeThreads.getTimeline();

                        // Create the collision event.
                        Event collisionEvent("collision", currentTimestamp);

                        // Creates a code for the collision scenario.
                        Variant collisionCode;
                        collisionCode.type = Variant::TYPE_INT;
                        collisionCode.asInt = 1;
                        collisionEvent.parameters["collisionCode"] = collisionCode;

                        // Reports that a collision event has been initialized.
                        // std::cout << "COLLISION INITIALIZED" << std::endl;

                        // Raises the collision event to the event manager.
                        eventManager.raiseEvent(collisionEvent);
                    }
                }
                // Rectangle of the entity (if any) at the bottom-right corner
                //If the entity exists at the coordinates...
                if (cRect.x + cRect.w < SCREEN_WIDTH && concepts.tileMap[mapPlayerBR[0]][mapPlayerBR[1]] != nullptr) {
                    // Store the rectangle of the captured entity
                    SDL_Rect hitBR = concepts.tileMap[mapPlayerBR[0]][mapPlayerBR[1]]->getRect();
                    concepts.hitBR = hitBR;
                    // Check if the target is intersecting
                    if (hasIntersection(&cRect, &hitBR) == true) {
                        // Create the current timestamp.
                        int64_t currentTimestamp = timeThreads.getTimeline();

                        // Create the collision event.
                        Event collisionEvent("collision", currentTimestamp);

                        // Creates a code for the collision scenario.
                        Variant collisionCode;
                        collisionCode.type = Variant::TYPE_INT;
                        collisionCode.asInt = 2;
                        collisionEvent.parameters["collisionCode"] = collisionCode;

                        // Reports that a collision event has been initialized.
                        // std::cout << "COLLISION INITIALIZED" << std::endl;

                        // Raises the collision event to the event manager.
                        eventManager.raiseEvent(collisionEvent);
                    }
                }
            }

            // Senses other shapes for collision.
            if (hasIntersection(&cRect, &sRect) == true) {
                // If there was an intersection on the top of the terrain rectangle,
                // the controllable rectangle lands on the terrain rectangle.
                if (intersect(&cRect, &sRect) == 2 || intersect(&cRect, &sRect) == 4) {
                    // Create the current timestamp.
                    int64_t currentTimestamp = timeThreads.getTimeline();

                    // Create the collision event.
                    Event collisionEvent("collision", currentTimestamp);

                    // Creates a code for the collision scenario.
                    Variant collisionCode;
                    collisionCode.type = Variant::TYPE_INT;
                    collisionCode.asInt = 3;
                    collisionEvent.parameters["collisionCode"] = collisionCode;

                    // Reports that a collision event has been initialized.
                    // std::cout << "COLLISION INITIALIZED" << std::endl;

                    // Raises the collision event to the event manager.
                    eventManager.raiseEvent(collisionEvent);
                }
                // More sides will be added in the future.
            }

            // Senses other shapes for collision.
            if (hasIntersection(&cRect, &mRect) == true) {
                // If there was an intersection on the top of the terrain rectangle,
                // the controllable rectangle lands on the terrain rectangle.
                if (intersect(&cRect, &mRect) == 2 || intersect(&cRect, &mRect) == 4) {
                    // Create the current timestamp.
                    int64_t currentTimestamp = timeThreads.getTimeline();

                    // Create the collision event.
                    Event collisionEvent("collision", currentTimestamp);

                    // Creates a code for the collision scenario.
                    Variant collisionCode;
                    collisionCode.type = Variant::TYPE_INT;
                    collisionCode.asInt = 4;
                    collisionEvent.parameters["collisionCode"] = collisionCode;

                    // Reports that a collision event has been initialized.
                    // std::cout << "COLLISION INITIALIZED" << std::endl;

                    // Raises the collision event to the event manager.
                    eventManager.raiseEvent(collisionEvent);
                }
                // More sides may be added in the future.
            }

            // Senses other shapes for collision.
            if (hasIntersection(&cRect, &vRect) == true) {
                // If there was an intersection on the top of the terrain rectangle,
                // the controllable rectangle lands on the terrain rectangle.
                if (intersect(&cRect, &vRect) == 2 || intersect(&cRect, &vRect) == 4) {
                    // Create the current timestamp.
                    int64_t currentTimestamp = timeThreads.getTimeline();

                    // Create the collision event.
                    Event collisionEvent("collision", currentTimestamp);

                    // Creates a code for the collision scenario.
                    Variant collisionCode;
                    collisionCode.type = Variant::TYPE_INT;
                    collisionCode.asInt = 5;
                    collisionEvent.parameters["collisionCode"] = collisionCode;

                    // Reports that a collision event has been initialized.
                    // std::cout << "COLLISION INITIALIZED" << std::endl;

                    // Raises the collision event to the event manager.
                    eventManager.raiseEvent(collisionEvent);
                }
                // More sides may be added in the future.
            }

            // Senses other shapes for collision.
            if (hasIntersection(&cRect, &despawnRect) == true) {
                // If there was an intersection on the top of the terrain rectangle,
                // the controllable rectangle lands on the terrain rectangle.
                if (intersect(&cRect, &despawnRect) == 1 || intersect(&cRect, &despawnRect) == 2 ||
                    intersect(&cRect, &despawnRect) == 3 || intersect(&cRect, &despawnRect) == 4) {
                    // Create the current timestamp.
                    int64_t currentTimestamp = timeThreads.getTimeline();

                    // Create the spawn event.
                    Event spawnEvent("spawn", currentTimestamp);

                    // Creates a spawn code for the death zone collision scenario.
                    Variant spawnCode;
                    spawnCode.type = Variant::TYPE_INT;
                    spawnCode.asInt = 5;
                    spawnEvent.parameters["spawnCode"] = spawnCode;

                    // Reports that a respawn event has been initialized.
                    std::cout << "RESPAWN INITIALIZED 6" << std::endl;
                    // json spawnMessage = {
                    //     {"eventType", "spawn"},
                    //     {"timestamp", currentTimestamp},
                    //     {"parameters", {
                    //         {"keyCode", SDL_SCANCODE_UP}
                    //     }},
                    //     {"clientId", clientId},
                    // };
                    json spawnMessage = eventToJson(spawnEvent);
                    spawnMessage["clientId"] = clientId;
                    spawnMessage["eventType"] = true;
                    std::string messageString = spawnMessage.dump();
                    zmq::message_t message(messageString.size());
                    memcpy(message.data(), messageString.c_str(), messageString.size());
                    receiver.send(message, zmq::send_flags::none);
                    

                    // Raises the respawn event to the event manager.
                    // eventManager.raiseEvent(spawnEvent);
                    zmq::message_t reply;
                    receiver.recv(reply, zmq::recv_flags::none);
                }
                // More sides will be added in the future.
            }

            // Senses other shapes for collision.
            if (hasIntersection(&cRect, &boundOne) == true && !latch) {
                latch = true;
                // If there was an intersection against the rectangle,
                // the controllable rectangle intersects the rectangle.
                if (intersect(&cRect, &boundOne) == 1 || intersect(&cRect, &boundOne) == 2 ||
                    intersect(&cRect, &boundOne) == 3 || intersect(&cRect, &boundOne) == 4) {
                    // If this is map 1, reset the map to map 2.
                    if (map == 1) {
                        map = 2;
                        for (int j = 0; j < MAP_HEIGHT; j++) {
                            for (int i = 0; i < MAP_WIDTH; i++) {
                                if (terrain02[j][i] == 0) {
                                    concepts.tileMap[i][j] = nullptr;
                                }
                                else if (terrain02[j][i] == 1) {
                                    concepts.tileMap[i][j] = new Entity(TILE_SIZE * i, TILE_SIZE * j, TILE_SIZE, TILE_SIZE,{255,0,0,255}, false, 0);
                                }
                                else if (terrain02[j][i] == 2) {
                                    concepts.tileMap[i][j] = new Entity(TILE_SIZE * i, TILE_SIZE * j, TILE_SIZE, TILE_SIZE,{200,150,100,255}, false, 0);
                                }
                                else if (terrain02[j][i] == 3) {
                                    concepts.tileMap[i][j] = new Entity(TILE_SIZE * i, TILE_SIZE * j, TILE_SIZE, TILE_SIZE,{150,0,255,255}, false, 0);
                                }
                            }
                        }
                        // Create the current timestamp.
                        int64_t currentTimestamp = timeThreads.getTimeline();

                        // Create the spawn event.
                        Event spawnEvent("spawn", currentTimestamp);

                        // Creates a spawn code for the bound one map 1 collision scenario.
                        Variant spawnCode;
                        spawnCode.type = Variant::TYPE_INT;
                        spawnCode.asInt = 1;
                        spawnEvent.parameters["spawnCode"] = spawnCode;

                        // Reports that a respawn event has been initialized.
                        std::cout << "BOUND ONE (MAP 1) SPAWN INITIALIZED" << std::endl;
                        std::cout << "RESPAWN INITIALIZED 7" << std::endl;

                        // Raises the respawn event to the event manager.
                        // eventManager.raiseEvent(spawnEvent);
                        json spawnMessage = eventToJson(spawnEvent);
                        spawnMessage["clientId"] = clientId;
                        spawnMessage["eventType"] = true;
                        std::string messageString = spawnMessage.dump();
                        zmq::message_t message(messageString.size());
                        memcpy(message.data(), messageString.c_str(), messageString.size());
                        receiver.send(message, zmq::send_flags::none);
                        

                        // Raises the respawn event to the event manager.
                        // eventManager.raiseEvent(spawnEvent);
                        zmq::message_t reply;
                        receiver.recv(reply, zmq::recv_flags::none);
                    }
                    // If this is map 2, reset the map to map 1.
                    else if (map == 2) {
                        map = 1;
                        for (int j = 0; j < MAP_HEIGHT; j++) {
                            for (int i = 0; i < MAP_WIDTH; i++) {
                                if (terrain01[j][i] == 0) {
                                    concepts.tileMap[i][j] = nullptr;
                                }
                                else if (terrain01[j][i] == 1) {
                                    concepts.tileMap[i][j] = new Entity(TILE_SIZE * i, TILE_SIZE * j, TILE_SIZE, TILE_SIZE,{255,0,0,255}, false, 0);
                                }
                                else if (terrain01[j][i] == 2) {
                                    concepts.tileMap[i][j] = new Entity(TILE_SIZE * i, TILE_SIZE * j, TILE_SIZE, TILE_SIZE,{200,150,100,255}, false, 0);
                                }
                                else if (terrain01[j][i] == 3) {
                                    concepts.tileMap[i][j] = new Entity(TILE_SIZE * i, TILE_SIZE * j, TILE_SIZE, TILE_SIZE,{150,0,255,255}, false, 0);
                                }
                            }
                        }
                        // Create the current timestamp.
                        int64_t currentTimestamp = timeThreads.getTimeline();

                        // Create the spawn event.
                        Event spawnEvent("spawn", currentTimestamp);

                        // Creates a spawn code for the bound one map 2 collision scenario.
                        Variant spawnCode;
                        spawnCode.type = Variant::TYPE_INT;
                        spawnCode.asInt = 2;
                        spawnEvent.parameters["spawnCode"] = spawnCode;

                        // Reports that a respawn event has been initialized.
                        std::cout << "BOUND ONE (MAP 2) SPAWN INITIALIZED" << std::endl;
                        std::cout << "RESPAWN INITIALIZED 8" << std::endl;

                        // Raises the respawn event to the event manager.
                        // eventManager.raiseEvent(spawnEvent);
                        json spawnMessage = eventToJson(spawnEvent);
                        spawnMessage["clientId"] = clientId;
                        spawnMessage["eventType"] = true;
                        std::string messageString = spawnMessage.dump();
                        zmq::message_t message(messageString.size());
                        memcpy(message.data(), messageString.c_str(), messageString.size());
                        receiver.send(message, zmq::send_flags::none);
                        

                        // Raises the respawn event to the event manager.
                        // eventManager.raiseEvent(spawnEvent);
                        zmq::message_t reply;
                        receiver.recv(reply, zmq::recv_flags::none);
                    }
                }
                // More sides will be added in the future.
            }
            // Senses other shapes for collision.
            else if (hasIntersection(&cRect, &boundTwo) == true && !latch) {
                latch = true;
                // If there was an intersection against the rectangle,
                // the controllable rectangle intersects the rectangle.
                if (intersect(&cRect, &boundTwo) == 1 || intersect(&cRect, &boundTwo) == 2 ||
                    intersect(&cRect, &boundTwo) == 3 || intersect(&cRect, &boundTwo) == 4) {
                    // If this is map 1, reset the map to map 2.
                    if (map == 1) {
                        map = 2;
                        for (int j = 0; j < MAP_HEIGHT; j++) {
                            for (int i = 0; i < MAP_WIDTH; i++) {
                                if (terrain02[j][i] == 0) {
                                    concepts.tileMap[i][j] = nullptr;
                                }
                                else if (terrain02[j][i] == 1) {
                                    concepts.tileMap[i][j] = new Entity(TILE_SIZE * i, TILE_SIZE * j, TILE_SIZE, TILE_SIZE,{255,0,0,255}, false, 0);
                                }
                                else if (terrain02[j][i] == 2) {
                                    concepts.tileMap[i][j] = new Entity(TILE_SIZE * i, TILE_SIZE * j, TILE_SIZE, TILE_SIZE,{200,150,100,255}, false, 0);
                                }
                                else if (terrain02[j][i] == 3) {
                                    concepts.tileMap[i][j] = new Entity(TILE_SIZE * i, TILE_SIZE * j, TILE_SIZE, TILE_SIZE,{150,0,255,255}, false, 0);
                                }
                            }
                        }
                        // Create the current timestamp.
                        int64_t currentTimestamp = timeThreads.getTimeline();

                        // Create the spawn event.
                        Event spawnEvent("spawn", currentTimestamp);

                        // Creates a spawn code for the bound two map 1 collision scenario.
                        Variant spawnCode;
                        spawnCode.type = Variant::TYPE_INT;
                        spawnCode.asInt = 3;
                        spawnEvent.parameters["spawnCode"] = spawnCode;

                        // Reports that a respawn event has been initialized.
                        std::cout << "BOUND TWO (MAP 1) SPAWN INITIALIZED" << std::endl;
                        std::cout << "RESPAWN INITIALIZED 9" << std::endl;

                        // Raises the respawn event to the event manager.
                        // eventManager.raiseEvent(spawnEvent);
                        json spawnMessage = eventToJson(spawnEvent);
                        spawnMessage["clientId"] = clientId;
                        spawnMessage["eventType"] = true;
                        std::string messageString = spawnMessage.dump();
                        zmq::message_t message(messageString.size());
                        memcpy(message.data(), messageString.c_str(), messageString.size());
                        receiver.send(message, zmq::send_flags::none);
                        

                        // Raises the respawn event to the event manager.
                        // eventManager.raiseEvent(spawnEvent);
                        zmq::message_t reply;
                        receiver.recv(reply, zmq::recv_flags::none);
                    }
                    // If this is map 2, reset the map to map 1.
                    else if (map == 2) {
                        map = 1;
                        for (int j = 0; j < MAP_HEIGHT; j++) {
                            for (int i = 0; i < MAP_WIDTH; i++) {
                                if (terrain01[j][i] == 0) {
                                    concepts.tileMap[i][j] = nullptr;
                                }
                                else if (terrain01[j][i] == 1) {
                                    concepts.tileMap[i][j] = new Entity(TILE_SIZE * i, TILE_SIZE * j, TILE_SIZE, TILE_SIZE,{255,0,0,255}, false, 0);
                                }
                                else if (terrain01[j][i] == 2) {
                                    concepts.tileMap[i][j] = new Entity(TILE_SIZE * i, TILE_SIZE * j, TILE_SIZE, TILE_SIZE,{200,150,100,255}, false, 0);
                                }
                                else if (terrain01[j][i] == 3) {
                                    concepts.tileMap[i][j] = new Entity(TILE_SIZE * i, TILE_SIZE * j, TILE_SIZE, TILE_SIZE,{150,0,255,255}, false, 0);
                                }
                            }
                        }
                        // Create the current timestamp.
                        int64_t currentTimestamp = timeThreads.getTimeline();

                        // Create the spawn event.
                        Event spawnEvent("spawn", currentTimestamp);

                        // Creates a spawn code for the bound two map 2 collision scenario.
                        Variant spawnCode;
                        spawnCode.type = Variant::TYPE_INT;
                        spawnCode.asInt = 4;
                        spawnEvent.parameters["spawnCode"] = spawnCode;

                        // Reports that a respawn event has been initialized.
                        std::cout << "BOUND TWO (MAP 2) SPAWN INITIALIZED" << std::endl;
                        std::cout << "RESPAWN INITIALIZED 10" << std::endl;

                        // Raises the respawn event to the event manager.
                        // eventManager.raiseEvent(spawnEvent);
                        json spawnMessage = eventToJson(spawnEvent);
                        spawnMessage["clientId"] = clientId;
                        spawnMessage["eventType"] = true;
                        std::string messageString = spawnMessage.dump();
                        zmq::message_t message(messageString.size());
                        memcpy(message.data(), messageString.c_str(), messageString.size());
                        receiver.send(message, zmq::send_flags::none);
                        

                        // Raises the respawn event to the event manager.
                        // eventManager.raiseEvent(spawnEvent);
                        zmq::message_t reply;
                        receiver.recv(reply, zmq::recv_flags::none);
                    }
                }
                // More sides will be added in the future.
            }
            else if (!(hasIntersection(&cRect, &boundOne) || hasIntersection(&cRect, &boundTwo))) {
                latch = false;
            }
        }
        json jsonString = {
            {"clientId", clientId},
            {"clientAddr", clientAddress},
            {"entities", json::array({
                {{"type", "horizontal"}, {"x", concepts.m->getRect().x}, {"y", concepts.m->getRect().y}},
                {{"type", "vertical"}, {"x", concepts.v->getRect().x}, {"y", concepts.v->getRect().y}}
            })}
        };


        std::string positionData = jsonString.dump();
        zmq::message_t message(positionData.size());
        memcpy(message.data(), positionData.c_str(), positionData.size());
        receiver.send(message, zmq::send_flags::none);

        // Receive updated positions from the server
        zmq::message_t reply;
        receiver.recv(reply, zmq::recv_flags::none);

        std::string updatedPositions(reply.to_string());
        auto parsedPositions = parseUpdatedPositions(updatedPositions);

        // Process and update positions of entities from other clients
        for (const auto& position : parsedPositions) {
            std::string clientId = position["clientId"];
            for (const auto& entity : position["entities"]) {
                std::string type = entity["type"];
                int x = entity["x"];
                int y = entity["y"];

                if (type == "horizontal") {
                    concepts.m->setPosition(x, y);  // Update horizontal entity
                } else if (type == "vertical") {
                    concepts.v->setPosition(x, y);  // Update vertical entity
                }
            }
        }

        nlohmann::json controllableEntityDetails = {
            {"clientId", clientId},
            {"clientAddr", clientAddress},
            {"x", concepts.c->getRect().x},
            {"y", concepts.c->getRect().y}
        };
        std::string controllablePositionData = controllableEntityDetails.dump();

        broadcastPosition(dealerSockets, controllablePositionData);

        // Set the background color to blue and clear the screen
        SDL_SetRenderDrawColor(game.renderer, 0, 0, 255, 255);
        SDL_RenderClear(game.renderer);

        // Render the shapes
        if (!replayManager.isReplaying) {

        concepts.s->render(game.renderer);
        for (int j = 0; j < MAP_HEIGHT; j++) {
            for (int i = 0; i < MAP_WIDTH; i++) {
                if (concepts.tileMap[i][j] != nullptr) {
                    concepts.tileMap[i][j]->render(game.renderer);
                }
            }
        }

        concepts.m->render(game.renderer);
        // cout<<"The X coor of moving object is: "<<concepts.m->getRect().x<< "Thee Y coor is: "<<concepts.m->getRect().y<<endl;
        if (replayManager.isRecording) {  // Record moving object
            // replayManager.recordEntity(concepts.m, SDL_GetTicks64());
            // replayManager.recordEntity(concepts.v, SDL_GetTicks64());
            // replayManager.recordEntity(concepts.c, SDL_GetTicks64());
            replayManager.recordEntity(concepts.m, SDL_GetTicks64(), 'M');
            replayManager.recordEntity(concepts.v, SDL_GetTicks64(), 'V');
            replayManager.recordEntity(concepts.c, SDL_GetTicks64(), 'C');
        }

        concepts.v->render(game.renderer);
        concepts.spawn->render(game.renderer);
        concepts.despawn->render(game.renderer);
        boundEntityOne.render(game.renderer);
        boundEntityTwo.render(game.renderer);
        concepts.c->render(game.renderer);
  

        {
            std::lock_guard<std::mutex> lock(positionMutex);
            for (const auto& [clientId, rect] : entityPositions) {
                SDL_SetRenderDrawColor(game.renderer, 0, 255, 0, 255); // Green color for entities
                SDL_RenderFillRect(game.renderer, &rect);
                // cout<<"[Render loop]The client id is: "<<clientId<< "The X coor is: "<<rect.x<< "Thee Y coor is: "<<rect.y<<endl;
                if (replayManager.isRecording) {  // Record server-rendered entities
                    Entity serverEntity(rect.x, rect.y, rect.w, rect.h, {0, 255, 0}, false, 0);
                    replayManager.recordEntity(&serverEntity, SDL_GetTicks64(),'S');
                }
            }
        }
    } else {
        // Replay rendering
            concepts.a->pause();
            replayManager.playReplay(game.renderer);
            concepts.a->unpause();
        }
        // Dispatch events.
        eventManager.dispatchEvents();
        // Present the rendered content
        SDL_RenderPresent(game.renderer);
        // Puts a delay on the game. (Can be changed!)
        SDL_Delay(16);
        // Sets the last time to the current time.
        lastTime = currentTime;
    }
    
    // Clean up and shut down SDL
    SDL_DestroyRenderer(game.renderer);
    SDL_DestroyWindow(game.window);
    SDL_Quit();

    return 0;
}

///**
// * Runs the game.
// * 
// * Use this main function only for Homework 2 Sections 2 and 3.
// * 
// * References resources and tutorials provided by Professor Card through
// * the "CSC 481-581 HW 1-4.pdf" located beneath the "Homework 1" title in
// * the "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
// * Moodle page. These resources can be found in the
// * https://wiki.libsdl.org/SDL2/FrontPage website.
// * 
// * \param argc the count argument
// * \param argv the string argument
// * \returns int 0 if successful, else, unsuccessful
// * 
// * @author Lillie Sharpe
// * @author Chinmay Nayak
// * @author Robbie Martin
// */
// int main(int argc, char* argv[]) {
//     /**
//      * This code section is heavily inspired from the example delta time
//      * calculation displayed by Professor Card on page 17 in the
//      * "S24 05 Timelines.pptx" PowerPoint that is available on the
//      * "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
//      * Moodle page through the "Lecture Notes" link beneath the
//      * "General Information and Discussions" subtitle.
//      * 
//      * Start of inspired code.
//      */
//     // int64_t currentT = timeline.getTime();
//     // int64_t deltaF = currentT - lastT;
//     // lastT = currentT;
//     /** End of inspired code. */
//     srand(static_cast<unsigned int>(time(0)));
//     int randomNum = rand() % 10000; // Generate a random number between 0 and 9999
//     std::string clientId = "client" + std::to_string(randomNum);  // Random client ID
//     int clientPort = 5560 + randomNum; // Unique port based on client ID
//     std::string clientAddress = "tcp://localhost:" + std::to_string(clientPort);

//     // Create the Game object.
//     Game game;
//     // Set the window up.
//     game.window = nullptr;
//     // Set the renderer up.
//     game.renderer = nullptr;

//     // Attempt to initialize the game.
//     if (!initializeSDL(&game.window, &game.renderer)) {
//         return 1; // If unsuccessful, initialization failed.
//     }

//     // Initialize ZeroMQ context and sockets
//     zmq::context_t context(1);
//     zmq::socket_t receiver(context, ZMQ_REQ);
//     receiver.connect("tcp://localhost:5555"); // For sending position updates

//     zmq::socket_t subscriber(context, ZMQ_SUB);
//     subscriber.connect("tcp://localhost:5556"); // For receiving position updates
//     subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0); // Subscribe to all messages

//     // Create the Concepts object.
//     Concepts concepts;

//     // Initialize quit to false.
//     concepts.quit = false;

//     // Create an event object.
//     SDL_Event e;

//     // Construct the anchor timeline.
//     Timeline anchor(nullptr, 1);

//     // Creates the static red shape and connects its address to concepts.
//     Entity staticEntity(Rectangle(100,100,100,100),{255,0,0,255}, false); // Static red shape.
//     concepts.s = &staticEntity;

//     // Creates the controllable green shape and connects its address to concepts.
//     Entity controllableEntity(Rectangle(300,300,50,50),{0,255,0,255}, true); // Controllable green shape.
//     concepts.c = &controllableEntity;

//     // Creates the moving black shape and connects its address to concepts.
//     Entity movingEntity(Rectangle(100,100,100,100),{0,0,0,255}, false); // Black moving shape.
//     concepts.m = &movingEntity;

//     // Initializes scaling and held through concepts.
//     concepts.scaling = false;
//     concepts.held = false;

//     // Initializes physics variables.
//     concepts.gravity = 9.8f;
//     concepts.speed = 5; // Speed of the Entity.
//     concepts.verticalVel = 0.0f;
//     concepts.thrust = -9.8f;

//     // Stores the variable that determines whether or not the game is paused.
//     concepts.a = &anchor;

//     // Create a timeline to run threads.
//     Timeline timeThreads(&anchor, 1); // Set tic to whatever is desired.

//     // Sets the last time.
//     int64_t lastTime = anchor.getTimeline();

//     std::unordered_map<std::string, SDL_Rect> otherClientEntities;

//     // Runs the game.
//     while (!concepts.quit) {
//         // Gets the current time.
//         int64_t currentTime = anchor.getTimeline();
//         // Calculates delta time.
//         float deltaTime = 0;
//         if (timeThreads.getTicks() != 3) {
//             deltaTime = ((currentTime - lastTime) / timeThreads.getTicks()) / 1000.0f;
//         } else {
//             deltaTime = ((currentTime - lastTime) * 2) / 1000.0f;
//         }
//         // Stores delta time in concepts.
//         concepts.delta = deltaTime;
//
//         while (SDL_PollEvent(&e) != 0) {
//             if (e.type == SDL_QUIT) {
//                 concepts.quit = true;
//             }
//         }

//         concepts.state = SDL_GetKeyboardState(nullptr);
//         concepts.moveSpeed = 5;

//     // If the player is pressing 'O'.
//         if (concepts.state[SDL_SCANCODE_O]) { // Pause game.
//             if (!concepts.a->isPaused) {
//                 concepts.a->pause();
//             }
//         }

//         // If the player is pressing 'P'.
//         if (concepts.state[SDL_SCANCODE_P]) { // Unpause game.
//             if (concepts.a->isPaused) {
//                 concepts.a->unpause();
//             }
//         }

//         // If the player is pressing 'B'.
//         if(concepts.state[SDL_SCANCODE_B]){ // Set tic to 0.5 (which is marked with 3).
//             timeThreads.setTicks(3);
//             printf("Tics set to 0.5.\n");
//         }

//         // If the player is pressing 'N'.
//         if(concepts.state[SDL_SCANCODE_N]){ // Set tic to 1.
//             timeThreads.setTicks(1);
//             printf("Tics set to 1.\n");
//         }

//         // If the player is pressing 'M'.
//         if(concepts.state[SDL_SCANCODE_M]){ // Set tic to 2.
//             timeThreads.setTicks(2);
//             printf("Tics set to 2.\n");
//         }

//         if (!concepts.a->isPaused) {

//             // Run threads.
//             startThreads(&timeThreads, &concepts, &game);

//             // Keeps track of the controllable rectangle.
//             Rectangle cRect = concepts.c->getRect();
//             // Keeps track of the static rectangle.
//             Rectangle sRect = concepts.s->getRect();
//             // Keeps track of the moving rectangle.
//             Rectangle mRect = concepts.m->getRect();

//             // Senses other shapes for collision.
//             if (hasIntersection(&cRect, &sRect) == true) {
//                 // If there was an intersection on the top of the terrain rectangle,
//                 // the controllable rectangle lands on the terrain rectangle.
//                 if (intersect(&cRect, &sRect) == 2) {
//                     // Causes vertical collision.
//                     concepts.delta = 0;
//                     concepts.verticalVel = 0;
//                 }
//                 // More sides will be added in the future.
//             }

//             // Senses other shapes for collision.
//             if (hasIntersection(&cRect, &mRect) == true) {
//                 // If there was an intersection on the top of the terrain rectangle,
//                 // the controllable rectangle lands on the terrain rectangle.
//                 if (intersect(&cRect, &mRect) == 2) {
//                     // Causes vertical collision.
//                     concepts.delta = 0;
//                     concepts.verticalVel = 0;
//                     // Enables player movement mimicking the moving entity.
//                     concepts.c->move(concepts.speed, static_cast<int>(concepts.verticalVel));
//                     if (concepts.c->getRect().x > 1820 || concepts.c->getRect().x < 100) {
//                         concepts.speed = -concepts.speed;
//                     }
//                 }
//                 // More sides may be added in the future.
//             }

//         }

//         // Keeps track of the controllable rectangle.
//         Rectangle cRect = concepts.c->getRect();
//         // Keeps track of the static rectangle.
//         Rectangle sRect = concepts.s->getRect();
//         // Keeps track of the moving rectangle.
//         Rectangle mRect = concepts.m->getRect();

//         // Senses other shapes for collision.
//         if (hasIntersection(&cRect, &sRect) == true) {
//             // If there was an intersection on the top of the terrain rectangle,
//             // the controllable rectangle lands on the terrain rectangle.
//             if (intersect(&cRect, &sRect) == 2) {
//                 // Causes vertical collision.
//                 concepts.delta = 0;
//                 concepts.verticalVel = 0;
//             }
//             // More sides will be added in the future.
//         }
   

//         // Senses other shapes for collision.
//         if (hasIntersection(&cRect, &mRect) == true) {
//             // If there was an intersection on the top of the terrain rectangle,
//             // the controllable rectangle lands on the terrain rectangle.
//             if (intersect(&cRect, &mRect) == 2) {
//                 // Causes vertical collision.
//                 concepts.delta = 0;
//                 concepts.verticalVel = 0;
//                 // Enables player movement mimicking the moving entity.
//                 concepts.c->move(concepts.speed, static_cast<int>(concepts.verticalVel));
//                 if (concepts.c->getRect().x > 1820 || concepts.c->getRect().x < 100) {
//                     concepts.speed = -concepts.speed;
//                 }
//             }
//             // More sides may be added in the future.
//         }

//          json jsonString = {
//             {"clientId", clientId},
//             {"clientAddr", clientAddress},
//             {"x", concepts.c->getRect().x},
//             {"y", concepts.c->getRect().y}
//         };

//         std::string positionData = jsonString.dump();
//         zmq::message_t message(positionData.size());
//         memcpy(message.data(), positionData.c_str(), positionData.size());
//         receiver.send(message, zmq::send_flags::none);

//         zmq::message_t reply;
//         receiver.recv(reply, zmq::recv_flags::none);

//         std::string updatedPositions(reply.to_string());
//         auto parsedPositions = parseUpdatedPositions(updatedPositions);

//         for (const auto& position : parsedPositions) {
//             std::string clientIdFromServer = position["clientId"];
//             int xFromServer = position["position"]["x"];
//             int yFromServer = position["position"]["y"];

//             if (clientIdFromServer != clientId) { // Ensure not rendering own entity
//                 otherClientEntities[clientIdFromServer] = {xFromServer, yFromServer, 50, 50};
//             }
//         }

//         SDL_SetRenderDrawColor(game.renderer, 0, 0, 255, 255);
//         SDL_RenderClear(game.renderer);

//         concepts.s->render(game.renderer);
//         concepts.c->render(game.renderer);

//         // Render entities from other clients
//         for (const auto& [id, rect] : otherClientEntities) {
//             SDL_SetRenderDrawColor(game.renderer, 0, 255, 0, 255);
//             SDL_RenderFillRect(game.renderer, &rect);
//         }

//         concepts.m->render(game.renderer);
//         SDL_RenderPresent(game.renderer);

//         // Puts a delay on the game. (Can be changed!)
//         SDL_Delay(16);
//         // Sets the last time to the current time.
//         lastTime = currentTime;
//     }

//     // Clean up and shut down SDL
//     SDL_DestroyRenderer(game.renderer);
//     SDL_DestroyWindow(game.window);
//     SDL_Quit();

//     return 0;
// }