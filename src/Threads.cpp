#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <vector>
#include <zmq.hpp>
#include "Timeline.h"
#include "Entity.h"
#include "Rectangle.h"
#include "Intersect.h"
#include "structs.h"
#include "Threads.h"
#include "Event.h"
#include "json.hpp" // Use relative path to the include directory
#include "InputHandler.h"
#include "QuitHandler.h"
#include "EventManager.h"
#include "EventHandler.h" 
using namespace std; 
using json = nlohmann::json;
std::mutex *_mutex; // The object for mutual exclusion of execution.
std::condition_variable *_cv_c; // For thread communication regarding the controllable entity.
Timeline *time_Threads; // To keep track of the timeline for these threads.
Concepts *concepts; // To keep track of initialized variables.
Game *game; // To keep track of game variables.
string clientId1;
zmq::socket_t* receiverPtr = nullptr;
/**
 * Runs both initialized threads through different functions.
 * 
 * This function is heavily inspired from the ThreadExample.run()
 * function provided by Professor Card on the "CSC 481/581 (001) Fall
 * 2024 Game Engine Foundations" course Moodle page underneath
 * the "Homework 2" subtitle. It is accessible by clicking on the
 * "ThreadExample.cpp" link. It was suggested to "build upon
 * the ThreadExample provided on Moodle" for this project according
 * to Section 4 of the Homework 2 Assignment PDF
 * "CSC 481-581 HW 2-2.pdf", which is accessible also underneath
 * the "Homework 2" subtitle on the "CSC 481/581 (001) Fall 2024
 * Game Engine Foundations" course Moodle page.
 */


void runPhysics();
void runInput(EventManager& eventManager);
void runThread(int id, EventManager& eventManager);

void runThread(int id, EventManager& eventManager) {
    if (id == 0) {
        runPhysics();
    } else {
        runInput(eventManager);
    }
}

/**
 * Handles motion physics for our program.
 * 
 * This function is heavily inspired from the ThreadExample.run()
 * function provided by Professor Card on the "CSC 481/581 (001) Fall
 * 2024 Game Engine Foundations" course Moodle page underneath
 * the "Homework 2" subtitle. It is accessible by clicking on the
 * "ThreadExample.cpp" link. It was suggested to "build upon
 * the ThreadExample provided on Moodle" for this project according
 * to Section 4 of the Homework 2 Assignment PDF
 * "CSC 481-581 HW 2-2.pdf", which is accessible also underneath
 * the "Homework 2" subtitle on the "CSC 481/581 (001) Fall 2024
 * Game Engine Foundations" course Moodle page.
 */
void runPhysics() {
    // Thread 1 tries to keep track of non-player rectangles for this tic.
    try
    {
        //if (!concepts->a->isPaused) {}

            // This block controls the physics for the Controllable Entity.
            { // Anonymous block to manage mutex lock scope.
                // Sets up the mutex lock.
                std::unique_lock<std::mutex> cv_lock(*_mutex);

                // Applies gravity to the player object.
                concepts->verticalVel += concepts->gravity * concepts->delta;
                concepts->c->move(0, static_cast<int>(concepts->verticalVel));
                        
                // Notifies all that the controllable entity had physics applied.
                _cv_c->notify_all();
            }

            // Controls physics for the Moving Entity.
            // Moves the moving shape in a continuous horizontal pattern.
            concepts->m->move(concepts->speed, 0);
            if (concepts->m->getRect().x > 1820 || concepts->m->getRect().x < 100) {
                concepts->speed = -concepts->speed;
            }

            // Controls physics for the Vertically Moving Entity.
            // Moves the moving shape in a continuous horizontal pattern.
            concepts->v->move(0, concepts->speed);
            if (concepts->v->getRect().y > 500 || concepts->v->getRect().y < 100) {
                concepts->speed = -concepts->speed;
            }
        //}

        // Add additional blocks to control additional shapes here if necessary.
    }
    catch (...)
    {
        std::cerr << "Thread 1 caught exception." << std::endl;
    }
}

/**
 * Handles player input for our program.
 * 
 * This function is heavily inspired from the ThreadExample.run()
 * function provided by Professor Card on the "CSC 481/581 (001) Fall
 * 2024 Game Engine Foundations" course Moodle page underneath
 * the "Homework 2" subtitle. It is accessible by clicking on the
 * "ThreadExample.cpp" link. It was suggested to "build upon
 * the ThreadExample provided on Moodle" for this project according
 * to Section 4 of the Homework 2 Assignment PDF
 * "CSC 481-581 HW 2-2.pdf", which is accessible also underneath
 * the "Homework 2" subtitle on the "CSC 481/581 (001) Fall 2024
 * Game Engine Foundations" course Moodle page.
 */
