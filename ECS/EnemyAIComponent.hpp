#pragma once

#include "ECS.hpp"
#include "Components.hpp"
#include "../Vector2D.hpp"

class EnemyAIComponent : public Component {
    public:
        Manager& manager;
        TransformComponent* transform;
        SpriteComponent* sprite;
        float chaseRange = 250.0f;  // Distance to start chasing
        float minDistance = 50.0f;  // Minimum distance to keep from player
        float backoffDistance = 45.0f; // Distance to start backing off (slightly less than minDistance)
        float moveSpeed = 1.0f; // Reduced from 1.7f to correct overall speed
        Vector2D lastPosition; // Track last position to detect when enemy stops
        bool wasMoving = false; // Track if enemy was moving last frame
        Vector2D lastDirection; // Keep track of last movement direction
        SDL_RendererFlip lastFlip = SDL_FLIP_NONE; // Track last flip direction
        const char* currentAnimation = "Idle"; // Track current animation

        // Collision state variables
        bool isStuck = false;
        Uint32 stuckTimer = 0; // Timer for how long to attempt unstuck maneuver
        const Uint32 STUCK_DURATION = 500; // milliseconds
        Vector2D unstuckDirection = {0,0};

        EnemyAIComponent(Manager& mManager) : manager(mManager) {}

        // Set the movement speed of the enemy
        void setSpeed(float speed) {
            moveSpeed = speed;
        }

        // Set the chase range of the enemy
        void setChaseRange(float range) {
            chaseRange = range;
        }

        void init() override {
            transform = &entity->getComponent<TransformComponent>();
            sprite = &entity->getComponent<SpriteComponent>();
            lastPosition = transform->position;
            lastDirection = Vector2D(0, 1); // Default facing down
            isStuck = false; // Ensure starts unstuck
            stuckTimer = 0;
        }

        // Called from Game::update when enemy collides with terrain
        void notifyTerrainCollision(bool collidedHorizontally, bool collidedVertically) {
            if (!isStuck) { // Only trigger if not already stuck
                isStuck = true;
                stuckTimer = SDL_GetTicks() + STUCK_DURATION; // Set timer for unstuck duration
                
                // Determine an initial direction to move away from the collision
                if (collidedHorizontally) {
                    // Stuck horizontally, try moving vertically (prefer player's Y direction)
                    auto& players = manager.getGroup(Game::groupPlayers);
                    if (!players.empty()) {
                        Vector2D playerPos = players[0]->getComponent<TransformComponent>().position;
                        unstuckDirection = {0, (playerPos.y > transform->position.y) ? 1.0f : -1.0f };
                    } else {
                        unstuckDirection = {0, (rand() % 2 == 0) ? 1.0f : -1.0f }; // Random vertical if no player
                    }
                } else { // collidedVertically
                    // Stuck vertically, try moving horizontally (prefer player's X direction)
                    auto& players = manager.getGroup(Game::groupPlayers);
                    if (!players.empty()) {
                        Vector2D playerPos = players[0]->getComponent<TransformComponent>().position;
                        unstuckDirection = {(playerPos.x > transform->position.x) ? 1.0f : -1.0f, 0 };
                    } else {
                        unstuckDirection = {(rand() % 2 == 0) ? 1.0f : -1.0f, 0 }; // Random horizontal if no player
                    }
                }
            }
        }

