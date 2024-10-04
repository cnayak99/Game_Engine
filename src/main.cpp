// main.cpp
#include <SDL2/SDL.h>
#include <iostream>
#include "sdl_setup.h"
#include "Entity.h"
#include "Rectangle.h"
#include "Physics.h"
#include "Intersect.h"
#include "Timeline.h"    
#include "structs.h"     
#include <zmq.hpp>
#include "json.hpp" // Use relative path to the include directory
#include <thread>
#include "Threads.h"

using namespace std; 
using json = nlohmann::json;

json parseUpdatedPositions(const std::string& updatedPositions) {
    return json::parse(updatedPositions); // Parse the JSON string into a JSON object
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


void broadcastPosition(std::unordered_map<std::string, zmq::socket_t>& dealerSockets, const std::string& positionData) {
    std::lock_guard<std::mutex> lock(knownAddressesMutex); // Lock for thread safety
    for (auto& [addr, socket] : dealerSockets) {
        zmq::message_t msg(positionData.size());
        memcpy(msg.data(), positionData.c_str(), positionData.size());

        // Send the message
        socket.send(std::move(msg), zmq::send_flags::none);
    }
}

// Function to listen for updates and establish connections
void listenForUpdates(zmq::socket_t& subscriberSocket, std::unordered_map<std::string, zmq::socket_t>& dealerSockets, zmq::context_t& context, const std::string& clientAddress) {
    while (true) {
        zmq::message_t update;
        subscriberSocket.recv(update, zmq::recv_flags::none);
        std::string updateStr(static_cast<char*>(update.data()), update.size());

        try {
            auto updatedClientAddresses = json::parse(updateStr);
            std::cout << "Received updated client addresses:" << std::endl;
            for (auto& [id, addr] : updatedClientAddresses.items()) {
                std::cout << id << ": " << addr << std::endl;

                // Ensure addr is treated as a string
                std::string addrStr = addr.get<std::string>();

                if (addrStr == clientAddress) {
                    // Skip connecting to itself
                    continue;
                }

                std::lock_guard<std::mutex> lock(knownAddressesMutex);
                if (dealerSockets.find(addrStr) == dealerSockets.end() && updatedClientAddresses.size() > 1) {
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
    std::string clientAddress = "tcp://localhost:" + std::to_string(clientPort);
    
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
    receiver.connect("tcp://localhost:5555"); // For sending position updates

    zmq::socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect("tcp://localhost:5556"); // For receiving position updates
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0); // Subscribe to all messages

    zmq::socket_t routerSocket(context, ZMQ_ROUTER);
    routerSocket.bind(clientAddress);

    // Map to store DEALER sockets by address
    std::unordered_map<std::string, zmq::socket_t> dealerSockets;

    // Map to track identities and their corresponding addresses
    std::unordered_map<std::string, std::string> identityToAddressMap;

    // Start thread for listening for server updates
    std::thread updateListener(listenForUpdates, std::ref(subscriber), std::ref(dealerSockets), std::ref(context), std::cref(clientAddress));
    updateListener.detach();

    // Start thread to handle incoming messages
    std::thread incomingHandler(handleIncomingMessages,std::ref(routerSocket),std::ref(identityToAddressMap));
    incomingHandler.detach();

    // Create the Concepts object.
    Concepts concepts;

    // Initialize quit to false.
    concepts.quit = false;

    // Create an event object.
    SDL_Event e;

    // Construct the anchor timeline.
    Timeline anchor(nullptr, 1);
    
    // Creates the static red shape and connects its address to concepts.
    Entity staticEntity(Rectangle(100,100,100,100),{255,0,0,255}, false); // Static red shape.
    concepts.s = &staticEntity;

    // Creates the controllable green shape and connects its address to concepts.
    Entity controllableEntity(Rectangle(300,300,50,50),{0,255,0,255}, true); // Controllable green shape.
    concepts.c = &controllableEntity;

    // Creates the moving black shape and connects its address to concepts.
    Entity movingEntity(Rectangle(100,100,100,100),{0,0,0,255}, false); // Black moving shape.
    concepts.m = &movingEntity;

    // Initializes scaling and held through concepts.
    concepts.scaling = false;
    concepts.held = false;

    // Initializes physics variables.
    concepts.gravity = 9.8f;
    concepts.speed = 5; // Speed of the Entity.
    concepts.verticalVel = 0.0f;
    concepts.thrust = -9.8f;

    // Sets the last time.
    int64_t lastTime = anchor.getTimeline();

    // Stores the variable that determines whether or not the game is paused.
    concepts.p = &anchor.isPaused;

    // Runs the game.
    while (!concepts.quit) {
        // Gets the current time.
        int64_t currentTime = anchor.getTimeline();
        // Calculates delta time.
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        // Stores delta time in concepts.
        concepts.delta = deltaTime;
        // TEMPORARY: prints calculated time variables.
        printf("Time: %ld\nLast Time: %ld\nDelta: %f\n", currentTime, lastTime, deltaTime);

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

        // Create a timeline to run threads.
        Timeline timeThreads(&anchor, 1); // Set tic to whatever is desired.

        if (concepts.state[SDL_SCANCODE_P]) { // Pause game.
            if (!concepts.held) {
                concepts.held = true;
                if (!anchor.isPaused) {
                    anchor.pause();
                }
                else {
                    anchor.unpause();
                }
            }
        }

        // Pausing functionality started here!
        if (!anchor.isPaused) {

            // Run threads.
            startThreads(&timeThreads, &concepts, &game);

            // Keeps track of the controllable rectangle.
            Rectangle cRect = concepts.c->getRect();
            // Keeps track of the static rectangle.
            Rectangle sRect = concepts.s->getRect();
            // Keeps track of the moving rectangle.
            Rectangle mRect = concepts.m->getRect();

            // Senses other shapes for collision.
            if (hasIntersection(&cRect, &sRect) == true) {
                // If there was an intersection on the top of the terrain rectangle,
                // the controllable rectangle lands on the terrain rectangle.
                if (intersect(&cRect, &sRect) == 2) {
                    // Causes vertical collision.
                    concepts.delta = 0;
                    concepts.verticalVel = 0;
                }
                // More sides will be added in the future.
            }

            // Senses other shapes for collision.
            if (hasIntersection(&cRect, &mRect) == true) {
                // If there was an intersection on the top of the terrain rectangle,
                // the controllable rectangle lands on the terrain rectangle.
                if (intersect(&cRect, &mRect) == 2) {
                    // Causes vertical collision.
                    concepts.delta = 0;
                    concepts.verticalVel = 0;
                    // Enables player movement mimicking the moving entity.
                    concepts.c->move(concepts.speed, static_cast<int>(concepts.verticalVel));
                    if (concepts.c->getRect().x > 1820 || concepts.c->getRect().x < 100) {
                        concepts.speed = -concepts.speed;
                    }
                }
                // More sides may be added in the future.
            }

        }
        // Pausing functionality ended here!

        json jsonString = {
            {"clientId", clientId},
            {"clientAddr", clientAddress},
            {"x", concepts.m->getRect().x},
            {"y", concepts.m->getRect().y}
        };

        std::string positionData  = jsonString.dump();
        zmq::message_t message(positionData.size());
        memcpy(message.data(), positionData.c_str(), positionData.size());
        receiver.send(message, zmq::send_flags::none);

        // Receive updated positions from the server
        zmq::message_t reply;
        receiver.recv(reply, zmq::recv_flags::none);
        // Parse and update positions of other entities based on received data
        string updatedPositions(reply.to_string());
        vector<string> peerAddresses;
        auto parsedPositions = parseUpdatedPositions(updatedPositions);
        // printPositions(parsedPositions);

        for (const auto& position : parsedPositions) {
            std::string clientId = position["clientId"]; // Get the clientId from the JSON object
            int x = position["position"]["x"]; // Get the x coordinate from the nested "position" object
            int y = position["position"]["y"]; // Get the y coordinate from the nested "position" object

            // Update movingEntity's position based on the server data for the controlling client

                concepts.m->setPosition(x, y); // Implement setPosition method in Entity class
            
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
        concepts.s->render(game.renderer);
        concepts.c->render(game.renderer);
        concepts.m->render(game.renderer);
        
        {
            std::lock_guard<std::mutex> lock(positionMutex);
            for (const auto& [clientId, rect] : entityPositions) {
                SDL_SetRenderDrawColor(game.renderer, 0, 255, 0, 255); // Green color for entities
                SDL_RenderFillRect(game.renderer, &rect);
            }
        }
        // Present the rendered content
        SDL_RenderPresent(game.renderer);

        // ~60 frames per second
        SDL_Delay(16);
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
//
//     SDL_Window* window = nullptr;
//     SDL_Renderer* renderer = nullptr;
//
//     if (!initializeSDL(&window, &renderer)) {
//         return 1; // Initialization failed
//     }
//  
//
//     // Initialize ZeroMQ context and sockets
//     zmq::context_t context(1);
//     zmq::socket_t receiver(context, ZMQ_REQ);
//     receiver.connect("tcp://localhost:5555"); // For sending position updates
//
//     zmq::socket_t subscriber(context, ZMQ_SUB);
//     subscriber.connect("tcp://localhost:5556"); // For receiving position updates
//     subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0); // Subscribe to all messages
//
//     bool quit = false;
//     SDL_Event e;
//   
//
//     Entity staticEntity(Rectangle(100,100,100,100),{255,0,0,255}, false);//Static Red Shape
//     Entity controllableEntity(Rectangle(300,300,50,50),{0,255,0,255}, true);//Controllable Green Shape 
//     Entity movingEntity(Rectangle(100,100,100,100),{0,0,0,255}, true);//Black Moving Shape
//     bool scaling = false;
//     bool held = false;
//     float gravity = 9.8f;
//     // Physics physics(gravity);
//
//     int speed = 5; // Speed of the Entity
//     float verticalVel =0.0f;
//     float thrust = -9.8f;
//     Uint32 lastTime = SDL_GetTicks();
//
//     // Stores the variable that determines whether or not the game is paused.
//     concepts.p = &anchor.isPaused;
//
//
//     std::unordered_map<std::string, SDL_Rect> otherClientEntities;
//
//     while (!quit) {
//         Uint32 currentTime = SDL_GetTicks();
//         float deltaTime = (currentTime - lastTime) / 1000.0f;
//         lastTime = currentTime;
//
//         while (SDL_PollEvent(&e) != 0) {
//             if (e.type == SDL_QUIT) {
//                 quit = true;
//             }
//         }
//
//         const Uint8* state = SDL_GetKeyboardState(nullptr);
//         int moveSpeed = 5;
//
    // Create a timeline to run threads.
    // Timeline timeThreads(&anchor, 1); // Set tic to whatever is desired.

    // // Pausing functionality started here!

    //     // Run threads.
    //     startThreads(&timeThreads, &concepts, &game);

    //     // Keeps track of the controllable rectangle.
    //     Rectangle cRect = concepts.c->getRect();
    //     // Keeps track of the static rectangle.
    //     Rectangle sRect = concepts.s->getRect();
    //     // Keeps track of the moving rectangle.
    //     Rectangle mRect = concepts.m->getRect();

    //     // Senses other shapes for collision.
    //     if (hasIntersection(&cRect, &sRect) == true) {
    //         // If there was an intersection on the top of the terrain rectangle,
    //         // the controllable rectangle lands on the terrain rectangle.
    //         if (intersect(&cRect, &sRect) == 2) {
    //             // Causes vertical collision.
    //             concepts.delta = 0;
    //             concepts.verticalVel = 0;
    //         }
    //         // More sides will be added in the future.
    //     }

    //     // Senses other shapes for collision.
    //     if (hasIntersection(&cRect, &mRect) == true) {
    //         // If there was an intersection on the top of the terrain rectangle,
    //         // the controllable rectangle lands on the terrain rectangle.
    //         if (intersect(&cRect, &mRect) == 2) {
    //             // Causes vertical collision.
    //             concepts.delta = 0;
    //             concepts.verticalVel = 0;
    //             // Enables player movement mimicking the moving entity.
    //             concepts.c->move(concepts.speed, static_cast<int>(concepts.verticalVel));
    //             if (concepts.c->getRect().x > 1820 || concepts.c->getRect().x < 100) {
    //                 concepts.speed = -concepts.speed;
    //             }
    //         }
    //         // More sides may be added in the future.
    //     }

//         // Pausing functionality ended here!
//
//         // Keeps track of the controllable rectangle.
//         Rectangle cRect = concepts.c->getRect();
//         // Keeps track of the static rectangle.
//         Rectangle sRect = concepts.s->getRect();
//         // Keeps track of the moving rectangle.
//         Rectangle mRect = concepts.m->getRect();
//
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
//    
//
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
//
//          json jsonString = {
//             {"clientId", clientId},
//             {"clientAddr", clientAddress},
//             {"x", concepts.c->getRect().x},
//             {"y", concepts.c->getRect().y}
//         };
//
//         std::string positionData = jsonString.dump();
//         zmq::message_t message(positionData.size());
//         memcpy(message.data(), positionData.c_str(), positionData.size());
//         receiver.send(message, zmq::send_flags::none);
//
//         zmq::message_t reply;
//         receiver.recv(reply, zmq::recv_flags::none);
//
//         std::string updatedPositions(reply.to_string());
//         auto parsedPositions = parseUpdatedPositions(updatedPositions);
//
//         for (const auto& position : parsedPositions) {
//             std::string clientIdFromServer = position["clientId"];
//             int xFromServer = position["position"]["x"];
//             int yFromServer = position["position"]["y"];
//
//             if (clientIdFromServer != clientId) { // Ensure not rendering own entity
//                 otherClientEntities[clientIdFromServer] = {xFromServer, yFromServer, 50, 50};
//             }
//         }
//
//         SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
//         SDL_RenderClear(renderer);
//
//         concepts.s->render(renderer);
//         concepts.c->render(renderer);
//
//         // Render entities from other clients
//         for (const auto& [id, rect] : otherClientEntities) {
//             SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
//             SDL_RenderFillRect(renderer, &rect);
//         }
//
//         concepts.m->render(renderer);
//         SDL_RenderPresent(renderer);
//         SDL_Delay(16);
//     }
//
//     // Clean up and shut down SDL
//     SDL_DestroyRenderer(renderer);
//     SDL_DestroyWindow(window);
//     SDL_Quit();
//
//     return 0;
// }
