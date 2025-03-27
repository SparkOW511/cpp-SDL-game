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
        const Uint32 shotAnimationDuration = 350; // Time for shot animation to complete
        int velX;
        int velY;
        int offsetY;
        int offsetX;
        SDL_RendererFlip bulletFlip;
        SDL_RendererFlip playerFlip;
        const char* direction;
        const char* playerDirection;
        bool isMoving;
        bool isShooting; // Track if player is shooting
        bool enabled = true; // Flag to enable/disable keyboard controls

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
            isShooting = false;
        }

        void update() override {
            // Skip input processing if controller is disabled
            if (!enabled) {
                // Reset velocity when disabled
                transform->velocity.x = 0;
                transform->velocity.y = 0;
                return;
            }
            
            const Uint8* keyState = SDL_GetKeyboardState(NULL);
            bool wasMoving = isMoving;
            bool wasShooting = isShooting;
            isMoving = false;
            
            // First, check if shooting animation should end
            if (isShooting && SDL_GetTicks() - lastShotTime >= shotAnimationDuration) {
                isShooting = false;
            }

            // Reset velocity
            transform->velocity.x = 0;
            transform->velocity.y = 0;

            // Process movement inputs
            // Handle vertical movement
            if (keyState[SDL_SCANCODE_W]) {
                transform->velocity.y = -1;
                isMoving = true;
            }
            else if (keyState[SDL_SCANCODE_S]) {
                transform->velocity.y = 1;
                isMoving = true;
            }
            
            // Handle horizontal movement
            if (keyState[SDL_SCANCODE_A]) {
                transform->velocity.x = -1;
                playerFlip = SDL_FLIP_HORIZONTAL;
                isMoving = true;
            }
            else if (keyState[SDL_SCANCODE_D]) {
                transform->velocity.x = 1;
                playerFlip = SDL_FLIP_NONE;
                isMoving = true;
            }
            
            // Update shooting parameters based on movement direction
            if (transform->velocity.y < 0) { // Moving up
                velX = 0;
                velY = -3;
                offsetY = 30;
                offsetX = 60;
                bulletFlip = SDL_FLIP_NONE;
                playerDirection = "ShootUp";
                direction = "bulletVertical";
            }
            else if (transform->velocity.y > 0) { // Moving down
                velX = 0;
                velY = 3;
                offsetY = 30;
                offsetX = 60;
                bulletFlip = SDL_FLIP_HORIZONTAL;
                playerDirection = "ShootDown";
                direction = "bulletVertical";
            }
            else if (transform->velocity.x != 0) { // Moving horizontally
                velY = 0;
                offsetY = 25;
                playerDirection = "Shoot";
                direction = "bulletHorizontal";
                
                if (transform->velocity.x < 0) { // Moving left
                    velX = -3;
                    offsetX = -35;
                    bulletFlip = SDL_FLIP_HORIZONTAL;
                }
                else { // Moving right
                    velX = 3;
                    offsetX = 65;
                    bulletFlip = SDL_FLIP_NONE;
                }
            }
            else {
                // Use last direction if not moving (for shooting while standing)
                if (playerFlip == SDL_FLIP_HORIZONTAL) {
                    velX = -3;
                    velY = 0;
                    offsetY = 25;
                    offsetX = -35;
                    bulletFlip = SDL_FLIP_HORIZONTAL;
                }
                else {
                    velX = 3;
                    velY = 0;
                    offsetY = 25;
                    offsetX = 65;
                    bulletFlip = SDL_FLIP_NONE;
                }
                playerDirection = "Shoot";
                direction = "bulletHorizontal";
            }

            // Handle shooting
            if (keyState[SDL_SCANCODE_SPACE]) {
                bool canShoot = true;
                if(ammo != nullptr) {
                    canShoot = ammo->canShoot();
                }
                
                if (SDL_GetTicks() - lastShotTime >= shotCooldown && canShoot) {
                    // Set shooting state
                    isShooting = true;
                    wasShooting = false; // Force animation change
                    
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
            
            // Animation handling - prioritize shooting over movement
            if (isShooting) {
                // Play or maintain shooting animation
                sprite->Play(playerDirection);
                sprite->SetFlip(playerFlip);
            } 
            else if (isMoving) {
                // Handle movement animations
                if (transform->velocity.y < 0) {
                    sprite->Play("WalkUp");
                } 
                else if (transform->velocity.y > 0) {
                    sprite->Play("WalkDown");
                }
                else if (transform->velocity.x != 0) {
                    sprite->Play("Walk");
                }
                sprite->SetFlip(playerFlip);
            } 
            else if (wasShooting || (!isMoving && wasMoving)) {
                // Just stopped shooting or moving - return to idle
                sprite->Play("Idle");
                sprite->SetFlip(playerFlip);
            }

            // Handle ESC to quit
            if (keyState[SDL_SCANCODE_ESCAPE]) {
                Game::isRunning = false;
            }
        }
};