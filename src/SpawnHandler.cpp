#include "SpawnHandler.h"  // Include your header first
#include "Event.h"
#include "structs.h"  // Include structs.h for full definition of Concepts and Game
#include "EventManager.h"
#include "defs.h"

/**
 * Handles the event of despawning and respawning the player.
 * 
 * @param e the event to be processed
 */
void SpawnHandler::onEvent(const Event& e) {
    // If the event is a spawn event; else, do nothing.
    if (e.type == "spawn") {
        // Receives the spawn code as a parameter.
        int spawnCode = e.parameters.at("spawnCode").asInt;

        // Gives a system statement confirming that the event was received.
        // std::cout << "Spawn Event Received: Despawn and Respawn."<< std::endl;

        // If the spawn code is 5, the player must despawn and respawn due to a death zone.
       if (spawnCode == 1) { // Spawn code 1, respawn due to bound one map 1 collision.
            youWin(game->renderer, *game->event, SCALE, WSCALE, concepts->tailLength,*concepts);
        }

        // Gives a system statement confirming that the event was completed.
        // std::cout << "Spawn Event Completed: Despawn and Respawn."<< std::endl;
    }
}

void SpawnHandler::youWin(SDL_Renderer* renderer, SDL_Event event, int scale, int wScale, int tailLength, Concepts &concepts) {
	SDL_Color Red = { 255, 0, 0 };
	SDL_Color White = { 255, 255, 255 };
	SDL_Color Black = { 0, 0, 0 };
	SDL_Color Yellow = { 255, 255, 0 };

	// Get the font used for displaying text
	TTF_Font* font = TTF_OpenFont((char*)"/usr/share/fonts/truetype/msttcorefonts/arial.ttf", 10);
	if (font == NULL) {
		std::cout << "Font loading error" << std::endl;
		return;
	}

	SDL_Surface* gameover = TTF_RenderText_Solid(font, "You won!", Yellow);
	SDL_Surface* retry = TTF_RenderText_Solid(font, "Press Enter to play again", White);
	SDL_Surface* score = TTF_RenderText_Solid(font, (std::string("Score: ") + std::to_string(tailLength * 10)).c_str(), Black);
	SDL_Texture* gameoverMessage = SDL_CreateTextureFromSurface(renderer, gameover);
	SDL_Texture* retryMessage = SDL_CreateTextureFromSurface(renderer, retry);
	SDL_Texture* scoreMessage = SDL_CreateTextureFromSurface(renderer, score);

	Entity gameoverRect(((scale*wScale) / 2)-(200/2), ((scale*wScale) / 2)-(100/2)-50, 200, 100,{255,255,255,255}, false, 0);
	concepts.gameoverRect=&gameoverRect;
	Entity retryRect(((scale*wScale) / 2) - ((300 / 2)), (((scale*wScale) / 2) - ((50 / 2))+150), 300, 50,
	{255,255,255,255}, false, 0);
	concepts.retryRect=&retryRect;
	Entity scoreRect(((scale*wScale) / 2) - (100 / 2), 0, 100, 30, {255,255,255,255}, false, 0);
	concepts.scoreRect=&scoreRect;

	SDL_RenderCopy(renderer, gameoverMessage, NULL, &concepts.gameoverRect->getRect());
	SDL_RenderCopy(renderer, retryMessage, NULL, &concepts.retryRect->getRect());
	SDL_RenderCopy(renderer, scoreMessage, NULL, &concepts.scoreRect->getRect());

	TTF_CloseFont(font);

	// Show victory screen while space has not been pressed
	while (true) {
		SDL_RenderPresent(renderer);

		if (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				exit(0);
			}

			if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
				return;
			}

		}

	}

}