#pragma once

#include "ECS.hpp"
#include "Components.hpp"
#include "../Vector2D.hpp"

class EnemyAIComponent : public Component {
    public:
        Manager& manager;
        TransformComponent* transform;
        SpriteComponent* sprite;
        float chaseRange = 200.0f;
        float moveSpeed = 1.5f;

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
            
            if(distance < chaseRange) {
                direction = direction.normalize();
                transform->position += direction * moveSpeed;
                
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