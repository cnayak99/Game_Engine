#ifndef PHYSICS_H
#define PHYSICS_H

#include "Entity.h"

class Physics {
public:
    Physics(float gravity = 9.8f);
    void applyGravity(Entity& entity, float deltaTime);

private:
    float gravity;
};

#endif // PHYSICS_H
