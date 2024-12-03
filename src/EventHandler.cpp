#include "Event.h"
#include "structs.h"
// EventHandler::EventHandler(Entity* source) : source(source) {
// };


// void PlayerHandler:: onEvent(Event e) {
//     switch (e.getType())
//     {
//     case 1:
//         // Collision Event
//         break;
//     case 2:
//         // Death Event
//         break;
//     case 3:
//         // Spawn Event
//         break;
//     case 4:
//         // Input Event
//         break;
//     default:
//         break;
//     }
// }

// Entity* PlayerHandler:: getSource() {
//     return source;
// }

// Handle input-related events.
// Handle input-related events.
// void InputHandler::onEvent(const Event& e) {
//     if (e.type == "input") {
//         int keyCode = e.parameters.at("keyCode").asInt;

//         if (keyCode == SDL_SCANCODE_UP) {
//             std::cout << "Move up!" << std::endl;
//             concepts->verticalVel = concepts->thrust;  // Move up logic
//         } else if (keyCode == SDL_SCANCODE_LEFT) {
//             std::cout << "Move left!" << std::endl;
//             concepts->c->move(-concepts->moveSpeed, 0);  // Move left logic
//         } else if (keyCode == SDL_SCANCODE_RIGHT) {
//             std::cout << "Move right!" << std::endl;
//             concepts->c->move(concepts->moveSpeed, 0);   // Move right logic
//         } else if (keyCode == SDL_SCANCODE_C) {
//             std::cout << "Change window size!" << std::endl;
            
//             if (!concepts->held) {
//                 concepts->held = true;  // Prevent multiple presses

//                 if (!concepts->scaling) {
//                     SDL_RenderSetLogicalSize(game->renderer, 1920, 1080);
//                     concepts->scaling = true;  // Enable scaling
//                 } else {
//                     SDL_RenderSetLogicalSize(game->renderer, 0, 0);
//                     concepts->scaling = false;  // Disable scaling
//                 }
//             }
//         }
//     }
// }

// void InputHandler::onEvent(const Event& e) {
//     if (e.type == "input") {
//         int keyCode = e.parameters.at("keyCode").asInt;

//         if (keyCode == SDL_SCANCODE_UP) {
//             std::cout << "Move up!" << std::endl;
//             concepts->verticalVel = concepts->thrust;  // Move up logic
//         } else if (keyCode == SDL_SCANCODE_LEFT) {
//             std::cout << "Move left!" << std::endl;
//             concepts->c->move(-concepts->moveSpeed, 0);  // Move left logic
//         } else if (keyCode == SDL_SCANCODE_RIGHT) {
//             std::cout << "Move right!" << std::endl;
//             concepts->c->move(concepts->moveSpeed, 0);   // Move right logic
//         } else if (keyCode == SDL_SCANCODE_C) {
//             std::cout << "Change window size!" << std::endl;
            
//             if (!concepts->held) {
//                 concepts->held = true;  // Prevent multiple presses

//                 if (!concepts->scaling) {
//                     SDL_RenderSetLogicalSize(game->renderer, 1920, 1080);
//                     concepts->scaling = true;  // Enable scaling
//                 } else {
//                     SDL_RenderSetLogicalSize(game->renderer, 0, 0);
//                     concepts->scaling = false;  // Disable scaling
//                 }
//             }
//         }
//     }
// }


// void QuitHandler::onEvent(const Event& e) {
//     if (e.type == "quit") {
//         std::cout << "Exiting game!" << std::endl;

//         concepts->quit = true;

//         // Send disconnect message using ZMQ.
//         json disconnectMessage = {
//             {"clientId", clientId1},
//             {"disconnect", true}
//         };
//         std::string messageString = disconnectMessage.dump();
//         zmq::message_t message(messageString.size());
//         memcpy(message.data(), messageString.c_str(), messageString.size());

//         if (receiverPtr) {
//             receiverPtr->send(message, zmq::send_flags::none);
//         }

//         return;  // Exit after sending the disconnect message.
//     }
// }

// void QuitHandler::onEvent(const Event& e) {
//     if (e.type == "quit") {
//         std::cout << "Exiting game!" << std::endl;

//         concepts.quit = true;  // Access `quit` directly on the `concepts` object

//         // Send disconnect message using ZMQ.
//         json disconnectMessage = {
//             {"clientId", clientId1},
//             {"disconnect", true}
//         };
//         std::string messageString = disconnectMessage.dump();
//         zmq::message_t message(messageString.size());
//         memcpy(message.data(), messageString.c_str(), messageString.size());

//         if (receiverPtr) {
//             receiverPtr->send(message, zmq::send_flags::none);
//         }

//         return;  // Exit after sending the disconnect message.
//     }
// }
