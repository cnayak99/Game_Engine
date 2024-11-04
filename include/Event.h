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

/**
 * Creates an Event.
 * 
 * References resources and tutorials provided by Professor Card through
 * the Events and Events 2 lecture slides in the "CSC 481/581 (001) 
 * Fall 2024 Game Engine Foundations" course Moodle page. 
 * These resources can be found in the
 * https://wiki.libsdl.org/SDL2/FrontPage website.
 * 
 * Events have two fields:
 * - an Entity pointer dictating the cause of the event
 * - an integer describing the event's type, representing what occurred.
 *      - 1 = Collision event; source collided with another entity. 
 *          - (e.g. a player collides with the floor -> an event is caused by the Entity they collided with of type 1)
 *      - 2 = Death event; source collided with a death zone or was otherise killed
 *          - (e.g. a player hits a death zone -> an event is caused by the player entity of type 2)
 *      - 3 = Spawn event; source was sent to its spawn point (if it has one)
 *          - (e.g. a player spawns -> an event is caused by the player entity of type 3)
 *      - 4 = Input event; input is being received from the user
 *          - (e.g. the user presses right -> an event of type 4 is caused (the source entity is irrelevant here))
 */



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
    std::string type;  // Event type (e.g., "input")
    std::unordered_map<std::string, Variant> parameters;  // Event parameters
    int64_t timestamp;  // Timestamp when the event was raised

    // Constructor that initializes the event with a type and timestamp
    Event(const std::string& type, int64_t timestamp)
        : type(type), timestamp(timestamp) {}
};



#endif // EVENT_H