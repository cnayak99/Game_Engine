// #ifndef EVENT_H
// #define EVENT_H

// #include "structs.h"
// #include "Entity.h"
// #include <SDL2/SDL.h>
// #include <map>
// #include <list>
// #include <vector>
// #include <mutex>
// #include <queue>
// #include <iostream>
// #include <zmq.hpp>
// #include "json.hpp" 

// using json = nlohmann::json;

// /**
//  * Creates an Event.
//  * 
//  * References resources and tutorials provided by Professor Card through
//  * the Events and Events 2 lecture slides in the "CSC 481/581 (001) 
//  * Fall 2024 Game Engine Foundations" course Moodle page. 
//  * These resources can be found in the
//  * https://wiki.libsdl.org/SDL2/FrontPage website.
//  * 
//  * Events have two fields:
//  * - an Entity pointer dictating the cause of the event
//  * - an integer describing the event's type, representing what occurred.
//  *      - 1 = Collision event; source collided with another entity. 
//  *          - (e.g. a player collides with the floor -> an event is caused by the Entity they collided with of type 1)
//  *      - 2 = Death event; source collided with a death zone or was otherise killed
//  *          - (e.g. a player hits a death zone -> an event is caused by the player entity of type 2)
//  *      - 3 = Spawn event; source was sent to its spawn point (if it has one)
//  *          - (e.g. a player spawns -> an event is caused by the player entity of type 3)
//  *      - 4 = Input event; input is being received from the user
//  *          - (e.g. the user presses right -> an event of type 4 is caused (the source entity is irrelevant here))
//  */


// // struct Concepts;
// // struct Game;

// struct Variant {
//     enum Type { TYPE_INT, TYPE_FLOAT, TYPE_STRING } type;
//     union {
//         int asInt;
//         float asFloat;
//         const char* asString;
//     };
// };

// class Event {
// public:
//     // Event(Entity* source, int type);
//     // int getType();
//     // Entity* getSource();
//     std::string type; // Event type (e.g., "collision", "input", etc.)
//     std::map<std::string, Variant> parameters; // Parameters for the event
// // private:
// //     int type;
// //     Entity* source;
// };

// /**
//  * Creates an EventHandler.
//  * 
//  * Event handlers have five (as of writing) fields: 
//  * - the first is an Entity pointer the handler will affect in response to events it hears. 
//  * - the remaining fields are booleans to determine what the handler should be listening for.
//  */
// class EventHandler {
// public:
//     // EventHandler(Entity* source);
//     virtual void onEvent(const Event& e) = 0;
//     // Entity* source;
// };

// class PlayerHandler : public EventHandler {
//     public:
//         using EventHandler::EventHandler;
//         void onEvent(const Event& e) override;
//         // Entity* getSource();
// };

// class InputHandler : public EventHandler {
// private:
//     Concepts* concepts;   
//     Game* game;           

// public:
//     InputHandler(Concepts* c, Game* g) : concepts(c), game(g) {}

//     void onEvent(const Event& e) override;
// };


// class QuitHandler : public EventHandler {
// private:
//     Concepts* concepts;  // Pointer to Concepts object
//     zmq::socket_t* receiverPtr;  // Pointer to ZMQ receiver socket
//     std::string clientId1;

// public:
//     QuitHandler(Concepts* c, zmq::socket_t* rcvPtr, const std::string& clientId)
//         : concepts(c), receiverPtr(rcvPtr), clientId1(clientId) {}

//     void onEvent(const Event& e) override;
// };


// class EventManager {
// private:
//     std::map<std::string, std::vector<EventHandler*>> listeners; // Map of event types to handlers
//     std::queue<Event> eventQueue; // Queue of events

// public:
//     // Register a handler for a specific event type
//     void registerListener(const std::string& eventType, EventHandler* handler);

//     // Raise an event by adding it to the queue
//     void raiseEvent(const Event& event);

//     // Dispatch all queued events to their respective handlers
//     void dispatchEvents();
// };


// #endif // EVENT_H

#ifndef EVENT_H
#define EVENT_H

#include "Entity.h"
#include <SDL2/SDL.h>
#include <map>
#include <list>
#include <vector>
#include <mutex>
#include <queue>
#include <iostream>
#include <zmq.hpp>
#include "json.hpp"
#include "structs.h"  // Include Concepts and Game structs

using json = nlohmann::json;

struct Variant {
    enum Type { TYPE_INT, TYPE_FLOAT, TYPE_STRING } type;
    union {
        int asInt;
        float asFloat;
        const char* asString;
    };
};

class Event {
public:
    std::string type; // Event type (e.g., "collision", "input", etc.)
    std::map<std::string, Variant> parameters; // Parameters for the event
};

// class PlayerHandler : public EventHandler {
// public:
//     using EventHandler::EventHandler;
//     void onEvent(const Event& e) override;
// };

// class InputHandler : public EventHandler {
// private:
//     Concepts* concepts;   
//     Game* game;           

// public:
//     InputHandler(Concepts* c, Game* g) : concepts(c), game(g) {}

//     void onEvent(const Event& e) override;
// };

// class QuitHandler : public EventHandler {
// private:
//     Concepts* concepts;  
//     zmq::socket_t* receiverPtr;  
//     std::string clientId1;

// public:
//     QuitHandler(Concepts* c, zmq::socket_t* rcvPtr, const std::string& clientId)
//         : concepts(c), receiverPtr(rcvPtr), clientId1(clientId) {}

//     void onEvent(const Event& e) override;
// };


#endif // EVENT_H