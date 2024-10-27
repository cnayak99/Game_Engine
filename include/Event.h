#ifndef EVENT_H
#define EVENT_H

#include "Entity.h"
#include <SDL2/SDL.h>

/**
 * Creates an Event.
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
 * Events have one field: an Entity pointer the handler will use to respond to events it hears. 
 * The source in the event and the handler may be different, depending on the event.
 */
class EventHandler {
public:
    EventHandler(Entity* source);
    void onEvent(Event e);
private:
    Entity* source;
};

class EventManager {
public:
    EventManager();
private:
    
};

#endif // EVENT_H
