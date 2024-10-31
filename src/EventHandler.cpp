#include "Event.h"

EventHandler::EventHandler(Entity* source) : source(source) {
    
}

void PlayerHandler:: onEvent(Event e) {
    switch (e.getType())
    {
    case 1:
        // Collision Event
        break;
    case 2:
        // Death Event
        break;
    case 3:
        // Spawn Event
        break;
    case 4:
        // Input Event
        break;
    default:
        break;
    }
}

Entity* PlayerHandler:: getSource() {
    return source;
}