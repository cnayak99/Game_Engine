#include <zmq.hpp>
#include <iostream>
#include <unordered_map>
#include <thread>
#include <string>

// Global map to store connected clients' identities
std::unordered_map<std::string, std::string> connectedClients;

void handleClient(zmq::socket_t& routerSocket) {
    while (true) {
        zmq::message_t identity;
        zmq::message_t emptyMsg;
        zmq::message_t joinMsg;

        // Receive identity of the client
        routerSocket.recv(identity, zmq::recv_flags::none);

        // Receive the empty frame
        routerSocket.recv(emptyMsg, zmq::recv_flags::none);

        // Receive the actual message
        routerSocket.recv(joinMsg, zmq::recv_flags::none);

        std::string identityStr(static_cast<char*>(identity.data()), identity.size());
        std::string joinMsgStr(static_cast<char*>(joinMsg.data()), joinMsg.size());

        if (joinMsgStr == "JOIN") {
            std::cout << "Client joined: " << identityStr << std::endl;

            // Add new client to the list
            connectedClients[identityStr] = "localhost";
            // Send the list of existing clients to the new client
            for (const auto& client : connectedClients) {
                // Don't send the new client itself
                if (client.first != identityStr) { 
                    zmq::message_t existingPeerMsg("EXISTING_PEER " + client.first);
                    // std::cout << "Client Existing: " << client.first << std::endl;
                    routerSocket.send(identity, zmq::send_flags::sndmore); // Send to the new client
                    routerSocket.send(emptyMsg, zmq::send_flags::sndmore); 
                    routerSocket.send(existingPeerMsg, zmq::send_flags::none); 
                }
            }

            // Notify all clients of the new client
            for (const auto& client : connectedClients) {
                if (client.first != identityStr) { // Send to all other clients except the new one
                    zmq::message_t newPeerMsg("NEW_PEER " + identityStr);
                    routerSocket.send(zmq::buffer(client.first), zmq::send_flags::sndmore); // Send to other clients
                    routerSocket.send(emptyMsg, zmq::send_flags::sndmore);
                    routerSocket.send(newPeerMsg, zmq::send_flags::none);
                }
            }
        }
    }
}

int main() {
    zmq::context_t context(1);
    zmq::socket_t routerSocket(context, zmq::socket_type::router);
    routerSocket.bind("tcp://*:5556"); // Bind to port 5556

    std::thread clientHandler(handleClient, std::ref(routerSocket));
    clientHandler.join();

    return 0;
}