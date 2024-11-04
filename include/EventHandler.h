#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "Event.h"  // Include the Event class definition

class EventHandler {
public:
    // Pure virtual function to handle events
    virtual void onEvent(const Event& e) = 0;

    // Virtual destructor to ensure proper cleanup of derived classes
    virtual ~EventHandler() {}
};

#endif // EVENTHANDLER_H
