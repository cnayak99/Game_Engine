#ifndef REPLAYMANAGER_H
#define REPLAYMANAGER_H

#include <vector>
#include "Entity.h"
#include "structs.h"
#include "defs.h"
#include <SDL2/SDL_ttf.h>

struct RecordedEntity {
    const Entity* entity;  // Pointer to the entity
    SDL_Rect position;     // Recorded position of the entity
    int64_t timestamp;     // Timestamp of when the entity was recorded
    std::vector<int> tailX;
    std::vector<int> tailY;
    int tailLength;
    int X;
    int Y;
};

class ReplayManager {
public:
    bool isRecording = false;
    bool isReplaying = false;  // Flag for replay mode
    // std::vector<RecordedEntity> recordedEntities;  // Store entities with their positions
    std::vector<RecordedEntity> recordedEntitiesF;  // Store entities of type M
    std::vector<RecordedEntity> recordedEntitiesV;  // Store entities of type V
    std::vector<RecordedEntity> recordedEntitiesC;  // Store entities of type C
    std::vector<RecordedEntity> recordedEntitiesS;  // Store entities of type C
    Concepts* concepts;

    void startRecording();
    void stopRecording();
    void startReplay();
    void stopReplay();
    // void recordEntity(const Entity* entity, int64_t timestamp);
    void recordEntity(const Entity* entity, int64_t timestamp,std::vector<int>tailX, std::vector<int>tailY, char type);
    void playReplay(SDL_Renderer* renderer);
    void renderEntity(SDL_Renderer* renderer, const RecordedEntity& recordedEntity);
    void renderStaticPlatforms(SDL_Renderer* renderer, Entity* tileMap[MAP_WIDTH][MAP_HEIGHT]);
    ReplayManager(Concepts* c)
        : concepts(c){}
    void renderFood(SDL_Renderer* renderer, const SDL_Rect& food);
    void renderPlayer(SDL_Renderer* renderer, SDL_Rect player, int x, int y, int scale, std::vector<int> tailX, std::vector<int> tailY, int tailLength);
    void renderScore(SDL_Renderer* renderer, int tailLength, int scale, int wScale, Concepts concepts);
	
};

#endif // REPLAYMANAGER_H