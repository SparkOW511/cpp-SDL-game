#pragma once

#include "ECS.hpp"
#include "Components.hpp"
#include "../Vector2D.hpp"

class EnemyAIComponent : public Component {
    public:
        Manager& manager;
        TransformComponent* transform;
        SpriteComponent* sprite;
        float chaseRange = 250.0f;
        float minDistance = 50.0f;
        float backoffDistance = 45.0f;
        float moveSpeed = 1.0f;
        Vector2D lastPosition;
        bool wasMoving = false;
        Vector2D lastDirection;
        SDL_RendererFlip lastFlip = SDL_FLIP_NONE;
        const char* currentAnimation = "Idle";

        bool isStuck = false;
        Uint32 stuckTimer = 0;
        const Uint32 STUCK_DURATION = 500;
        Vector2D unstuckDirection = {0,0};

        EnemyAIComponent(Manager& mManager) : manager(mManager) {}

        void setSpeed(float speed) {
            moveSpeed = speed;
        }

        void setChaseRange(float range) {
            chaseRange = range;
        }

        void init() override {
            transform = &entity->getComponent<TransformComponent>();
            sprite = &entity->getComponent<SpriteComponent>();
            lastPosition = transform->position;
            lastDirection = Vector2D(0, 1);
            isStuck = false;
            stuckTimer = 0;
        }

        void notifyTerrainCollision(bool collidedHorizontally, bool collidedVertically) {
            if (!isStuck) {
                isStuck = true;
                stuckTimer = SDL_GetTicks() + STUCK_DURATION;
                
                if (collidedHorizontally) {
                    auto& players = manager.getGroup(Game::groupPlayers);
                    if (!players.empty()) {
                        Vector2D playerPos = players[0]->getComponent<TransformComponent>().position;
                        unstuckDirection = {0, (playerPos.y > transform->position.y) ? 1.0f : -1.0f };
                    } else {
                        unstuckDirection = {0, (rand() % 2 == 0) ? 1.0f : -1.0f };
                    }
                } else {
                    auto& players = manager.getGroup(Game::groupPlayers);
                    if (!players.empty()) {
                        Vector2D playerPos = players[0]->getComponent<TransformComponent>().position;
                        unstuckDirection = {(playerPos.x > transform->position.x) ? 1.0f : -1.0f, 0 };
                    } else {
                        unstuckDirection = {(rand() % 2 == 0) ? 1.0f : -1.0f, 0 };
                    }
                }
            }
        }

        void update() override {
            if (!transform || !sprite) {
                return;
            }

            if (isStuck && SDL_GetTicks() > stuckTimer) {
                isStuck = false; 
            }

            if (isStuck) {
                transform->position.x += unstuckDirection.x * moveSpeed * 0.8f;
                transform->position.y += unstuckDirection.y * moveSpeed * 0.8f;
                
                determineAnimation(unstuckDirection);
                sprite->Play(currentAnimation);
                if (std::abs(unstuckDirection.x) > std::abs(unstuckDirection.y)) {
                    sprite->SetFlip(unstuckDirection.x > 0 ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
                    lastFlip = sprite->spriteFlip;
                }
                lastDirection = unstuckDirection;
                return;
            }

            if (!player || !player->isActive() || Game::gameOver || Game::questionActive) {
                sprite->Play("Idle");
                currentAnimation = "Idle";
                transform->velocity.x = 0;
                transform->velocity.y = 0;
                return;
            }

            Vector2D playerPos = player->getComponent<TransformComponent>().position;
            Vector2D enemyPos = transform->position;
            Vector2D direction = playerPos - enemyPos;
            float distance = direction.magnitude();
            
            bool isMoving = false;
            
            if(distance < chaseRange && distance > minDistance) {
                Vector2D moveDirection;
                
                if (distance > 0) {
                    moveDirection.x = direction.x / distance;
                    moveDirection.y = direction.y / distance;
                    
                    if (moveDirection.x != 0 && moveDirection.y != 0) {
                        moveDirection.x *= 0.7071f;
                        moveDirection.y *= 0.7071f;
                    }
                } else {
                    moveDirection.x = 0;
                    moveDirection.y = 0;
                }
                
                transform->position.x += moveDirection.x * moveSpeed;
                transform->position.y += moveDirection.y * moveSpeed;
                
                isMoving = true;
                lastDirection = moveDirection;
                
                determineAnimation(moveDirection);
                sprite->Play(currentAnimation);
                
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
            else if (distance <= backoffDistance) {
                Vector2D backoffDirection;
                
                if (distance > 0) {
                    backoffDirection.x = -direction.x / distance;
                    backoffDirection.y = -direction.y / distance;
                    
                    if (backoffDirection.x != 0 && backoffDirection.y != 0) {
                        backoffDirection.x *= 0.7071f;
                        backoffDirection.y *= 0.7071f;
                    }
                } else {
                    backoffDirection.x = 0;
                    backoffDirection.y = -1;
                }
                
                transform->position.x += backoffDirection.x * moveSpeed * 0.5f;
                transform->position.y += backoffDirection.y * moveSpeed * 0.5f;
                
                isMoving = true;
                
                sprite->Play(currentAnimation);
                
                if (std::abs(lastDirection.x) > std::abs(lastDirection.y)) {
                    sprite->SetFlip(lastFlip);
                }
            }
            else if (distance > backoffDistance && distance <= minDistance) {
                sprite->Play(currentAnimation);
                isMoving = false;
            }
            else {
                sprite->Play("Idle");
                currentAnimation = "Idle";
                
                if (distance < chaseRange) {
                    Vector2D idleDirection;
                    if (distance > 0) {
                        idleDirection.x = direction.x / distance;
                        idleDirection.y = direction.y / distance;
                        lastDirection = idleDirection;
                    }
                }
                
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
            
            lastPosition = transform->position;
            wasMoving = isMoving;
        }
        
    private:
        void determineAnimation(const Vector2D& direction) {
            if (std::abs(direction.x) > std::abs(direction.y)) {
                currentAnimation = "Walk";
            } else {
                if (direction.y > 0) {
                    currentAnimation = "WalkDown";
                } else {
                    currentAnimation = "WalkUp";
                }
            }
        }
}; 