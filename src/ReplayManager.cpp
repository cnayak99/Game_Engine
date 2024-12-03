#include "ReplayManager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
using namespace std; 
#include "defs.h"



void ReplayManager::startRecording() {
    isRecording = true;
    // recordedEntities.clear();  // Clear previous recordings
    recordedEntitiesF.clear(); //Food
    // recordedEntitiesC.clear(); //
    recordedEntitiesV.clear();//Player 
    // recordedEntitiesS.clear();
    std::cout << "Recording started.\n";
}

void ReplayManager::stopRecording() {
    isRecording = false;
    // std::cout << "Recording stopped. Total entities recorded: " << recordedEntities.size() << "\n";
}

void ReplayManager::startReplay() {
    if (!recordedEntitiesF.empty() && !recordedEntitiesV.empty()) {
        isReplaying = true;
        std::cout << "Replay started.\n";
    } else {
        std::cout << "No entities to replay.\n";
    }
}

void ReplayManager::stopReplay() {
    isReplaying = false;
    std::cout << "Replay stopped.\n";
}

void ReplayManager::recordEntity(const Entity* entity, int64_t timestamp, std::vector<int>tailX, std::vector<int>tailY, char type) {
    if (isRecording) {
        SDL_Rect recordedPosition = entity->getRect();
        RecordedEntity recordedEntity;

        switch (type) {
            case 'F':
                recordedEntity = {entity, recordedPosition, timestamp};
                recordedEntitiesF.push_back(recordedEntity);
                break;
            case 'V':
                recordedEntity = {entity, recordedPosition, timestamp, tailX, tailY, concepts->tailLength, concepts->x, concepts->y};
                recordedEntitiesV.push_back(recordedEntity);
                break;
            // case 'C':
            //     recordedEntitiesC.push_back(recordedEntity);
            //     break;
            // case 'S':
            //     recordedEntitiesS.push_back(recordedEntity);
            //     break;
        }

        // Debug log
        std::cout << "[ReplayManager] Recorded Entity: Rect(" 
                  << recordedPosition.x << ", " << recordedPosition.y 
                  << ", " << recordedPosition.w << ", " << recordedPosition.h 
                  << "), Color(" << (int)entity->getColor().r << ", " 
                  << (int)entity->getColor().g << ", " 
                  << (int)entity->getColor().b 
                  << "), Timestamp: " << timestamp 
                  << "\n";
    }
}

void ReplayManager::playReplay(SDL_Renderer* renderer) {
    if (recordedEntitiesF.empty()  && recordedEntitiesV.empty()) return;

    int64_t startTime = SDL_GetTicks64();
    size_t indexF = 0;
    // size_t indexC = 0;
    size_t indexV = 0;
    // size_t indexS = 0;

    while (indexF < recordedEntitiesF.size() || indexV < recordedEntitiesV.size()) {
        SDL_SetRenderDrawColor(renderer, 105, 105, 105, 255);
        SDL_RenderClear(renderer);
        // Process one element from recordedEntitiesF
        if (indexF < recordedEntitiesF.size()) {
            const auto& recordedEntity = recordedEntitiesF[indexF];
            if (recordedEntity.entity != nullptr) {
                SDL_Rect rect = recordedEntity.position;
                renderFood(renderer, rect);
                // cout<<"Food Entity: X- "<<rect.x<< " Y: "<<rect.y<<endl;

            }
            ++indexF;
        }

        if (indexV < recordedEntitiesV.size()) {
            const auto& recordedEntity = recordedEntitiesV[indexV];
            if (recordedEntity.entity != nullptr) {
                SDL_Rect rect = recordedEntity.position;

                renderPlayer(renderer, rect, recordedEntity.X, recordedEntity.Y, SCALE, recordedEntity.tailX, recordedEntity.tailY, recordedEntity.tailLength);
                renderScore(renderer, recordedEntity.tailLength, SCALE, WSCALE, *concepts);
            }
            ++indexV;
        }

        

        // if (indexS < recordedEntitiesS.size()) {
        //     const auto& recordedEntity = recordedEntitiesS[indexS];
        //     if (recordedEntity.entity != nullptr) {
        //         SDL_Rect rect = recordedEntity.position;

        //         SDL_SetRenderDrawColor(renderer,
        //                                recordedEntity.entity->getColor().r,
        //                                recordedEntity.entity->getColor().g,
        //                                recordedEntity.entity->getColor().b,
        //                                255);

        //         SDL_RenderFillRect(renderer, &rect);
        //     }
        //     ++indexV;
        // }

        // Present the rendered frame
        SDL_RenderPresent(renderer);

        // Add a delay to simulate timing between frames
        int64_t elapsedTime = SDL_GetTicks64() - startTime;
        if (elapsedTime < 16) {  // Assuming a frame duration of ~16ms for 60 FPS
            SDL_Delay(16 - elapsedTime);
        }

        startTime = SDL_GetTicks64();
    }
}
void ReplayManager::renderFood(SDL_Renderer* renderer, const SDL_Rect& food) {
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_RenderFillRect(renderer, &food);
}

void ReplayManager::renderPlayer(SDL_Renderer* renderer, SDL_Rect player, int x, int y, int scale, vector<int> tailX, vector<int> tailY, int tailLength) {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	player.w = scale;
	player.h = scale;

	// Gets x and y of all tail blocks and renders them
	for (int i = 0; i < tailLength; i++) {
		player.x = tailX[i];
		player.y = tailY[i];
		SDL_RenderFillRect(renderer, &player);
	}

	player.x = x;
	player.y = y;

	SDL_RenderFillRect(renderer, &player);
}
void ReplayManager::renderScore(SDL_Renderer* renderer, int tailLength, int scale, int wScale, Concepts concepts) {
	SDL_Color Black = { 0, 0, 0 };

	// Get the font used for displaying text
	TTF_Font* font = TTF_OpenFont((char*)"/usr/share/fonts/truetype/msttcorefonts/arial.ttf", 10);
	if (font == NULL) {
		cout << "Font loading error" << endl;
		return;
	}

	SDL_Surface* score = TTF_RenderText_Solid(font, (string("Score: ") + to_string(tailLength * 10)).c_str(), Black);
	SDL_Texture* scoreMessage = SDL_CreateTextureFromSurface(renderer, score);
	Entity scoreRect(((scale*wScale) / 2) - (100 / 2), 0, 100, 30, {255,255,255,255}, false, 0);
	concepts.scoreRect=&scoreRect;

	SDL_RenderCopy(renderer, scoreMessage, NULL, &concepts.scoreRect->getRect());
	TTF_CloseFont(font);
}