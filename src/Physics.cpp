#include "Physics.h"

/**
 * Creates a physics entity.
 */
Physics::Physics(float gravity) : gravity(gravity) {}

/**
 * Applies gravity.
 * 
 * Based on physics knowledge.
 */
void Physics::applyGravity(Entity& entity, float deltaTime) {
    // Apply gravity to the entity's vertical speed
    float acceleration = gravity * deltaTime;
    entity.velocity.y += acceleration;
    // Update entity's position based on its velocity
    entity.move(0, entity.velocity.y);
}