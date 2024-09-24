#include "Physics.h"
#include "defs.h"

void applyGravity(Entity entity, float deltaTime) {
    if (entity.weighted) {
        // Apply gravity to the entity's vertical speed
        float acceleration = GRAVITY * deltaTime;
        entity.velocity.y += acceleration;
        // Update entity's position based on its velocity
        entity.move(0, entity.velocity.y);
    }
}
