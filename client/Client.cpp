#include <zmq.hpp>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <cstdlib> // For srand() and rand()
#include <ctime>   // For time()

// Function to handle messages from the server
void listenForMessages(zmq::socket_t& dealerSocket, std::unordered_map<std::string, std::string>& knownPeers) {
    while (true) {
        zmq::message_t message;
        dealerSocket.recv(message, zmq::recv_flags::none);
        std::string msgStr(static_cast<char*>(message.data()), message.size());

        // Handle different types of messages
        if (msgStr.find("NEW_PEER") == 0) {
            std::string newPeerID = msgStr.substr(9); // Extract peer ID from message
            if (knownPeers.find(newPeerID) == knownPeers.end()) {
                knownPeers[newPeerID] = "localhost"; // Store new peer info
                std::cout << "New peer connected: " << newPeerID << std::endl;
            }
        } else if (msgStr.find("EXISTING_PEER") == 0) {
            std::string existingPeerID = msgStr.substr(14); // Extract peer ID from message
            if (knownPeers.find(existingPeerID) == knownPeers.end()) {
                knownPeers[existingPeerID] = "localhost"; // Store existing peer info
                std::cout << "Existing peer: " << existingPeerID << std::endl;
            }
        } else {
            std::cout << "Received message: " << msgStr << std::endl;
        }

        // Print out all known peers
        std::cout << "Known peers: ";
        for (const auto& peer : knownPeers) {
            std::cout << peer.first << " ";
        }
        std::cout << std::endl;
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