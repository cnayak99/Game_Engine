#include "Rectangle.h"
#include "Intersect.h"

/**
 * Heavily influenced by the "SDL_HasIntersectionF()" function in the
 * "SDL_rect.h" header.
 * 
 * Checks two rectangles to see if they intersect for basic collision.
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
extern bool hasIntersection(const Rectangle * player, const Rectangle * terrain) {
    // Checks all four bounds of the player rectangle in comparison to the bounds
    // of a terrain rectangle. Currently, they check the horizontal middle of a rectangle.
    if ((player->x >= terrain->x) && (player->x < (terrain->x + terrain->width - 1))) {
        if ((player->y >= terrain->y) && (player->y < (terrain->y + terrain->height - 1))) {
            return true;
        } else if (((player->y + player->height - 1) >= terrain->y) && ((player->y + player->height - 1) < (terrain->y + terrain->height - 1))) {
            return true;
        }
    } else if (((player->x + player->width - 1) >= terrain->x) && ((player->x + player->width - 1) < (terrain->x + terrain->width - 1))) {
        if ((player->y >= terrain->y) && (player->y < (terrain->y + terrain->height - 1))) {
            return true;
        } else if (((player->y + player->height - 1) >= terrain->y) && ((player->y + player->height - 1) < (terrain->y + terrain->height - 1))) {
            return true;
        }
    }
    // Returns false if no boundaries intersected.
    return false;
}

/**
 * Heavily influenced by the "SDL_IntersectFRect()" function in the
 * "SDL_rect.h" header.
 * 
 * Verifies where the collision occurred.
 * 
 * \param first the first Rectangle to check
 * \param second the second Rectangle to check
 * \param result the result of the check (removed for now due to redundancy)
 * \returns bool that is true if the Rectangles intersect, and false
 * if they do not
 * 
 * Heavily inspired by the 'hasIntersection()' function in this same
 * file.
 * 
 * @author Lillie Sharpe (lssharpe)
 */
extern int intersect(const Rectangle * player, const Rectangle * terrain) {
    // Checks all four bounds of the player rectangle in comparison to the bounds
    // of a terrain rectangle. Currently, they check the horizontal middle of a rectangle.
    // Functionality for the 'result' variable, the third variable which was removed due to
    // redundancy, will be added at a later time.
    if ((player->x >= terrain->x) && (player->x < (terrain->x + terrain->width - 1))) {
        if ((player->y >= terrain->y) && (player->y < (terrain->y + terrain->height - 1))) {
            return 1;
        } else if (((player->y + player->height - 1) >= terrain->y) && ((player->y + player->height - 1) < (terrain->y + terrain->height - 1))) {
            return 2;
        }
    } else if (((player->x + player->width - 1) >= terrain->x) && ((player->x + player->width - 1) < (terrain->x + terrain->width - 1))) {
        if ((player->y >= terrain->y) && (player->y < (terrain->y + terrain->height - 1))) {
            return 3;
        } else if (((player->y + player->height - 1) >= terrain->y) && ((player->y + player->height - 1) < (terrain->y + terrain->height - 1))) {
            return 4;
        }
    }
    // Returns -1 if no boundaries intersected.
    return -1;
}