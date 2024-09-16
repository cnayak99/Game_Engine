#ifndef TIME_H
#define TIME_H

#include <mutex>

/**
 * Creates a Time struct, which keeps track of and monitors a
 * timeline within the game.
 * 
 * This class is heavily inspired from the example Timeline class
 * displayed by Professor Card on pages 10 and 11 in the
 * "S24 05 Timelines.pptx" PowerPoint that is available on the
 * "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page through the "Lecture Notes" link beneath the
 * "General Information and Discussions" subtitle.
 * 
 * @author Lillie Sharpe (lssharpe)
 */
class Time {
    private:
        std::mutex m; // For multithreading threads of different sizes.
        int64_t start; // The start time of the anchor.
        int64_t pauseElapsed; // Elapsed pause time.
        int64_t pauseLast; // Last pause time.
        int64_t tic; // Units of anchor time for each step.
        bool isPaused; // Keeps track of pause status.
        Time *anchor; // For the base time.
    public:
        Time(Time *anchor, int64_t tic); // Constructs Time struct.
        // All three following functions must use a mutex.
        int64_t getTime(); // Returns Time.
        void pause(); // Pauses the game.
        void unpause(); // Unpauses the game.
};

#endif