#include <zmq.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include "../include/json.hpp"

// Use the nlohmann namespace for convenience
using json = nlohmann::json;

void broadcast(zmq::socket_t& socket, const std::string& message) {
    zmq::message_t broadcastMessage(message.size());
    memcpy(broadcastMessage.data(), message.c_str(), message.size());
    socket.send(broadcastMessage, zmq::send_flags::none);
}

int main() {
    zmq::context_t context(1);
    zmq::socket_t receiver(context, ZMQ_REP);
    receiver.bind("tcp://*:5555");

    // Map to store client positions. Key is a string (clientId), value is a pair of integers (x, y positions)
    std::unordered_map<std::string, std::pair<int, int>> clientPositions;

    // Print message indicating the server has started
    std::cout << "Server has started and is listening on port 5555." << std::endl;

    while (true) {
        zmq::message_t request;
        receiver.recv(request, zmq::recv_flags::none);
        std::string clientData = request.to_string();

        // Parse the received JSON data
        json jsonData;
        try {
            jsonData = json::parse(clientData); // Parse the incoming JSON

            // Extract client ID and position from the JSON
            std::string clientId = jsonData["clientId"];
            int x = jsonData["x"];
            int y = jsonData["y"];

            // Check if this client ID is unique
            if (clientPositions.find(clientId) == clientPositions.end()) {
                std::cout << "New client connected with ID: " << clientId << std::endl;
            }

            // Update the client's position in the map
            clientPositions[clientId] = {x, y}; // Use clientId as the map key
        } catch (const json::parse_error& e) {
            std::cerr << "Parse error: " << e.what() << std::endl;
            continue; // Skip to the next iteration if parsing fails
        }

        // Prepare the JSON response
        json positionUpdates = json::array();
        for (const auto& [id, pos] : clientPositions) {
            positionUpdates.push_back({
                {"clientId", id},
                {"position", {{"x", pos.first}, {"y", pos.second}}}
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
