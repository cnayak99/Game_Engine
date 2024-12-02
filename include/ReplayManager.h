#ifndef REPLAYMANAGER_H
#define REPLAYMANAGER_H

#include <vector>
#include "Entity.h"
#include "structs.h"
#include "defs.h"

struct RecordedEntity {
    const Entity* entity;  // Pointer to the entity
    SDL_Rect position;     // Recorded position of the entity
    int64_t timestamp;     // Timestamp of when the entity was recorded
    std::vector<bool> bricks; 
    int livesCount;
};

class ReplayManager {
public:
    bool isRecording = false;
    bool isReplaying = false;  // Flag for replay mode
    // std::vector<RecordedEntity> recordedEntities;  // Store entities with their positions
    std::vector<RecordedEntity> recordedEntitiesP;  // Store entities of type M
    std::vector<RecordedEntity> recordedEntitiesV;  // Store entities of type V
    std::vector<RecordedEntity> recordedEntitiesB;  // Store entities of type C
    std::vector<RecordedEntity> recordedEntitiesS;  // Store entities of type C
    Concepts* concepts;

    void startRecording();
    void stopRecording();
    void startReplay();
    void stopReplay();
    // void recordEntity(const Entity* entity, int64_t timestamp);
    void recordEntity(const Entity* entity, int64_t timestamp, const std::vector<bool>& bricks,int &livesCount, char type);
    void playReplay(SDL_Renderer* renderer);
    void renderText(std::string text, int x, int y, SDL_Renderer* renderer, SDL_Rect& lives);
    void renderEntity(SDL_Renderer* renderer, const RecordedEntity& recordedEntity);
    void renderStaticPlatforms(SDL_Renderer* renderer, Entity* tileMap[MAP_WIDTH][MAP_HEIGHT]);
    void setBricks(int i, SDL_Rect& brick);
    ReplayManager(Concepts* c)
        : concepts(c){}
};

#endif // REPLAYMANAGER_H