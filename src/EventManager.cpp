#include "Event.h"

EventManager::EventManager() {
    for (int i = 1; i <= 4; i++) {
        handlers.emplace(i, new std::list<EventHandler *>);
    }
    std::mutex mutex_event;
}
void EventManager::handlerRegister(std::list<int> types, EventHandler* h) {
    for (int type:types) {
        handlers.insert_or_assign(type, h);
    }
}
void EventManager::handlerDeregister(std::list<int> types, EventHandler* h) {
    for (int type:types) {
        handlers.at(type).remove(h);
    }
}
void EventManager::raise(Event e) {
    std::lock_guard lock(mutex_event);
    raisedEvents.push_back(e);
}