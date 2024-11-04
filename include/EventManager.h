#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <string>
#include "Event.h"  // Make sure to include the Event class definition
#include "EventHandler.h"
class EventManager {
private:
    std::unordered_map<std::string, std::vector<EventHandler*>> listeners;
    std::queue<Event> eventQueue; 
    std::mutex queueMutex;  // Protect access to the queue in a multithreaded environment

public:
    // Registers a listener for a specific event type
    void registerListener(const std::string& eventType, EventHandler* handler);
    
    // Raises an event, adding it to the event queue
    void raiseEvent(const Event& event);
    
    // Dispatches events in the queue to the appropriate listeners
    void dispatchEvents();
};

#endif // EVENTMANAGER_H
