#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include "Timeline.h"
#include "Entity.h"
#include "Rectangle.h"

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
    std::condition_variable *_cv_s; // For thread communication regarding the static entity.
    std::condition_variable *_cv_m; // For thread communication regarding the moving entity.
    Timeline *time_Threads;
    Entity *staticE;
    Entity *controllableE;
    Entity *movingE;

    public:
        Threads(int i, Threads *other, std::mutex *_mutex, std::condition_variable *_cv_s, std::condition_variable *_cv_m, Timeline *time_Threads, Entity *staticE, Entity *controllableE, Entity *movingE)
        {
            this->i = i; // set the id of this thread
            if(i==0) { busy = true; }
            else { this->other = other; }
            this->_mutex = _mutex;
            this->_cv_s = _cv_s;
            this->_cv_m = _cv_m;
            this->time_Threads = time_Threads;
            this->staticE = staticE;
            this->controllableE = controllableE;
            this->movingE = movingE;
        }

        bool isBusy()
        {
            std::lock_guard<std::mutex> lock(*_mutex);  // this locks the mutuex until the variable goes out of scope (i.e., when the function returns in this case)
            return busy;
        }

        void run()
        {
            // Keeps track of one thread, which I will refer to as Thread 1.
            if(i==0)
            {
                // Thread 1 tries to keep track of non-player rectangles for this tic.
                try
                {
                    // This block controls the Static Entity.
                    { // Anonymous block to manage mutex lock scope.
                        std::unique_lock<std::mutex> cv_lock(*this->_mutex);
                        // Insert implementation for static entity here.
                        _cv_s->notify_all();
                    }

                    // This block controls the Moving Entity.
                    { // Anonymous block to manage mutex lock scope.
                        std::unique_lock<std::mutex> cv_lock(*this->_mutex);
                        // Insert implementation for moving entity here.
                        _cv_m->notify_all();
                    }

                    // This block manages the thread itself.
                    {// Anonymous block to manage mutex lock scope.
                        std::unique_lock<std::mutex> cv_lock(*this->_mutex);
                        busy = !busy;
                        _cv_s->notify_all();
                        _cv_m->notify_all();
                    }
                }
                catch (...)
                {
                    std::cerr << "Thread 1 caught exception." << std::endl;
                }
            }
            else // id == 1
            {
                while(other->isBusy())
                {
                    // Thread 2 tries to keep track of the player rectangle for this tic.
                    try 
                    {
                        // This block manages user input.
                        // First, manage user input.
                        { // Anonymous block to keep track of user input.
                            // Keep track of user input here.
                        }

                        // This block manages player entity collision with static entity.
                        { // Anonymous block to manage mutex lock scope.
                            std::unique_lock<std::mutex> lock(*_mutex);
                            _cv_s->wait(lock);
                            // Implement collision between player entity and static entity here.
                        }

                        // This block manages player entity collision with moving entity.
                        { // Anonymous block to manage mutex lock scope.
                            std::unique_lock<std::mutex> lock(*_mutex);
                            _cv_m->wait(lock);
                            // Implement collision between player entity and moving entity here.
                        }
                    }
                    catch (...)
                    {
                        std::cerr << "Thread 2 caught exception." << std::endl;
                    }
                }
            }
        }
};

/**
 * Wrapper function because threads can't take pointers to member functions.
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
 */
void run_wrapper(Threads *th)
{
    th->run();
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
 */
int main(Timeline *time_Threads, Entity *staticE, Entity *controllableE, Entity *movingE)
{    // Mutex to handle locking, condition variables to handle notifications between threads
    std::mutex m;
    std::condition_variable cv_s;
    std::condition_variable cv_m;

    // Create thread objects
    Threads t1(0, NULL, &m, &cv_s, &cv_m, time_Threads, staticE, controllableE, movingE);
    Threads t2(1, &t1, &m, &cv_s, &cv_m, time_Threads, staticE, controllableE, movingE);

    // Start threads
    std::thread first(run_wrapper, &t1);
    std::thread second(run_wrapper, &t2);

    // Make sure both threads are complete before stopping main thread
    first.join();
    second.join();
}