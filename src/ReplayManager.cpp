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
    recordedEntitiesP.clear(); 
    recordedEntitiesB.clear(); 
    recordedEntitiesV.clear(); 
    // recordedEntitiesS.clear();
    std::cout << "Recording started.\n";
}

void ReplayManager::stopRecording() {
    isRecording = false;
    // std::cout << "Recording stopped. Total entities recorded: " << recordedEntities.size() << "\n";
}

void ReplayManager::startReplay() {
    if (!recordedEntitiesP.empty() && !recordedEntitiesB.empty()) {
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

void ReplayManager::recordEntity(const Entity* entity, int64_t timestamp, const std::vector<bool>& bricks, int& livesCount, char type) {
    if (isRecording) {

        SDL_Rect recordedPosition = entity->getRect();
        RecordedEntity recordedEntity = {entity, recordedPosition, timestamp, bricks,livesCount};

        switch (type) {
            case 'P':
                recordedEntitiesP.push_back(recordedEntity);
                break;
            case 'B':
                recordedEntitiesB.push_back(recordedEntity);
                break;
            case 'V':
                recordedEntitiesV.push_back(recordedEntity);
                break;
            case 'S':
                recordedEntitiesS.push_back(recordedEntity);
                break;
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
    if (recordedEntitiesP.empty()&& recordedEntitiesB.empty()&& recordedEntitiesV.empty()) return;

    int64_t startTime = SDL_GetTicks64();
    size_t indexP = 0;
    size_t indexB = 0;
    size_t indexV = 0;
    // size_t indexS = 0;

    while (indexP < recordedEntitiesP.size()|| indexB < recordedEntitiesB.size() || indexV < recordedEntitiesV.size()) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color (blue)
        SDL_RenderClear(renderer);

        if (indexB < recordedEntitiesB.size()) {
            const auto& recordedEntity = recordedEntitiesB[indexB];
            if (recordedEntity.entity != nullptr) {
                SDL_Rect rect = recordedEntity.position;
                SDL_SetRenderDrawColor(renderer,255,255,255,255);
                SDL_RenderFillRect(renderer, &rect);
            }
            ++indexB;
        }

        // Process one element from recordedEntitiesM
        if (indexP < recordedEntitiesP.size()) {
            const auto& recordedEntity = recordedEntitiesP[indexP];
            if (recordedEntity.entity != nullptr) {
                SDL_Rect rect = recordedEntity.position;

                SDL_SetRenderDrawColor(renderer,255,255,255,255);

                SDL_RenderFillRect(renderer, &rect);
                renderText(std::to_string(recordedEntity.livesCount), WIDTH/2+FONT_SIZE/2, FONT_SIZE*1.5, renderer, concepts->lives->getRect());

            }
            
            ++indexP;
        }

        if (indexV < recordedEntitiesV.size()) {
            const auto& recordedEntity = recordedEntitiesV[indexV];
            if (recordedEntity.entity != nullptr) {
                SDL_Rect rect = recordedEntity.position;

                // SDL_SetRenderDrawColor(renderer,
                //                        recordedEntity.entity->getColor().r,
                //                        recordedEntity.entity->getColor().g,
                //                        recordedEntity.entity->getColor().b,
                //                        255);

                // SDL_RenderFillRect(renderer, &rect);
                for(int i=0; i<COL*ROW; i++) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                    if(i%2==0)SDL_SetRenderDrawColor(renderer, 0,255, 0, 255);
                    if(recordedEntity.bricks[i]) {
                        setBricks(i,rect);
                        SDL_RenderFillRect(renderer, &rect);
                    }
                }
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
void ReplayManager::setBricks(int i, SDL_Rect& brick) {
    brick.x = (((i % COL) + 1) * SPACING) + ((i % COL) * brick.w) - (SPACING / 2);
    brick.y = brick.h * 3 + (((i % ROW) + 1) * SPACING) + ((i % ROW) * brick.h) - (SPACING / 2);
}

void ReplayManager::renderText(std::string text, int x, int y, SDL_Renderer* renderer, SDL_Rect& lives) {
    SDL_Surface* surface;
    SDL_Texture* texture;
    TTF_Font* font = TTF_OpenFont((char*)"/usr/share/fonts/truetype/msttcorefonts/arial.ttf", 25);
    std::string fullText = "Lives Remaining: " + text;
    const char* t = fullText.c_str(); 
    surface = TTF_RenderText_Solid(font, t, {255,255,255});
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    lives.w = surface->w;
    lives.h = surface->h;
    lives.x = x - lives.w + 100;
    lives.y = y - lives.h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &lives);
    SDL_DestroyTexture(texture);
}
