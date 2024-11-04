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
    zmq::socket_t* receiverPtr;  // Pointer to ZMQ receiver socket
    std::string clientId1;

public:
    QuitHandler(Concepts* c, zmq::socket_t* rcvPtr, const std::string& clientId)
        : concepts(c), receiverPtr(rcvPtr), clientId1(clientId) {}

    void onEvent(const Event& e) override;  // Ensure this matches the base class signature
};

#endif // QUITHANDLER_H
