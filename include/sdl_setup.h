// sdl_setup.h
#ifndef SDL_SETUP_H
#define SDL_SETUP_H

#include <SDL2/SDL.h>

/**
 * Initializes the SDL environment.
 * 
 * References resources and tutorials provided by Professor Card through
 * the "CSC 481-581 HW 1-4.pdf" located beneath the "Homework 1" title in
 * the "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page. These resources can be found in the
 * https://wiki.libsdl.org/SDL2/FrontPage website. Particularly based on
 * tutorials provided and referenced during Step 1 in the referenced
 * pdf file.
 * 
 * \param window the SDL window
 * \param renderer the SDL renderer
 * \returns true if successful, false if unsuccessful
 */
bool initializeSDL(SDL_Window** window, SDL_Renderer** renderer);

#endif // SDL_SETUP_H
