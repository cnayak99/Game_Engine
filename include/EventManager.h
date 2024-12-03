#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <string>
#include "Event.h"  // Make sure to include the Event class definition
#include "EventHandler.h"

struct EventComparator {
    bool operator()(const Event& e1, const Event& e2) {
        return e1.timestamp > e2.timestamp;  // Min-heap: earlier timestamps have higher priority
    }
};

class EventManager {
private:
    std::unordered_map<std::string, std::vector<EventHandler*>> listeners;
    std::priority_queue<Event, std::vector<Event>, EventComparator> eventQueue; 
    std::mutex queueMutex;  // Protect access to the queue in a multithreaded environment

public:
    // Registers a listener for a specific event type
    void registerListener(const std::string& eventType, EventHandler* handler);
    
    // Raises an event, adding it to the priority queue
    void raiseEvent(const Event& event);
    
    // Dispatches events in the queue to the appropriate listeners
    void dispatchEvents();
};


#endif // EVENTMANAGER_H
