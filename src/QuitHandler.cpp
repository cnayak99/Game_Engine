#include "QuitHandler.h"

// Handle quit-related events.
void QuitHandler::onEvent(const Event& e) {
    if (e.type == "quit") {
        std::cout << "Exiting game!" << std::endl;

        // concepts->quit = true;

        // // Send disconnect message using ZMQ.
        // json disconnectMessage = {
        //     {"clientId", clientId1},
        //     {"disconnect", true}
        // };
        // std::string messageString = disconnectMessage.dump();
        // zmq::message_t message(messageString.size());
        // memcpy(message.data(), messageString.c_str(), messageString.size());

        // if (receiverPtr) {
        //     receiverPtr->send(message, zmq::send_flags::none);
        // }

        // return;  // Exit after sending the disconnect message.
    }
}
