#ifndef THREADS_H
#define THREADS_H

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
void runThread(int id);

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
void runPhysics();

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
void runInput();

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
 * https://en.cppreference.com/w/cpp/container/vector. Because
 * the threads no longer were accessible after they were executed,
 * we also learned about the "auto" keyword for the sake of joining
 * them back together, which we learned about from
 * https://en.cppreference.com/w/cpp/language/auto.
 */
int startThreads(Timeline *t, Concepts *c, Game *g);

#endif