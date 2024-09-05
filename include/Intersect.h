#include "Rectangle.h"

/**
 * Heavily influenced by the "SDL_HasIntersectionF()" function in the
 * "SDL_rect.h" header.
 * 
 * Checks two rectangles to see if they intersect for basic collision.
 * 
 * \param first the first Rectangle to check
 * \param second the second Rectangle to check
 * \returns bool that is true if the Rectangles intersect, and false
 * if they do not
 * 
 * @author Lillie Sharpe (lssharpe)
 */
extern bool hasIntersection(const Rectangle * first, const Rectangle * second);

/**
 * Heavily influenced by the "SDL_IntersectFRect()" function in the
 * "SDL_rect.h" header.
 * 
 * Checks two rectangles to see if they intersect for basic collision.
 * 
 * \param first the first Rectangle to check
 * \param second the second Rectangle to check
 * \param result the result of the check (removed for now due to redundancy)
 * \returns bool that is true if the Rectangles intersect, and false
 * if they do not
 * 
 * @author Lillie Sharpe (lssharpe)
 */
extern int intersect(const Rectangle * first, const Rectangle * second);