void runInput(EventManager& eventManager) {
    // Thread 2 tries to manage player input for this tic.
    try 
    {
        // Sets up the mutex lock.
        std::unique_lock<std::mutex> cv_lock(*_mutex);
        if (concepts->state[SDL_SCANCODE_UP]) {  // Move up
            int64_t currentTimestamp = time_Threads->getTimeline();  // Get current time from Timeline

            Event inputEvent("input", currentTimestamp);  // Create an event with a timestamp

            Variant keyCode;
            keyCode.type = Variant::TYPE_INT;
            keyCode.asInt = SDL_SCANCODE_UP;
            inputEvent.parameters["keyCode"] = keyCode;

            // std::cout << "UP pressed" << std::endl;
            eventManager.raiseEvent(inputEvent);  // Raise the event with timestamp
        }

        if (concepts->state[SDL_SCANCODE_DOWN]) {  // Crouch
            //Create and Raise Events
            int64_t currentTimestamp = time_Threads->getTimeline();  // Get current time from Timeline

            Event inputEvent("input", currentTimestamp);

            Variant keyCode;
            keyCode.type = Variant::TYPE_INT;
            keyCode.asInt = SDL_SCANCODE_DOWN;
            inputEvent.parameters["keyCode"] = keyCode;

            // std::cout << "LEFT pressed" << std::endl;
            eventManager.raiseEvent(inputEvent);
        }

        if (concepts->state[SDL_SCANCODE_LEFT] && !concepts->state[SDL_SCANCODE_DOWN]) {  // Move left
            //Create and Raise Events
            int64_t currentTimestamp = time_Threads->getTimeline();  // Get current time from Timeline

            Event inputEvent("input", currentTimestamp);

            Variant keyCode;
            keyCode.type = Variant::TYPE_INT;
            keyCode.asInt = SDL_SCANCODE_LEFT;
            inputEvent.parameters["keyCode"] = keyCode;

            // std::cout << "LEFT pressed" << std::endl;
            eventManager.raiseEvent(inputEvent);
        }

        if (concepts->state[SDL_SCANCODE_RIGHT] && !concepts->state[SDL_SCANCODE_DOWN]) {  // Move right
            //Create and Raise Events
            int64_t currentTimestamp = time_Threads->getTimeline();  // Get current time from Timeline

            Event inputEvent("input", currentTimestamp);

            Variant keyCode;
            keyCode.type = Variant::TYPE_INT;
            keyCode.asInt = SDL_SCANCODE_RIGHT;
            inputEvent.parameters["keyCode"] = keyCode;

            // std::cout << "RIGHT pressed" << std::endl;
            eventManager.raiseEvent(inputEvent);
        }
    
        // If the player is pressing 'C'.
        if (concepts->state[SDL_SCANCODE_C]) { // Change window size.
            //Create and Raise Events
            int64_t currentTimestamp = time_Threads->getTimeline();  // Get current time from Timeline

            Event inputEvent("input", currentTimestamp);

            Variant keyCode;
            keyCode.type = Variant::TYPE_INT;
            keyCode.asInt = SDL_SCANCODE_C;
            inputEvent.parameters["keyCode"] = keyCode;

            // std::cout << "C pressed" << std::endl;
            eventManager.raiseEvent(inputEvent);
        }
        else {
            concepts->held = false;
        }

        // If the player is pressing 'ESC'.
        if (concepts->state[SDL_SCANCODE_ESCAPE]) { // Exit the game.
            //Create and Raise Events
            int64_t currentTimestamp = time_Threads->getTimeline();  // Get current time from Timeline
            Event quitEvent("quit", currentTimestamp);
            eventManager.raiseEvent(quitEvent);
        }

        // Notifies Thread 1.
        _cv_c->notify_all();
    }
    catch (...)
    {
        std::cerr << "Thread 2 caught exception." << std::endl;
    }
}

/**
 * Runs the two threads.
 * 
 * This function is heavily inspired from the ThreadExample class
 * provided by Professor Card on the "CSC 481/581 (001) Fall
 * 2024 Game Engine Foundations" course Moodle page underneath
 * the "Homework 2" subtitle. It is accessible by clicking on the
 * "ThreadExample.cpp" link. It was suggested to "build upon
 * the ThreadExample provided on Moodle" for this project according
 * to Section 4 of the Homework 2 Assignment PDF
 * "CSC 481-581 HW 2-2.pdf", which is accessible also underneath
 * the "Homework 2" subtitle on the "CSC 481/581 (001) Fall 2024
 * Game Engine Foundations" course Moodle page.
 * 
 * The following websites are where we learned about their
 * respective functions. We learned about the <vector> header,
 * including the push_back() function, from
 * https://en.cppreference.com/w/cpp/container/vector. More details
 * on the push_back() function were located from
 * https://en.cppreference.com/w/cpp/container/vector/push_back.
 * Because the threads no longer were accessible after they were
 * executed, we also looked into using the (auto& : container)
 * for loop structure for the sake of joining them back together,
 * before the end of the program. One of the team members (Lillie
 * Sharpe) has used this concept before, but to refresh ourselves,
 * we viewed more information about it from
 * https://en.cppreference.com/w/cpp/language/range-for#:~:text=Range-based%20loop%20(since%20C++11)%20{statement...%20Executes%20a%20for%20loop%20over,
 * which included studying the concept of temporary entities
 * (like our std::threads) within the same webpage.
 */
int startThreads(Timeline* t, Concepts* c, Game* g, zmq::socket_t& receiver, std::string& clientId, EventManager &eventManager) {
    std::mutex m;
    _mutex = &m;
    std::condition_variable cv_c;
    _cv_c = &cv_c;

    std::vector<std::thread> both;
    time_Threads = t;
    concepts = c;
    game = g;
    clientId1 = clientId;
    receiverPtr = &receiver;

    for (int i = 0; i < 2; i++) {
        both.push_back(std::thread([i, &eventManager]() {
            runThread(i, eventManager);
        }));
    }

    for (auto& th : both) {
        th.join();
    }

    return 0;
}