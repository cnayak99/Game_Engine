#include "Event.h"

EventHandler::EventHandler(Entity* source, bool listenCollide, bool listenDestroy, bool listenSpawn, bool listenInput) 
: source(source), listenCollide(listenCollide), listenDestroy(listenDestroy), listenSpawn(listenSpawn), listenInput(listenInput) {

}

void EventHandler::onEvent(Event e) {
    switch (e.getType())
    {
    case 1:
        if (listenCollide) {
            
        }
        break;
    case 2:
        if (listenDestroy) {

        }
        break;
    case 3:
        if (listenSpawn) {

        }
        break;
    case 4:
        if (listenInput) {

        }
        break;
    default:
        break;
    }
}
