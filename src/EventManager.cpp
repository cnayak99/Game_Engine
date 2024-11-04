#include "Event.h"
#include <string>
#include "structs.h"
#include "EventManager.h"
#include "EventHandler.h"
// EventManager::EventManager() {
//     for (int i = 1; i <= 4; i++) {
//         handlers.emplace(i, new std::list<EventHandler *>);
//     }
//     std::mutex mutex_event;
// }
// void EventManager::handlerRegister(std::list<int> types, EventHandler* h) {
//     for (int type:types) {
//         handlers.insert_or_assign(type, h);
//     }
// }
// void EventManager::handlerDeregister(std::list<int> types, EventHandler* h) {
//     for (int type:types) {
//         handlers.at(type).remove(h);
//     }
// }
// void EventManager::raise(Event e) {
//     std::lock_guard lock(mutex_event);
//     raisedEvents.push_back(e);
// }
// Register a handler for a specific event type.
void EventManager::registerListener(const std::string& eventType, EventHandler* handler) {
    std::cout<<"Registering Event: "<< eventType<<std::endl;
    listeners[eventType].push_back(handler);
}

// Raise an event by adding it to the queue.
// void EventManager::raiseEvent(const Event& event) {
//     std::cout << "Raising Event of type: " << event.type << std::endl;
//     eventQueue.push(event);
// }

void EventManager::raiseEvent(const Event& event) {
    std::lock_guard<std::mutex> lock(queueMutex);  // Lock the mutex for thread-safe access
    eventQueue.push(event);  // Add event to the queue
      // Debug statement
      std::cout <<". Queue size: " << eventQueue.size() << std::endl;
}

// Dispatch all queued events to their respective handlers.
void EventManager::dispatchEvents() {
    std::cout<<"Dispatching Event: "<<std::endl;
    std::lock_guard<std::mutex> lock(queueMutex);
    while (!eventQueue.empty()) {
        Event e = eventQueue.front();
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

// void EventManager::dispatchEvents() {
    // std::cout << "Dispatching Events..." << std::endl;

    // std::lock_guard<std::mutex> lock(queueMutex);  // Lock the mutex for thread-safe access

    // if (eventQueue.empty()) {
    //     std::cout << "Event queue is empty!" << std::endl;
    //     return;
    // }

    // while (!eventQueue.empty()) {
    //     Event e = eventQueue.front();
    //     eventQueue.pop();

    //     // Print event details (type and any other relevant information)
    //     std::cout << "Dispatching Event of type: " << e.type << ". Queue size after pop: " << eventQueue.size() << std::endl;

    //     // Dispatch logic (commented out for now)
    //     if (listeners.find(e.type) != listeners.end()) {
    //         for (auto& handler : listeners[e.type]) {
    //             handler->onEvent(e);  // Dispatch the event to all registered handlers
    //             std::cout << "Event dispatched to handler" << std::endl;  // Debug statement
    //         }
    //     } else {
    //         std::cout << "No listeners found for event type: " << e.type << std::endl;
    //     }
    // }
// }