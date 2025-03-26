#pragma once

#include "ECS.hpp"
#include "Components.hpp"
#include "../Vector2D.hpp"

class EnemyAIComponent : public Component {
    public:
        Manager& manager;
        TransformComponent* transform;
        SpriteComponent* sprite;
        float chaseRange = 200.0f;  // Distance to start chasing
        float minDistance = 50.0f;  // Minimum distance to keep from player
        float moveSpeed = 1.7f;

        EnemyAIComponent(Manager& mManager) : manager(mManager) {}

        void init() override {
            transform = &entity->getComponent<TransformComponent>();
            sprite = &entity->getComponent<SpriteComponent>();
        }

        void update() override {
            auto& players = manager.getGroup(Game::groupPlayers);
            if (players.empty()) return;

            auto& player = players[0];
            Vector2D playerPos = player->getComponent<TransformComponent>().position;
            Vector2D enemyPos = transform->position;
            Vector2D direction = playerPos - enemyPos;
            float distance = direction.magnitude();

            sprite->Play("Idle");
            
            // Only chase if player is within chase range but not too close
            if(distance < chaseRange && distance > minDistance) {
                direction = direction.normalize();
                transform->position += direction * moveSpeed;
                
                // Set animation based on movement direction
                if (std::abs(direction.x) > std::abs(direction.y)) {
                    sprite->Play("Walk");
                    if (direction.x > 0) {
                        sprite->SetFlip(SDL_FLIP_NONE);
                    } else {
                        sprite->SetFlip(SDL_FLIP_HORIZONTAL);
                    }
                } else {
                    if (direction.y > 0) {
                        sprite->Play("WalkDown");
                    } else {
                        sprite->Play("WalkUp");
                    }
                }
            }
            // If too close, move away slightly to maintain minimum distance
            else if (distance <= minDistance) {
                // Move away from player
                direction = direction.normalize() * -1.0f; // Invert direction
                transform->position += direction * moveSpeed * 0.5f; // Slower backoff
                
                // Set animation based on movement direction
                if (std::abs(direction.x) > std::abs(direction.y)) {
                    sprite->Play("Walk");
                    if (direction.x > 0) {
                        sprite->SetFlip(SDL_FLIP_NONE);
                    } else {
                        sprite->SetFlip(SDL_FLIP_HORIZONTAL);
                    }
                } else {
                    if (direction.y > 0) {
                        sprite->Play("WalkDown");
                    } else {
                        sprite->Play("WalkUp");
                    }
                }
            }
        }
}; 