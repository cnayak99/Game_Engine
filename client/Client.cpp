#include <zmq.hpp>
#include <string>
#include <iostream>

int main() {
    zmq::context_t context(1);

    // REQ socket for registration
    zmq::socket_t requester(context, ZMQ_REQ);
    requester.connect("tcp://localhost:5556");

    // Send registration request to the server
    std::string clientMessage = "Client requesting registration";
    zmq::message_t request(clientMessage.size());
    memcpy(request.data(), clientMessage.c_str(), clientMessage.size());
    requester.send(request, zmq::send_flags::none);

    // Receive client ID from the server
    zmq::message_t reply;
    requester.recv(reply);
    std::string received(static_cast<char*>(reply.data()), reply.size());
    std::cout << "Received: " << received << std::endl;

    // Switch to SUB socket for updates
    zmq::socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect("tcp://localhost:5555");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    while (true) {
        zmq::message_t update;
        subscriber.recv(update);
        std::string updateMessage(static_cast<char*>(update.data()), update.size());
        std::cout << "Update received: " << updateMessage << std::endl;
    }

    return 0;
}
