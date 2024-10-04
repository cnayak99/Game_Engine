#include <zmq.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include "../include/json.hpp"

// Use the nlohmann namespace for convenience
using json = nlohmann::json;
/*
This code has been referred from the zeromq official wesite
Links:
1) https://zguide.zeromq.org/docs/chapter3/
2) https://zguide.zeromq.org/docs/chapter5/
3) http://wiki.zeromq.org/tutorials:dealer-and-router
*/
void broadcast(zmq::socket_t& socket, const std::string& message) {
    zmq::message_t broadcastMessage(message.size());
    memcpy(broadcastMessage.data(), message.c_str(), message.size());
    socket.send(broadcastMessage, zmq::send_flags::none);
}

int main() {
    zmq::context_t context(1);
    zmq::socket_t receiver(context, ZMQ_REP);
    receiver.bind("tcp://*:5555");

    // Create a single broadcaster socket for publishing updates
    zmq::socket_t broadcaster(context, ZMQ_PUB);
    broadcaster.bind("tcp://*:5556");

    // Map to store client positions and addresses
    std::unordered_map<std::string, std::tuple<int, int, std::string>> clientData;
    std::unordered_map<std::string, std::string> clientAddresses;

    std::cout << "Server has started and is listening on port 5555." << std::endl;

    while (true) {
        zmq::message_t request;
        receiver.recv(request, zmq::recv_flags::none);
        std::string clientDataString = request.to_string();

        // Parse the received JSON data
        json jsonData;
        try {
            jsonData = json::parse(clientDataString);

            // Extract client ID, address, and position from the JSON
            std::string clientId = jsonData["clientId"];
            std::string clientAddr = jsonData["clientAddr"];
            int x = jsonData["x"];
            int y = jsonData["y"];

            // Check if this client ID is unique
            if (clientData.find(clientId) == clientData.end()) {
                std::cout << "New client connected with ID: " << clientId 
                          << " and Address: " << clientAddr << std::endl;
                clientAddresses[clientId] = clientAddr;

                // Convert the updated addresses map to JSON
                json addressesJson = clientAddresses;
                std::string addressesString = addressesJson.dump();

                // Send the updated addresses to all clients
                zmq::message_t addressUpdate(addressesString.size());
                memcpy(addressUpdate.data(), addressesString.c_str(), addressesString.size());
                broadcaster.send(addressUpdate, zmq::send_flags::none);
            }

            // Update the client's position and address in the map
            clientData[clientId] = std::make_tuple(x, y, clientAddr);
        } catch (const json::parse_error& e) {
            std::cerr << "Parse error: " << e.what() << std::endl;
            continue;
        }

        // Prepare the JSON response
        json positionUpdates = json::array();
        for (const auto& [id, data] : clientData) {
            positionUpdates.push_back({
                {"clientId", id},
                {"clientAddr", std::get<2>(data)},
                {"position", {{"x", std::get<0>(data)}, {"y", std::get<1>(data)}}}
            });
        }

        // Convert the response to string and send it
        std::string replyData = positionUpdates.dump();
        zmq::message_t reply(replyData.size());
        memcpy(reply.data(), replyData.c_str(), replyData.size());
        receiver.send(reply, zmq::send_flags::none);
    }

    return 0;
}