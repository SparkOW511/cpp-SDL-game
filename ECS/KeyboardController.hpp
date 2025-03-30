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
        const char* lastPlayerDirection;  // Track last shoot animation direction
        bool isMoving;
        bool isShooting; // Track if player is shooting
        bool wasShooting; // Track if player was shooting in the previous frame
        bool facingUp;    // Track if player is facing up
        bool facingDown;  // Track if player is facing down
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
            wasShooting = false;
            facingUp = false;
            facingDown = false;
            playerDirection = "Idle";
            lastPlayerDirection = "Idle";
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
            wasShooting = isShooting;
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
                facingUp = true;
                facingDown = false;
                
                // Update last direction for shooting
                playerDirection = "ShootUp";
                lastPlayerDirection = "ShootUp";
            }
            else if (keyState[SDL_SCANCODE_S]) {
                transform->velocity.y = 1;
                isMoving = true;
                facingDown = true;
                facingUp = false;
                
                // Update last direction for shooting
                playerDirection = "ShootDown";
                lastPlayerDirection = "ShootDown";
            }
            else {
                // Keep the facing direction but stop moving vertically
                if (!isMoving) {
                    // Only reset these if no other movement
                    // This preserves diagonal movement intent
                    facingUp = false;
                    facingDown = false;
                }
            }
            
            // Handle horizontal movement
            if (keyState[SDL_SCANCODE_A]) {
                transform->velocity.x = -1;
                playerFlip = SDL_FLIP_HORIZONTAL;
                isMoving = true;
                
                // Update last direction for shooting
                playerDirection = "Shoot";
                lastPlayerDirection = "Shoot";
                
                // If moving horizontally, reset vertical facing flags
                if (!keyState[SDL_SCANCODE_W] && !keyState[SDL_SCANCODE_S]) {
                    facingUp = false;
                    facingDown = false;
                }
            }
            else if (keyState[SDL_SCANCODE_D]) {
                transform->velocity.x = 1;
                playerFlip = SDL_FLIP_NONE;
                isMoving = true;
                
                // Update last direction for shooting
                playerDirection = "Shoot";
                lastPlayerDirection = "Shoot";
                
                // If moving horizontally, reset vertical facing flags
                if (!keyState[SDL_SCANCODE_W] && !keyState[SDL_SCANCODE_S]) {
                    facingUp = false;
                    facingDown = false;
                }
            }
            
            // Update shooting parameters based on last movement direction
            if (facingUp || transform->velocity.y < 0 || lastPlayerDirection == "ShootUp") { // Facing or moving up
                velX = 0;
                velY = -3;
                offsetY = 30;
                
                // Adjust offset based on the player's horizontal orientation
                if (playerFlip == SDL_FLIP_HORIZONTAL) {
                    offsetX = 40; // Offset for left-facing player shooting up
                } else {
                    offsetX = 60; // Offset for right-facing player shooting up
                }
                
                bulletFlip = SDL_FLIP_NONE;
                direction = "bulletVertical";
                playerDirection = "ShootUp";
                // Don't change playerFlip here - maintain horizontal orientation
            }
            else if (facingDown || transform->velocity.y > 0 || lastPlayerDirection == "ShootDown") { // Facing or moving down
                velX = 0;
                velY = 3;
                offsetY = 30;
                
                // Adjust offset based on the player's horizontal orientation
                if (playerFlip == SDL_FLIP_HORIZONTAL) {
                    offsetX = 40; // Offset for left-facing player shooting down
                } else {
                    offsetX = 60; // Offset for right-facing player shooting down
                }
                
                bulletFlip = SDL_FLIP_HORIZONTAL;
                direction = "bulletVertical";
                playerDirection = "ShootDown";
                // Don't change playerFlip here - maintain horizontal orientation
            }
            else if (transform->velocity.x != 0 || lastPlayerDirection == "Shoot") { // Moving horizontally
                velY = 0;
                offsetY = 25;
                direction = "bulletHorizontal";
                playerDirection = "Shoot";
                
                if (transform->velocity.x < 0 || playerFlip == SDL_FLIP_HORIZONTAL) { // Moving left or facing left
                    velX = -3;
                    offsetX = -35;
                    bulletFlip = SDL_FLIP_HORIZONTAL;
                    playerFlip = SDL_FLIP_HORIZONTAL;
                }
                else { // Moving right or facing right
                    velX = 3;
                    offsetX = 65;
                    bulletFlip = SDL_FLIP_NONE;
                    playerFlip = SDL_FLIP_NONE;
                }
            }
            else {
                // Default if no other condition matches
                // Use last stored facing based on flip
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
                lastPlayerDirection = "Shoot";
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
                // Play shooting animation in the appropriate direction
                sprite->Play(lastPlayerDirection);
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
                // Determine idle animation based on last direction
                sprite->Play("Idle");
                sprite->SetFlip(playerFlip);
            }

            // Handle ESC to quit
            if (keyState[SDL_SCANCODE_ESCAPE]) {
                Game::isRunning = false;
            }
        }
};