#pragma once

#include "../Game.hpp"
#include "ECS.hpp"
#include "Components.hpp"

class KeyboardController : public Component {
    public:
        TransformComponent *transform;
        SpriteComponent *sprite;
        AmmoComponent *ammo;
        char playerInput;
        Uint32 lastShotTime;
        const Uint32 shotCooldown = 350;
        int velX;
        int velY;
        int offsetY;
        int offsetX;
        SDL_RendererFlip bulletFlip;
        SDL_RendererFlip playerFlip;
        const char* direction;
        const char* playerDirection;
        bool isMoving;

        void init() override {
            transform = &entity->getComponent<TransformComponent>();
            sprite = &entity->getComponent<SpriteComponent>();
            
            // Check if entity has AmmoComponent
            if(entity->hasComponent<AmmoComponent>()) {
                ammo = &entity->getComponent<AmmoComponent>();
            } else {
                ammo = nullptr;
            }
            
            lastShotTime = 0;
            isMoving = false;
        }

        void update() override {
            const Uint8* keyState = SDL_GetKeyboardState(NULL);
            bool wasMoving = isMoving;
            isMoving = false;

            // Reset velocity
            transform->velocity.x = 0;
            transform->velocity.y = 0;

            // Handle movement
            if (keyState[SDL_SCANCODE_W]) {
                transform->velocity.y = -1;
                sprite->Play("WalkUp");
                playerInput = 'w';
                velX = 0;
                velY = -3;
                offsetY = 30;
                offsetX = 60;
                bulletFlip = SDL_FLIP_NONE;
                playerFlip = SDL_FLIP_NONE;
                playerDirection = "ShootUp";
                direction = "bulletVertical";
                isMoving = true;
            }
            if (keyState[SDL_SCANCODE_S]) {
                transform->velocity.y = 1;
                sprite->Play("WalkDown");
                playerInput = 's';
                velX = 0;
                velY = 3;
                offsetY = 30;
                offsetX = 60;
                bulletFlip = SDL_FLIP_HORIZONTAL;
                playerFlip = SDL_FLIP_NONE;
                playerDirection = "ShootDown";
                direction = "bulletVertical";
                isMoving = true;
            }
            if (keyState[SDL_SCANCODE_A]) {
                transform->velocity.x = -1;
                sprite->Play("Walk");
                sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
                playerInput = 'a';
                velX = -3;
                velY = 0;
                offsetY = 25;
                offsetX = -35;
                bulletFlip = SDL_FLIP_HORIZONTAL;
                playerFlip = SDL_FLIP_HORIZONTAL;
                playerDirection = "Shoot";
                direction = "bulletHorizontal";
                isMoving = true;
            }
            if (keyState[SDL_SCANCODE_D]) {
                transform->velocity.x = 1;
                sprite->Play("Walk");
                playerInput = 'd';
                velX = 3;
                velY = 0;
                offsetY = 25;
                offsetX = 65;
                bulletFlip = SDL_FLIP_NONE;
                playerFlip = SDL_FLIP_NONE;
                playerDirection = "Shoot";
                direction = "bulletHorizontal";
                isMoving = true;
            }

            // Handle shooting
            if (keyState[SDL_SCANCODE_SPACE]) {
                bool canShoot = true;
                if(ammo != nullptr) {
                    canShoot = ammo->canShoot();
                }
                
                if (SDL_GetTicks() - lastShotTime >= shotCooldown && canShoot) {
                    // Play shooting animation
                    sprite->Play(playerDirection);
                    sprite->SetFlip(playerFlip);
                    
                    // Create projectile
                    Game::assets->CreateProjectile(
                        Vector2D(transform->position.x + offsetX, transform->position.y + offsetY),
                        Vector2D(velX, velY),
                        250,
                        1,
                        direction,
                        bulletFlip
                    );
                    
                    // Decrease ammo
                    if(ammo != nullptr) {
                        ammo->shoot();
                    }
                    
                    // Reset shot cooldown timer
                    lastShotTime = SDL_GetTicks();
                }
            }
            else if (!isMoving && wasMoving) {
                // Return to idle animation when not moving and not shooting
                sprite->Play("Idle");
                sprite->spriteFlip = playerFlip;
            }

            // Handle ESC to quit
            if (keyState[SDL_SCANCODE_ESCAPE]) {
                Game::isRunning = false;
            }
        }
};