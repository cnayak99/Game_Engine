#ifndef QUITHANDLER_H
#define QUITHANDLER_H

#include "Event.h"
#include <iostream>
#include "structs.h"  // Include Concepts and Game structs
#include <zmq.hpp>
#include "json.hpp" // Use relative path to the include directory
#include "EventHandler.h"  // Include EventHandler first

using json = nlohmann::json;

// QuitHandler class to handle quitting-related events.
class QuitHandler : public EventHandler {
private:
    Concepts* concepts;  // Pointer to Concepts object
    Game* game;

public:
    QuitHandler(Concepts* c, Game* g)
        : concepts(c), game(g){}

    void onEvent(const Event& e) override;  // Ensure this matches the base class signature
};

#endif // QUITHANDLER_H
