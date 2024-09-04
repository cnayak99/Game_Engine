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
 * @author Lillie Sharpe (lssharpe)
 * @author Robbie Martin (rdmarti5)
 */
extern bool hasIntersection(const Rectangle * player, const Rectangle * terrain) {
    // Checks the left bounds of the player rectangle
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
 * \param result the result of the check
 * \returns bool that is true if the Rectangles intersect, and false
 * if they do not
 * 
 * @author Lillie Sharpe (lssharpe)
 */
extern int intersect(const Rectangle * player, const Rectangle * terrain, Rectangle * result) {
    // Add result later.
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
    return -1;
}