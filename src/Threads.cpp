#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <vector>

#include "Timeline.h"
#include "Entity.h"
#include "Rectangle.h"
#include "Intersect.h"
#include "structs.h"
#include "Threads.h"

std::mutex *_mutex; // the object for mutual exclusion of execution
std::condition_variable *_cv_c; // For thread communication regarding the controllable entity.
Timeline *time_Threads; // To keep track of the timeline for these threads.
Concepts *concepts; // To keep track of initialized variables.
Game *game; // To keep track of game variables.

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
void runThread(int id) {
    if (id == 0) {
        runPhysics();
    } else {
        runInput();
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
    printf("Thread 1 start.");

    // Thread 1 tries to keep track of non-player rectangles for this tic.
    try
    {
        printf("Thread 1 trying.");

        //if (!concepts->a->isPaused) {}

            // This block controls the physics for the Controllable Entity.
            { // Anonymous block to manage mutex lock scope.
                // Sets up the mutex lock.
                std::unique_lock<std::mutex> cv_lock(*_mutex);

                // Applies gravity to the player object.
                concepts->verticalVel += concepts->gravity * concepts->delta;
                concepts->c->move(0, static_cast<int>(concepts->verticalVel));
                        
                printf("Thread 1 applied gravity.");

                // Notifies all that the controllable entity had physics applied.
                _cv_c->notify_all();
            }

            // Controls physics for the Moving Entity.
            // Moves the moving shape in a continuous horizontal pattern.
            concepts->m->move(concepts->speed, 0);
            if (concepts->m->getRect().x > 1820 || concepts->m->getRect().x < 100) {
                concepts->speed = -concepts->speed;
            }
        //}

        printf("Thread 1 moved Entity M.");

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
void runInput() {
    printf("Start Thread 2.");

    // Thread 2 tries to manage player input for this tic.
    try 
    {
        // Sets up the mutex lock.
        std::unique_lock<std::mutex> cv_lock(*_mutex);

        printf("Thread 2 is processing player input.");

        // If the player is pressing 'P'.
        if (concepts->state[SDL_SCANCODE_P]) { // Pause game.
            if (!concepts->held) {
                concepts->held = true;
                if (!concepts->a->isPaused) {
                    time_Threads->pause();
                }
                else {
                    time_Threads->unpause();
                }
            }
        }

        //if (!concepts->a->isPaused) {

            // If the player is pressing 'B'.
            if(concepts->state[SDL_SCANCODE_B]){ // Set tic to 0.5 (which is marked with 3).
                time_Threads->setTicks(3);
                printf("Tics set to 0.5.\n");
            }

            // If the player is pressing 'N'.
            if(concepts->state[SDL_SCANCODE_N]){ // Set tic to 1.
                time_Threads->setTicks(1);
                printf("Tics set to 1.\n");
            }

            // If the player is pressing 'M'.
            if(concepts->state[SDL_SCANCODE_M]){ // Set tic to 2.
                time_Threads->setTicks(2);
                printf("Tics set to 2.\n");
            }

            // If the player is pressing up.
            if(concepts->state[SDL_SCANCODE_UP]){ // Move up.
                concepts->verticalVel = concepts->thrust;
            }

            // If the player is pressing left.
            if(concepts->state[SDL_SCANCODE_LEFT]){ // Move left.
                concepts->c->move(-concepts->moveSpeed,0);
            }

            // If the player is pressing right.
            if(concepts->state[SDL_SCANCODE_RIGHT]){// Move right.
                concepts->c->move(concepts->moveSpeed, 0);
            }
        
            // If the player is pressing 'C'.
            if (concepts->state[SDL_SCANCODE_C]) { // Change window size.
                if (!concepts->held) {
                    concepts->held = true;
                    if (!concepts->scaling) {
                        SDL_RenderSetLogicalSize(game->renderer, 1920, 1080);
                        concepts->scaling = true;
                    }
                    else {
                        SDL_RenderSetLogicalSize(game->renderer, 0, 0);
                        concepts->scaling = false;
                    }
                }
            }
            else {
                concepts->held = false;
            }
        //}

        // If the player is pressing 'ESC'.
        if (concepts->state[SDL_SCANCODE_ESCAPE]) {// Exit the game.
            concepts->quit = true; 
        }

        printf("Thread 2 processed player input.");

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
int startThreads(Timeline *t, Concepts *c, Game *g)
{
    // Mutex to handle locking, condition variables to handle notifications between threads.
    std::mutex m;
    _mutex = &m;
    std::condition_variable cv_c;
    _cv_c = &cv_c;

    // Creates a vector with both threads stored inside.
    std::vector<std::thread> both;

    // Initialize the three fields.
    time_Threads = t;
    concepts = c;
    game = g;

    // Initializes a list of two threads and runs them.
    for (int i = 0; i < 2; i++) {
        both.push_back(std::thread(runThread, i));
    }

    // Joins both threads as they finish.
    for (auto& th : both){
        th.join();
    }

    // Exit successfully.
    return 0;
}