        void update() override {
            // ---> ADD NULL CHECK FOR TRANSFORM/SPRITE <---
            if (!transform || !sprite) {
                return;
            }
            // ---------------------------------------------

            // Check if stuck timer has expired
            if (isStuck && SDL_GetTicks() > stuckTimer) {
                isStuck = false; 
            }

            // If stuck, perform unstuck maneuver instead of normal AI
            if (isStuck) {
                // Move in the determined unstuck direction
                transform->position.x += unstuckDirection.x * moveSpeed * 0.8f; // Slightly slower unstuck move
                transform->position.y += unstuckDirection.y * moveSpeed * 0.8f;
                
                // Use appropriate animation for unstuck movement
                determineAnimation(unstuckDirection);
                sprite->Play(currentAnimation);
                if (std::abs(unstuckDirection.x) > std::abs(unstuckDirection.y)) {
                    sprite->SetFlip(unstuckDirection.x > 0 ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
                    lastFlip = sprite->spriteFlip;
                }
                lastDirection = unstuckDirection; // Update facing direction
                return; // Skip normal AI logic
            }

            // Use the global player pointer directly for safety after load
            if (!player || !player->isActive() || Game::gameOver || Game::questionActive) {
                // Play idle animation
                sprite->Play("Idle");
                currentAnimation = "Idle";
                // Stop moving
                transform->velocity.x = 0;
                transform->velocity.y = 0;
                return;
            }

            // Player is guaranteed to be valid here due to the check above
            Vector2D playerPos = player->getComponent<TransformComponent>().position;
            Vector2D enemyPos = transform->position;
            Vector2D direction = playerPos - enemyPos;
            float distance = direction.magnitude();
            
            bool isMoving = false;
            
            // Only chase if player is within chase range but not too close
            if(distance < chaseRange && distance > minDistance) {
                // Get direction but don't normalize yet
                Vector2D moveDirection;
                
                // Avoid division by zero
                if (distance > 0) {
                    moveDirection.x = direction.x / distance; // Manual normalization
                    moveDirection.y = direction.y / distance;
                    
                    // Fix diagonal movement
                    if (moveDirection.x != 0 && moveDirection.y != 0) {
                        // Apply sqrt(2)/2 to normalize diagonal speed
                        moveDirection.x *= 0.7071f;
                        moveDirection.y *= 0.7071f;
                    }
                } else {
                    moveDirection.x = 0;
                    moveDirection.y = 0;
                }
                
                // Apply movement
                transform->position.x += moveDirection.x * moveSpeed;
                transform->position.y += moveDirection.y * moveSpeed;
                
                isMoving = true;
                lastDirection = moveDirection;
                
                // Set animation based on movement direction
                determineAnimation(moveDirection);
                // Actually play the animation we determined
                sprite->Play(currentAnimation);
                
                // Set flip for horizontal movement
                if (std::abs(moveDirection.x) > std::abs(moveDirection.y)) {
                    if (moveDirection.x > 0) {
                        sprite->SetFlip(SDL_FLIP_NONE);
                        lastFlip = SDL_FLIP_NONE;
                    } else {
                        sprite->SetFlip(SDL_FLIP_HORIZONTAL);
                        lastFlip = SDL_FLIP_HORIZONTAL;
                    }
                }
            }
            // If too close, move away slightly to maintain minimum distance
            else if (distance <= backoffDistance) {
                // Calculate backoff direction (away from player)
                Vector2D backoffDirection;
                
                // Avoid division by zero
                if (distance > 0) {
                    // First get normalized direction from player to enemy
                    backoffDirection.x = -direction.x / distance;
                    backoffDirection.y = -direction.y / distance;
                    
                    // Fix diagonal movement
                    if (backoffDirection.x != 0 && backoffDirection.y != 0) {
                        backoffDirection.x *= 0.7071f;
                        backoffDirection.y *= 0.7071f;
                    }
                } else {
                    // Default backoff if at same position
                    backoffDirection.x = 0;
                    backoffDirection.y = -1;
                }
                
                // Apply slower backoff movement
                transform->position.x += backoffDirection.x * moveSpeed * 0.5f;
                transform->position.y += backoffDirection.y * moveSpeed * 0.5f;
                
                isMoving = true;
                
                // Keep using the same animation but don't change it
                sprite->Play(currentAnimation);
                
                // Keep using the same flip state
                if (std::abs(lastDirection.x) > std::abs(lastDirection.y)) {
                    sprite->SetFlip(lastFlip);
                }
            }
            // If in the buffer zone, just idle to prevent oscillation
            else if (distance > backoffDistance && distance <= minDistance) {
                // Just stay still in the buffer zone to prevent jittering
                sprite->Play(currentAnimation);
                isMoving = false;
            }
            // If not moving, set to idle but keep facing direction
            else {
                // Play idle animation but preserve facing direction
                sprite->Play("Idle");
                currentAnimation = "Idle";
                
                // If within chase range, update facing direction  
                if (distance < chaseRange) {
                    // Create a normalized direction vector toward player
                    Vector2D idleDirection;
                    if (distance > 0) {
                        idleDirection.x = direction.x / distance;
                        idleDirection.y = direction.y / distance;
                        lastDirection = idleDirection;
                    }
                }
                
                // Maintain flip based on horizontal direction
                if (std::abs(lastDirection.x) > std::abs(lastDirection.y)) {
                    if (lastDirection.x > 0) {
                        sprite->SetFlip(SDL_FLIP_NONE);
                        lastFlip = SDL_FLIP_NONE;
                    } else {
                        sprite->SetFlip(SDL_FLIP_HORIZONTAL);
                        lastFlip = SDL_FLIP_HORIZONTAL;
                    }
                }
            }
            
            // Update tracking variables
            lastPosition = transform->position;
            wasMoving = isMoving;
        }
        
    private:
        // Determine animation based on direction without immediately playing it
        void determineAnimation(const Vector2D& direction) {
            if (std::abs(direction.x) > std::abs(direction.y)) {
                // Horizontal movement/facing
                currentAnimation = "Walk";
            } else {
                // Vertical movement/facing
                if (direction.y > 0) {
                    // Moving down
                    currentAnimation = "WalkDown";
                } else {
                    // Moving up
                    currentAnimation = "WalkUp";
                }
            }
        }
}; 