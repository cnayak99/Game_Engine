#include "Event.h"

Event::Event(Entity* source, int type) : source(source), type(type) {    
}

int Event::getType() {
    return type;
}

Entity* Event::getSource() {
    return source;
}