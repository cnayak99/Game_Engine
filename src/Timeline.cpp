#include <mutex>
#include <cmath>
#include <chrono>
#include <condition_variable>

#include "Timeline.h"

/** Allows pause and unpause to communicate. */
std::condition_variable cv_m;

/**
 * Creates a Timeline struct, which keeps track of and monitors a
 * timeline within the game.
 * 
 * This struct is heavily inspired from the example Timeline class
 * displayed by Professor Card on pages 10 and 11 in the
 * "S24 05 Timelines.pptx" PowerPoint that is available on the
 * "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page through the "Lecture Notes" link beneath the
 * "General Information and Discussions" subtitle.
 * 
 * @author Lillie Sharpe (lssharpe)
 * @author Robbie Martin (rdmarti5)
 * 
 */

Timeline::Timeline(Timeline *anchor, int64_t tic): anchor(anchor), tic(tic){}

/**
 * Returns Timeline.
 * 
 * Utilizes mutex.
 * 
 * We referenced the following website to learn how to utilize
 * mutex as a lock in C++:
 * https://en.cppreference.com/w/cpp/thread/mutex
 */
int64_t Timeline::getTimeline() {
    using namespace std::chrono;
    // Locks mutex and automatically unlocks itself.
    std::unique_lock<std::mutex> cv_lock(m);
    // Keeps track of the count.
    int64_t countFull = 0;
    // Returns the elapsed time of this Timeline object.
    std::chrono::time_point<std::chrono::system_clock> curr = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    if (anchor == nullptr) {
        std::chrono::duration elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curr - std::chrono::time_point_cast<std::chrono::milliseconds>(start));
        // Checks the value of tic to update it accordingly.
        if (tic != 3) {
            // If tic is not 3, treat it like normal.
            count = elapsed.count() / tic;
        } else {
            // If tic is 3, treat it like .5.
            count = elapsed.count() * 2;
        }
        countFull = count;
        // Notifies unpause that the game is paused.
        cv_m.notify_all();
        return countFull;
    }
    if (tic != 3) {
        // If tic is not 3, treat it like normal.
        count = anchor->getTimeline() / tic;
    } else {
        // If tic is 3, treat it like .5.
        count = anchor->getTimeline() * 2;
    }
    countFull = count - pauseElapsed;
    // Notifies unpause that the game is paused.
    cv_m.notify_all();
    return countFull;
}

/**
 * Pauses the game.
 * 
 * Utilizes mutex.
 * 
 * We referenced the following website to learn how to utilize
 * mutex as a lock in C++:
 * https://en.cppreference.com/w/cpp/thread/mutex
 */
void Timeline::pause() {
    // Pauses the game.
    pauseLast = getTimeline();
    isPaused = true;
}

/**
 * Unpauses the game.
 * 
 * Utilizes mutex.
 * 
 * We referenced the following website to learn how to utilize
 * mutex as a lock in C++:
 * https://en.cppreference.com/w/cpp/thread/mutex
 */
void Timeline::unpause() {
    // Unpauses the game.
    pauseElapsed = getTimeline() - pauseLast;
    isPaused = false;
}

/**
 * Sets the rate at which a timeline moves forward for each unit of anchor time.
 */
void Timeline::setTicks(int64_t tic) {
    // Locks mutex and automatically unlocks itself.
    std::unique_lock<std::mutex> cv_lock(m);
    this->tic = tic;
    // Notifies unpause that the game is paused.
    cv_m.notify_all();
}

/**
 * Returns the rate at which a timeline moves forward for each unit of anchor time.
 */

int64_t Timeline::getTicks() {
    // Locks mutex and automatically unlocks itself.
    std::unique_lock<std::mutex> cv_lock(m);
    // Stores the current tic.
    int64_t currentTic = this->tic;
    // Notifies unpause that the game is paused.
    cv_m.notify_all();
    // Returns the current tic.
    return currentTic;
}
