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

        EnemyAIComponent(Manager& mManager) : manager(mManager) {}

        // Set the movement speed of the enemy
        void setSpeed(float speed) {
            moveSpeed = speed;
        }

        void init() override {
            transform = &entity->getComponent<TransformComponent>();
            sprite = &entity->getComponent<SpriteComponent>();
            lastPosition = transform->position;
            lastDirection = Vector2D(0, 1); // Default facing down
        }

        void update() override {
            auto& players = manager.getGroup(Game::groupPlayers);
            
            // If there's no player, game is over, or a question is active, reset to idle and stop moving
            if (players.empty() || Game::gameOver || Game::questionActive) {
                // Play idle animation
                sprite->Play("Idle");
                currentAnimation = "Idle";
                // Stop moving
                transform->velocity.x = 0;
                transform->velocity.y = 0;
                return;
            }

            auto& player = players[0];
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