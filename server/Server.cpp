#include <zmq.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include "../include/json.hpp"

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

    zmq::socket_t broadcaster(context, ZMQ_PUB);
    broadcaster.bind("tcp://*:5556");

    std::unordered_map<std::string, json> clientData;
    std::unordered_map<std::string, std::string> clientAddresses;

    std::cout << "Server has started and is listening on port 5555." << std::endl;

    while (true) {
        zmq::message_t request;
        receiver.recv(request, zmq::recv_flags::none);
        std::string clientDataString = request.to_string();

        json jsonData;
        try {
            jsonData = json::parse(clientDataString);

            if (jsonData.contains("disconnect") && jsonData["disconnect"]) {
                if (!jsonData.contains("clientId")) {
                    std::cerr << "Disconnect message missing clientId: " << clientDataString << std::endl;
                    receiver.send(zmq::message_t(0), zmq::send_flags::none);
                    continue;
                }

                std::string clientId = jsonData["clientId"];
                clientData.erase(clientId);
                clientAddresses.erase(clientId);
                std::cout << "SERVER MESS: " << clientId << " disconnected" << std::endl;

                json disconnectNotification = {{"disconnectedClientId", clientId}};
                std::string disconnectMessage = disconnectNotification.dump();
                zmq::message_t notification(disconnectMessage.size());
                memcpy(notification.data(), disconnectMessage.c_str(), disconnectMessage.size());
                broadcaster.send(notification, zmq::send_flags::none);
                receiver.send(zmq::message_t(0), zmq::send_flags::none);
                continue;
            }

            if (!jsonData.contains("clientId") || !jsonData.contains("clientAddr") || !jsonData.contains("entities")) {
                std::cerr << "Received incomplete data: " << clientDataString << std::endl;
                continue;
            }

            std::string clientId = jsonData["clientId"];
            std::string clientAddr = jsonData["clientAddr"];
            clientAddresses[clientId] = clientAddr;

            if (clientData.find(clientId) == clientData.end()) {
                std::cout << "New client connected with ID: " << clientId 
                          << " and Address: " << clientAddr << std::endl;

                json addressesJson = clientAddresses;
                std::string addressesString = addressesJson.dump();
                zmq::message_t addressUpdate(addressesString.size());
                memcpy(addressUpdate.data(), addressesString.c_str(), addressesString.size());
                broadcaster.send(addressUpdate, zmq::send_flags::none);
            }

            // Update the client's entities
            clientData[clientId] = jsonData["entities"];

        } catch (const json::parse_error& e) {
            std::cerr << "Parse error: " << e.what() << std::endl;
            continue;
        } catch (const json::type_error& e) {
            std::cerr << "Type error: " << e.what() << std::endl;
            continue;
        }

        // Prepare position updates
        json positionUpdates = json::array();
        for (const auto& [id, entities] : clientData) {
            positionUpdates.push_back({
                {"clientId", id},
                {"entities", entities}
            });
        }

        // Send the updated positions
        std::string replyData = positionUpdates.dump();
        zmq::message_t reply(replyData.size());
        memcpy(reply.data(), replyData.c_str(), replyData.size());
        receiver.send(reply, zmq::send_flags::none);
    }

    return 0;
}