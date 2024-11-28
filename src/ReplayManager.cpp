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
    recordedEntitiesM.clear(); 
    recordedEntitiesC.clear(); 
    recordedEntitiesV.clear(); 
    recordedEntitiesS.clear();
    std::cout << "Recording started.\n";
}

void ReplayManager::stopRecording() {
    isRecording = false;
    // std::cout << "Recording stopped. Total entities recorded: " << recordedEntities.size() << "\n";
}

void ReplayManager::startReplay() {
    if (!recordedEntitiesV.empty() && !recordedEntitiesC.empty() && !recordedEntitiesM.empty()) {
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

void ReplayManager::recordEntity(const Entity* entity, int64_t timestamp, char type) {
    if (isRecording) {
        SDL_Rect recordedPosition = entity->getRect();
        RecordedEntity recordedEntity = {entity, recordedPosition, timestamp};

        switch (type) {
            case 'M':
                recordedEntitiesM.push_back(recordedEntity);
                break;
            case 'V':
                recordedEntitiesV.push_back(recordedEntity);
                break;
            case 'C':
                recordedEntitiesC.push_back(recordedEntity);
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
    if (recordedEntitiesM.empty() && recordedEntitiesV.empty() && recordedEntitiesC.empty()) return;

    int64_t startTime = SDL_GetTicks64();
    size_t indexM = 0;
    size_t indexC = 0;
    size_t indexV = 0;
    size_t indexS = 0;

    while (indexM < recordedEntitiesM.size() || indexC < recordedEntitiesC.size() || indexV < recordedEntitiesV.size() || indexS < recordedEntitiesS.size()) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Set background color (blue)
        SDL_RenderClear(renderer);

        renderStaticPlatforms(renderer, concepts->tileMap);        // Process one element from recordedEntitiesC
        if (indexC < recordedEntitiesC.size()) {
            const auto& recordedEntity = recordedEntitiesC[indexC];
            if (recordedEntity.entity != nullptr) {
                SDL_Rect rect = recordedEntity.position;

                SDL_SetRenderDrawColor(renderer,
                                       recordedEntity.entity->getColor().r,
                                       recordedEntity.entity->getColor().g,
                                       recordedEntity.entity->getColor().b,
                                       255);

                SDL_RenderFillRect(renderer, &rect);
            }
            ++indexC;
        }

        // Process one element from recordedEntitiesM
        if (indexM < recordedEntitiesM.size()) {
            const auto& recordedEntity = recordedEntitiesM[indexM];
            if (recordedEntity.entity != nullptr) {
                SDL_Rect rect = recordedEntity.position;

                SDL_SetRenderDrawColor(renderer,
                                       recordedEntity.entity->getColor().r,
                                       recordedEntity.entity->getColor().g,
                                       recordedEntity.entity->getColor().b,
                                       255);

                SDL_RenderFillRect(renderer, &rect);
            }
            ++indexM;
        }

        if (indexV < recordedEntitiesV.size()) {
            const auto& recordedEntity = recordedEntitiesV[indexV];
            if (recordedEntity.entity != nullptr) {
                SDL_Rect rect = recordedEntity.position;

                SDL_SetRenderDrawColor(renderer,
                                       recordedEntity.entity->getColor().r,
                                       recordedEntity.entity->getColor().g,
                                       recordedEntity.entity->getColor().b,
                                       255);

                SDL_RenderFillRect(renderer, &rect);
            }
            ++indexV;
        }

        if (indexS < recordedEntitiesS.size()) {
            const auto& recordedEntity = recordedEntitiesS[indexS];
            if (recordedEntity.entity != nullptr) {
                SDL_Rect rect = recordedEntity.position;

                SDL_SetRenderDrawColor(renderer,
                                       recordedEntity.entity->getColor().r,
                                       recordedEntity.entity->getColor().g,
                                       recordedEntity.entity->getColor().b,
                                       255);

                SDL_RenderFillRect(renderer, &rect);
            }
            ++indexV;
        }

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

void ReplayManager::renderStaticPlatforms(SDL_Renderer* renderer, Entity* tileMap[MAP_WIDTH][MAP_HEIGHT]) {
    for (int j = 0; j < MAP_HEIGHT; j++) {
        for (int i = 0; i < MAP_WIDTH; i++) {
            if (tileMap[i][j] != nullptr) {
                tileMap[i][j]->render(renderer);
            }
        }
    }
}
// void ReplayManager::playReplay(SDL_Renderer* renderer) {
//     if (recordedEntitiesM.empty() && recordedEntitiesV.empty() && recordedEntitiesC.empty()) return;

//     int64_t startTime = SDL_GetTicks64();

//     auto renderEntities = [&](const std::vector<RecordedEntity>& entities) {
//         for (const auto& recordedEntity : entities) {
//             if (recordedEntity.entity != nullptr) {
//                 SDL_Rect rect = recordedEntity.position;

//                 SDL_SetRenderDrawColor(renderer,
//                                        recordedEntity.entity->getColor().r,
//                                        recordedEntity.entity->getColor().g,
//                                        recordedEntity.entity->getColor().b,
//                                        255);

//                 SDL_RenderFillRect(renderer, &rect);
//             }
//         }
//     };

//     while (isReplaying) {
//         SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Set background color (blue)
//         SDL_RenderClear(renderer);

//         // Render all entities in one go
//         renderEntities(recordedEntitiesC);
//         renderEntities(recordedEntitiesM);
//         renderEntities(recordedEntitiesV);

//         SDL_RenderPresent(renderer);

//         // Add a delay to simulate timing between frames
//         int64_t elapsedTime = SDL_GetTicks64() - startTime;
//         if (elapsedTime < 16) {  // Assuming a frame duration of ~16ms for 60 FPS
//             SDL_Delay(16 - elapsedTime);
//         }

//         startTime = SDL_GetTicks64();
//     }
// }