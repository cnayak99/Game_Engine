#include <zmq.hpp>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <cstdlib> // For srand() and rand()
#include <ctime>   // For time()

// Function to handle messages from the server
void listenForUpdates(zmq::socket_t& subscriberSocket, std::vector<zmq::socket_t>& peerSockets, zmq::context_t& context) {
    while (true) {
        zmq::message_t update;
        subscriberSocket.recv(update, zmq::recv_flags::none);
        std::string updateStr(static_cast<char*>(update.data()), update.size());

        // Parse the received JSON data
        try {
            auto updatedClientAddresses = json::parse(updateStr);
            std::cout << "Received updated client addresses:" << std::endl;
            for (auto& [id, addr] : updatedClientAddresses.items()) {
                std::cout << id << ": " << addr << std::endl;

                // Connect to new peers using PAIR sockets
                zmq::socket_t peerSocket(context, ZMQ_PAIR);
                peerSocket.connect(addr);
                peerSockets.push_back(std::move(peerSocket));

                // Send a connection request
                zmq::message_t connectMsg("CONNECT");
                peerSockets.back().send(connectMsg, zmq::send_flags::none);
            }
        } catch (const json::parse_error& e) {
            std::cerr << "Parse error: " << e.what() << std::endl;
        }
    }
}

void listenForPeers(zmq::socket_t& pairSocket) {
    while (true) {
        zmq::message_t message;
        pairSocket.recv(message, zmq::recv_flags::none);
        std::string msgStr(static_cast<char*>(message.data()), message.size());

        if (msgStr == "CONNECT") {
            std::cout << "Received connection request from peer" << std::endl;

            // Send acknowledgment back
            zmq::message_t ackMsg("ACK");
            pairSocket.send(ackMsg, zmq::send_flags::none);
        } else if (msgStr == "ACK") {
            std::cout << "Received acknowledgment from peer" << std::endl;
        }
    }
}

int main() {
    zmq::context_t context(1);
    zmq::socket_t dealerSocket(context, zmq::socket_type::dealer);

    // Set a unique ID for each client to identify them on the server
    srand(static_cast<unsigned int>(time(0))); // Use srand to seed the random number generator
    int randomNum = rand() % 10000; // Generate a random number between 0 and 9999
    std::string clientId = "client" + std::to_string(randomNum);  // Random client ID
    dealerSocket.setsockopt(ZMQ_IDENTITY, clientId.c_str(), clientId.size()); // Set identity
    dealerSocket.connect("tcp://localhost:5556"); // Connect to port 5556

    std::unordered_map<std::string, std::string> knownPeers;

    // Proper message framing for ROUTER/DEALER pattern
    zmq::message_t emptyMsg("", 0);
    zmq::message_t joinMsg("JOIN", 4);

    // Send client identity (automatically included by DEALER)
    dealerSocket.send(emptyMsg, zmq::send_flags::sndmore); // Empty frame for ROUTER
    dealerSocket.send(joinMsg, zmq::send_flags::none);     // Actual message

    // Start listening for messages in a separate thread
    std::thread listener(listenForMessages, std::ref(dealerSocket), std::ref(knownPeers));

    // Main thread can handle user input or sending other messages if needed
    listener.join();

    return 0;
}