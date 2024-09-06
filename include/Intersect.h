#include "Rectangle.h"

/**
 * Checks two rectangles to see if they intersect for basic collision.
 * 
 * References resources and tutorials provided by Professor Card through
 * the "CSC 481-581 HW 1-4.pdf" located beneath the "Homework 1" title in
 * the "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page. These resources can be found in the
 * https://wiki.libsdl.org/SDL2/FrontPage website. Heavily influenced by
 * the "SDL_HasIntersectionF()" function in the "SDL_rect.h" header, which
 * can be located here: https://wiki.libsdl.org/SDL2/SDL_Rect.
 * 
 * \param player the first Rectangle to check
 * \param terrain the second Rectangle to check
 * \returns bool that is true if the Rectangles intersect, and false
 * if they do not
 * 
 * Most of the work has been completed by Lillie Sharpe, but Robbie Martin
 * helped with the logic.
 * 
 * @author Lillie Sharpe (lssharpe)
 */
extern bool hasIntersection(const Rectangle * first, const Rectangle * second);

/**
 * Verifies where the collision occurred.
 * 
 * References resources and tutorials provided by Professor Card through
 * the "CSC 481-581 HW 1-4.pdf" located beneath the "Homework 1" title in
 * the "CSC 481/581 (001) Fall 2024 Game Engine Foundations" course
 * Moodle page. These resources can be found in the
 * https://wiki.libsdl.org/SDL2/FrontPage website. Heavily influenced by
 * the "SDL_IntersectFRect()" function in the "SDL_rect.h" header., which
 * can be located here: https://wiki.libsdl.org/SDL2/SDL_Rect.
 * 
 * \param first the first Rectangle to check
 * \param second the second Rectangle to check
 * \param result the result of the check (removed for now due to redundancy)
 * \returns int that is positive if the Rectangles intersect, and negative
 * if they do not
 * 
 * Heavily inspired by the 'hasIntersection()' function in this same
 * file.
 * 
 * @author Lillie Sharpe (lssharpe)
 */
extern int intersect(const Rectangle * first, const Rectangle * second);