// main.cpp
#include <SDL2/SDL.h>
#include <iostream>
#include "../include/sdl_setup.h"
#include "../include/Entity.h"
#include "../include/Rectangle.h"
#include "../include/Physics.h"
#include "../include/Intersect.h"
#include "../include/Time.h"
#include "../include/Physics.h"
#include <../include/zmq.hpp> 
#include "../include/json.hpp"
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
    int clientPort = 5560 + randomNum; // Unique port based on client ID
    std::string clientAddress = "tcp://localhost:" + std::to_string(clientPort);
      
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!initializeSDL(&window, &renderer)) {
        return 1; // Initialization failed
    }
    

    // Initialize ZeroMQ context and sockets
    zmq::context_t context(1);
    zmq::socket_t receiver(context, ZMQ_REQ);
    receiver.connect("tcp://localhost:5555"); // For sending position updates

    zmq::socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect("tcp://localhost:5556"); // For receiving position updates
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0); // Subscribe to all messages

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

                movingEntity.setPosition(x, y); // Implement setPosition method in Entity class
            
        }              

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderClear(renderer);

        // Render the shapes
        staticEntity.render(renderer);
        controllableEntity.render(renderer);
        movingEntity.render(renderer);

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