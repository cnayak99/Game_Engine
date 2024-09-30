// main.cpp
#include <SDL2/SDL.h>
#include <iostream>
#include "sdl_setup.h"
#include "Entity.h"
#include "Rectangle.h"
#include "Physics.h"
#include "Intersect.h"
#include "Time.h"
#include "Physics.h"
#include <zmq.hpp> 
#include <SDL2/SDL_ttf.h>
#include "json.hpp"
#include <thread>

using namespace std; 
using json = nlohmann::json;

/** Constructs the Time object. Tics can be 1, half, or double.*/
// Time timeline = Time(Time.anchor, 1); // How to construct anchor address?

/**
 * Creates a Time struct, which keeps track of and monitors a
 * timeline within the game.
 * 
 * This statement is heavily inspired from the example delta time
 * calculation displayed by Professor Card on page 17 in the
 * "S24 05 Timelines.pptx" PowerPoint that is available on the
 * "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page through the "Lecture Notes" link beneath the
 * "General Information and Discussions" subtitle.
 */
// int64_t lastT = timeline.getTime();

/**
 * Runs the game.
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

json parseUpdatedPositions(const std::string& updatedPositions) {
    return json::parse(updatedPositions); // Parse the JSON string into a JSON object
}

void printPositions(const json& positions) {
    for (const auto& position : positions) {
        std::string clientId = position["clientId"];
        std::string clientAddr = position["clientAddr"];
        int x = position["position"]["x"];
        int y = position["position"]["y"];
        cout<< "ClientAddr: "<<clientAddr<<endl;
        // std::cout << "Client ID: " << clientId << "Client Addr: "<< clientAddr <<" Position: (" << x << ", " << y << ")" << std::endl;
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

void printPositions(SDL_Renderer* renderer, TTF_Font* font, const std::vector<std::tuple<std::string, int, int>>& positions) {
    SDL_Color textColor = {255, 255, 255}; // White color for the text
    int yOffset = 20; // Starting Y position for text rendering

    for (const auto& [clientId, x, y] : positions) {
        std::string positionText = clientId + " is at position (" + std::to_string(x) + ", " + std::to_string(y) + ")";

        // Create an SDL_Surface with the text
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, positionText.c_str(), textColor);
        if (textSurface == nullptr) {
            std::cerr << "Failed to create text surface: " << TTF_GetError() << std::endl;
            continue;
        }

        // Create a texture from the surface
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface); // Free the surface once the texture is created

        if (textTexture == nullptr) {
            std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
            continue;
        }

        // Set the position and size for rendering the text
        SDL_Rect textRect = {20, yOffset, textSurface->w, textSurface->h}; // x = 20, y = yOffset
        yOffset += textSurface->h + 10; // Increment yOffset for next line of text

        // Render the text on the screen
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        // Clean up the texture
        SDL_DestroyTexture(textTexture);
    }
}
int main(int argc, char* argv[]) {
    /**
     * This code section is heavily inspired from the example delta time
     * calculation displayed by Professor Card on page 17 in the
     * "S24 05 Timelines.pptx" PowerPoint that is available on the
     * "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
     * Moodle page through the "Lecture Notes" link beneath the
     * "General Information and Discussions" subtitle.
     * 
     * Start of inspired code.
     */
    // int64_t currentT = timeline.getTime();
    // int64_t deltaF = currentT - lastT;
    // lastT = currentT;
    /** End of inspired code. */

    srand(static_cast<unsigned int>(time(0)));
    int randomNum = rand() % 10000; // Generate a random number between 0 and 9999
    std::string clientId = "client" + std::to_string(randomNum);  // Random client ID
    // std::string clientId = "Admin";
    int clientPort = 5560 + randomNum; // Unique port based on client ID
    std::string clientAddress = "tcp://localhost:" + std::to_string(clientPort);
      
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!initializeSDL(&window, &renderer)) {
        return 1; // Initialization failed
    }
    if (TTF_Init() == -1) {
        std::cerr << "Error initializing SDL_ttf: " << TTF_GetError() << std::endl;
        return -1;
    }

    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/msttcorefonts/Arial.ttf", 24);
    if (font == nullptr) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
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
    bool quit = false;
    SDL_Event e;
    

    Entity staticEntity(Rectangle(100,100,100,100),{255,0,0,255}, false);//Static Red Shape
    Entity controllableEntity(Rectangle(300,300,50,50),{0,255,0,255}, true);//Controllable Green Shape 
    Entity movingEntity(Rectangle(100,100,100,100),{0,0,0,255}, true);//Black Moving Shape
    bool scaling = false;
    bool held = false;
    float gravity = 9.8f;
    // Physics physics(gravity);

    int speed = 5; // Speed of the Entity
    float verticalVel =0.0f;
    float thrust = -9.8f;
    Uint32 lastTime = SDL_GetTicks();



    while (!quit) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
        
        const Uint8* state = SDL_GetKeyboardState(nullptr);
        int moveSpeed = 5;

        if(state[SDL_SCANCODE_UP]){
            // controllableEntity.move(0, -moveSpeed);
            verticalVel = thrust;
        }

        if(state[SDL_SCANCODE_LEFT]){//Move Right
            controllableEntity.move(-moveSpeed,0);
        }
        if(state[SDL_SCANCODE_RIGHT]){//Move Right
            controllableEntity.move(moveSpeed, 0);
        }
        if (state[SDL_SCANCODE_C]) {
            if (!held) {
                held = true;
                if (!scaling) {
                    SDL_RenderSetLogicalSize(renderer, 1920, 1080);
                    scaling = true;
                }
                else {
                    SDL_RenderSetLogicalSize(renderer, 0, 0);
                    scaling = false;
                }
            }
        }
        else {
            held = false;
        }
        if (state[SDL_SCANCODE_ESCAPE]) {// Exit the game
            quit = true; 
        }

        verticalVel += gravity * deltaTime;
        controllableEntity.move(0, static_cast<int>(verticalVel));
        //Apply Gravity to this object

        // Move the shape in a continuous pattern (horizontal)
        movingEntity.move(speed, 0);
        if (movingEntity.getRect().x > 1820 || movingEntity.getRect().x < 100) {
            speed = -speed;
        }

        // Keeps track of the controllable rectangle.
        Rectangle c = controllableEntity.getRect();
        // Keeps track of the moving rectangle.
        Rectangle m = movingEntity.getRect();
        // Keeps track of the static rectangle.
        Rectangle s = staticEntity.getRect();
        // Stores the address of a Rectangle entity.
        // Due to redundancy, for now, 'result' has been removed.
        // Rectangle * result;

        // Senses other shapes for collision.
        if (hasIntersection(&c, &m) == true) {
            // If there was an intersection on the top of the terrain rectangle,
            // the controllable rectangle lands on the terrain rectangle.
            if (intersect(&c, &m) == 2) {
                // Causes vertical collision.
                deltaTime = 0;
                verticalVel = 0;
                // Enables player movement mimicking the moving entity.
                controllableEntity.move(speed, static_cast<int>(verticalVel));
                if (controllableEntity.getRect().x > 1820 || controllableEntity.getRect().x < 100) {
                    speed = -speed;
                }
            }
            // More sides will be added in the future.
        }

        // Senses other shapes for collision.
        if (hasIntersection(&c, &s) == true) {
            // If there was an intersection on the top of the terrain rectangle,
            // the controllable rectangle lands on the terrain rectangle.
            if (intersect(&c, &s) == 2) {
                // Causes vertical collision.
                deltaTime = 0;
                verticalVel = 0;
            }
            // More sides will be added in the future.
        }

        // Send the position of the controllable entity to the server
        // string positionData = clientId + ":" + to_string(movingEntity.getRect().x) + "," +
        //                       to_string(movingEntity.getRect().y);
        json jsonString = {
            {"clientId", clientId},
            {"clientAddr", clientAddress},
            {"x", movingEntity.getRect().x},
            {"y", movingEntity.getRect().y}
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
            if (clientId != "Admin") {
                movingEntity.setPosition(x, y); // Implement setPosition method in Entity class
            }
        }

        nlohmann::json controllableEntityDetails = {
            {"clientId", clientId},
            {"clientAddr", clientAddress},
            {"x", controllableEntity.getRect().x},
            {"y", controllableEntity.getRect().y}
        };
        std::string controllablePositionData = controllableEntityDetails.dump();

        broadcastPosition(dealerSockets, controllablePositionData);

              

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderClear(renderer);

        // Render the shapes
        staticEntity.render(renderer);
        controllableEntity.render(renderer);
        movingEntity.render(renderer);
        // printPositions(renderer, font, parsedPositions);

        // Render entities based on updated positions
        {
            std::lock_guard<std::mutex> lock(positionMutex);
            for (const auto& [clientId, rect] : entityPositions) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green color for entities
                SDL_RenderFillRect(renderer, &rect);
            }
        }
        // Present the rendered content
        SDL_RenderPresent(renderer);

        // ~60 frames per second
        SDL_Delay(16);
    }

    // Clean up and shut down SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}