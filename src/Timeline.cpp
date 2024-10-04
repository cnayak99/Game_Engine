#include "Timeline.h"
#include <mutex>
#include <cmath>
#include <chrono>

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
    std::lock_guard<std::mutex> guard(m);
    // Returns the elapsed time of this Timeline object.
    std::chrono::time_point<std::chrono::system_clock> curr = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    if (anchor == nullptr) {
        std::chrono::duration elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curr - std::chrono::time_point_cast<std::chrono::milliseconds>(start));
        
        if (tic != 3) {
            // If tic is not 3, treat it like normal.
            count = elapsed.count() / tic;
        } else {
            // If tic is 3, treat it like .5.
            count = elapsed.count() * 2;
        }
        return count;
    }
    if (tic != 3) {
        // If tic is not 3, treat it like normal.
        count = anchor->getTimeline() / tic;
    } else {
        // If tic is 3, treat it like .5.
        count = anchor->getTimeline() * 2;
    }
    return count - pauseElapsed;
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
    // Locks mutex and automatically unlocks itself.
    std::lock_guard<std::mutex> guard(m);
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
    // Locks mutex and automatically unlocks itself.
    std::lock_guard<std::mutex> guard(m);
    // Unpauses the game.
    pauseElapsed = anchor->getTimeline() - pauseLast;
    isPaused = false;
}

/**
 * Sets the rate at which a timeline moves forward for each unit of anchor time.
 */

void Timeline::setTicks(int64_t tic) {
    this->tic = tic;
}

/**
 * Returns the rate at which a timeline moves forward for each unit of anchor time.
 */

int64_t Timeline::getTicks() {
    return tic;
}
