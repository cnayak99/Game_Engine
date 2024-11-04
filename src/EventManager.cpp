#include "Event.h"
#include <string>
#include "structs.h"
#include "EventManager.h"
#include "EventHandler.h"

void EventManager::registerListener(const std::string& eventType, EventHandler* handler) {
    std::cout<<"Registering Event: "<< eventType<<std::endl;
    listeners[eventType].push_back(handler);
}

void EventManager::raiseEvent(const Event& event) {
    std::lock_guard<std::mutex> lock(queueMutex);  // Lock the mutex for thread-safe access

    // Add the event to the priority queue
    eventQueue.push(event);

    // Debug statement
    std::cout << "Raising Event: " << event.type << ". Queue size: " << eventQueue.size() << std::endl;
}

// Dispatch all queued events to their respective handlers.
void EventManager::dispatchEvents() {
    std::cout<<"Dispatching Event: "<<std::endl;
    std::lock_guard<std::mutex> lock(queueMutex);
    while (!eventQueue.empty()) {
        Event e = eventQueue.top();
        eventQueue.pop();
        std::cout << "Dispatching Event of type: " << e.type << std::endl; 
        if (listeners.find(e.type) != listeners.end()) {
            for (auto& handler : listeners[e.type]) {
                handler->onEvent(e); // Dispatch the event to all registered handlers.
                std::cout << "Event dispatched to handler" << std::endl;
            }
        }
    }
}

