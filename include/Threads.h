#ifndef THREADS_H
#define THREADS_H

#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <iostream>

/**
 * Initializes the threads for our program.
 * 
 * This class is heavily inspired from the ThreadExample class
 * provided by Professor Card on the "CSC 481/581 (001) Fall
 * 2024 Game Engine Foundations" course Moodle page underneath
 * the "Homework 2" subtitle. It is accessible by clicking on the
 * "ThreadExample.cpp" link. It was suggested to "build upon
 * the ThreadExample provided on Moodle" for this project according
 * to Section 4 of the Homework 2 Assignment PDF
 * "CSC 481-581 HW 2-2.pdf", which is accessible also underneath
 * the "Homework 2" subtitle on the "CSC 481/581 (001) Fall 2024
 * Game Engine Foundations" course Moodle page.
 */
class Threads
{
    bool busy; // a member variable used to indicate thread "status"
    int i; // an identifier
    Threads *other; // a reference to the "other" thread in this example
    std::mutex *_mutex; // the object for mutual exclusion of execution
    std::condition_variable *_cv_s; // For messaging between threads regarding the static entity.
    std::condition_variable *_cv_m; // For messaging between threads regarding the moving entity.

    public:
        Threads(int i, Threads *other, std::mutex *_mutex, std::condition_variable *_cv_s, std::condition_variable *_cv_m); // Compiles the Threads object.
        bool isBusy(); // Alerts a thread if the other is busy.
        void run(); // Runs both threads.
        int main(); // Begins running both threads.
};

#endif