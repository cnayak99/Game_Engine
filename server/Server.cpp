// #include <zmq.hpp>
// #include <iostream>
// #include <string>
// #include <unordered_map>
// #include <unordered_set> // For tracking unique clients
// #include <vector>

// void broadcast(zmq::socket_t& socket, const std::string& message) {
//     zmq::message_t broadcastMessage(message.size());
//     memcpy(broadcastMessage.data(), message.c_str(), message.size());
//     socket.send(broadcastMessage, zmq::send_flags::none);
// }

// int main() {
//     zmq::context_t context(1);
//     zmq::socket_t receiver(context, ZMQ_REP); // To receive position updates
//     zmq::socket_t broadcaster(context, ZMQ_PUB); // To broadcast updates
//     receiver.bind("tcp://*:5555");
//     broadcaster.bind("tcp://*:5556");

//     // Print message indicating the server has started
//     std::cout << "Server has started and is listening on port 5555." << std::endl;

//     // Store client positions. Key is a string (clientId), value is a pair of integers (x, y positions)
//     std::unordered_map<std::string, std::pair<int, int>> clientPositions;

//     // Store unique client IDs
//     std::unordered_set<std::string> uniqueClients;

//     while (true) {
//         zmq::message_t request;
//         receiver.recv(request, zmq::recv_flags::none);

//         std::string clientData = request.to_string();
        
//         // Extract client ID and position from the received message
//         size_t colonPos = clientData.find(':');
//         if (colonPos != std::string::npos) {
//             std::string clientId = clientData.substr(0, colonPos); // Extract clientId
//             std::string coordinates = clientData.substr(colonPos + 1); // Skip ": "

//             size_t delimiterPos = coordinates.find(',');
//             if (delimiterPos != std::string::npos) {
//                 int x = std::stoi(coordinates.substr(0, delimiterPos)); // X coordinate
//                 int y = std::stoi(coordinates.substr(delimiterPos + 1)); // Y coordinate

//                 // Update the client's position in the map
//                 clientPositions[clientId] = {x, y}; // Use clientId as the map key

//                 // Check if the client is connecting for the first time
//                 if (uniqueClients.find(clientId) == uniqueClients.end()) {
//                     // New client detected, print client ID
//                     std::cout << "New client connected: " << clientId << std::endl;

//                     // Add clientId to the set of unique clients
//                     uniqueClients.insert(clientId);
//                 }

//                 // Prepare a message to broadcast
//                 std::string broadcastMessage;
//                 for (const auto& [id, pos] : clientPositions) {
//                     broadcastMessage += id + ":" + std::to_string(pos.first) + "," + std::to_string(pos.second) + ";";
//                 }

//                 // Broadcast the updated positions to all subscribers
//                 broadcast(broadcaster, broadcastMessage);
//             }
//         }

//         // Optionally reply back to the client (you might want to remove this if not needed)
//         std::string replyMessage = "Position updated.";
//         zmq::message_t reply(replyMessage.size());
//         memcpy(reply.data(), replyMessage.c_str(), replyMessage.size());
//         receiver.send(reply, zmq::send_flags::none);
//     }

//     return 0;
// }
#include <zmq.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

void broadcast(zmq::socket_t& socket, const std::string& message) {
    zmq::message_t broadcastMessage(message.size());
    memcpy(broadcastMessage.data(), message.c_str(), message.size());
    socket.send(broadcastMessage, zmq::send_flags::none);
}

int main() {
    zmq::context_t context(1);
    zmq::socket_t receiver(context, ZMQ_REP);
    receiver.bind("tcp://*:5555");

    // Print message indicating the server has started
    std::cout << "Server has started and is listening on port 5555." << std::endl;

    // Store client positions. Key is a string (clientId), value is a pair of integers (x, y positions)
    std::unordered_map<std::string, std::pair<int, int>> clientPositions;

    while (true) {
        zmq::message_t request;
        receiver.recv(request, zmq::recv_flags::none);
        // std::string receivedData = request.to_string();

        // size_t delimiterPos = receivedData.find(",");
        // int x = std::stoi(receivedData.substr(0, delimiterPos));
        // int y = std::stoi(receivedData.substr(delimiterPos + 1));

        // // Check if this client ID is new
        // if (clientPositions.find(clientId) == clientPositions.end()) {
        //     // Print message indicating a new client has connected
        //     std::cout << "New client connected with ID: " << clientId << std::endl;
        //     clientPositions[clientId] = {x, y};
        //     ++clientId;
        // } else {
        //     clientPositions[clientId] = {x, y};
        // }

        std::string clientData = request.to_string();
        
        // Extract client ID and position from the received message
        size_t colonPos = clientData.find(':');
        if (colonPos != std::string::npos) {
            std::string clientId = clientData.substr(0, colonPos); // Extract clientId
            std::string coordinates = clientData.substr(colonPos + 1); // Skip ": "
            
            size_t delimiterPos = coordinates.find(',');
            if (delimiterPos != std::string::npos) {
                int x = std::stoi(coordinates.substr(0, delimiterPos)); // X coordinate
                int y = std::stoi(coordinates.substr(delimiterPos + 1)); // Y coordinate
                
                // Check if this client ID is unique
                if (clientPositions.find(clientId) == clientPositions.end()) {
                    std::cout << "New client connected with ID: " << clientId << std::endl;
                }

                // Update the client's position in the map
                clientPositions[clientId] = {x, y}; // Use clientId as the map key
            }
        }

        std::string positionUpdates;
        for (const auto& [id, pos] : clientPositions) {
            positionUpdates += "Client " + id + ": (" + 
                               std::to_string(pos.first) + ", " + 
                               std::to_string(pos.second) + ")\n";
        }

        zmq::message_t reply(positionUpdates.size());
        memcpy(reply.data(), positionUpdates.c_str(), positionUpdates.size());
        receiver.send(reply, zmq::send_flags::none);
    }

    return 0;
}