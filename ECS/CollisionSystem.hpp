#pragma once

#include "ECS.hpp"
#include "Components.hpp"
#include "../Collision.hpp"

class CollisionSystem : public System {
    private:
        Manager& manager;
        float damageTimer = 1.0f; // Start with cooldown active
        const float damageCooldown = 0.3f; // 300ms cooldown
        float objectCollisionDelay = 1.0f; // Delay before object collisions are active (1 second)
        bool objectCollisionsEnabled = false;
    
    public:
        CollisionSystem(Manager& mManager) : manager(mManager) {}
        
        void update() override {
            auto& players = manager.getGroup(Game::groupPlayers);
            auto& enemies = manager.getGroup(Game::groupEnemies);
            auto& colliders = manager.getGroup(Game::groupColliders);
            auto& projectiles = manager.getGroup(Game::groupProjectiles);
            auto& objects = manager.getGroup(Game::groupObjects);

            if (players.empty()) return;

            // Update timers
            damageTimer -= 1.0f/60.0f;
            
            if (!objectCollisionsEnabled) {
                objectCollisionDelay -= 1.0f/60.0f;
                if (objectCollisionDelay <= 0.0f) {
                    objectCollisionsEnabled = true;
                }
            }

            auto& player = players[0];
            SDL_Rect playerCol = player->getComponent<ColliderComponent>().collider;
            Vector2D playerPos = player->getComponent<TransformComponent>().position;

            // Player collision with terrain
            for(auto& c : colliders) {
                SDL_Rect cCol = c->getComponent<ColliderComponent>().collider;
                if(Collision::AABB(cCol, playerCol)) {
                    player->getComponent<TransformComponent>().position = playerPos;
                }
            }

            // Player collision with objects (clues) - only if enabled
            if (objectCollisionsEnabled) {
                for(auto& o : objects) {
                    if(Collision::AABB(player->getComponent<ColliderComponent>().collider, 
                                      o->getComponent<ColliderComponent>().collider)) {
                        std::cout << "Clue picked up" << std::endl;
                        player->getComponent<HealthComponent>().heal(20);
                        o->destroy();
                    }
                }
            }

            // Enemy collision with projectiles and player
            for(auto& e : enemies) {
                // Projectile collision
                for(auto& p : projectiles) {
                    if(Collision::AABB(e->getComponent<ColliderComponent>().collider, 
                                      p->getComponent<ColliderComponent>().collider)) {
                        std::cout << "Hit enemy" << std::endl;
                        e->getComponent<HealthComponent>().takeDamage(25);
                        p->destroy();
                    }
                }

                // Player collision with damage cooldown
                if(Collision::AABB(playerCol, e->getComponent<ColliderComponent>().collider) && damageTimer <= 0) {
                    player->getComponent<HealthComponent>().takeDamage(5);
                    damageTimer = damageCooldown;
                }

                // Destroy dead enemies
                if(e->getComponent<HealthComponent>().health <= 0) {
                    e->destroy();
                }
            }
        }
}; 