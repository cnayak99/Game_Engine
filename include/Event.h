#ifndef EVENT_H
#define EVENT_H

#include "Entity.h"
#include <SDL2/SDL.h>

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

class Event {
public:
    Event(Entity* source, int type);
    int getType();
    Entity* getSource();
private:
    int type;
    Entity* source;
};

/**
 * Creates an EventHandler.
 * 
 * Event handlers have five (as of writing) fields: 
 * - the first is an Entity pointer the handler will affect in response to events it hears. 
 * - the remaining fields are booleans to determine what the handler should be listening for.
 */
class EventHandler {
public:
    EventHandler(Entity* source);
    virtual void onEvent(Event e) = 0;
    Entity* source;
};

class PlayerHandler : public EventHandler {
    public:
        using EventHandler::EventHandler;
        void onEvent(Event e);
        Entity* getSource();
};

class EventManager {
public:
    EventManager();
private:
    
};

#endif // EVENT_H
