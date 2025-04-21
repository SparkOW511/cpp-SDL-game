#pragma once

#include "../Game.hpp"
#include "ECS.hpp"
#include "Components.hpp"

class KeyboardController : public Component {
    public:
        TransformComponent *transform;
        SpriteComponent *sprite;
        AmmoComponent *ammo;
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
        const char* lastPlayerDirection;  // Track last shoot animation direction
        bool isMoving;
        bool isShooting; // Track if player is shooting
        bool wasShooting; // Track if player was shooting in the previous frame
        bool facingUp;    // Track if player is facing up
        bool facingDown;  // Track if player is facing down
        bool enabled = true; // Flag to enable/disable keyboard controls
        
        // Cooldown tracking after entering game state
        Uint32 gameStartTime = 0;
        const Uint32 gameStartCooldown = 250; // Reduced from 500 to 250 ms for faster state transition
        
        // Track state transitions and mouse button state
        GameState previousGameState = STATE_MAIN_MENU;
        bool wasMouseButtonDown = false;
        bool requireMouseRelease = true; // Force mouse release after state transitions

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
            gameStartTime = SDL_GetTicks(); // Set initial cooldown time on initialization
            isMoving = false;
            isShooting = false;
            wasShooting = false;
            facingUp = false;
            facingDown = false;
            lastPlayerDirection = "Idle";
            previousGameState = Game::gameState;
            wasMouseButtonDown = false;
            requireMouseRelease = true; // Initially require a mouse release
        }

        void update() override {
            // Skip input processing if controller is disabled
            if (!enabled) {
                // Reset velocity when disabled
                transform->velocity.x = 0;
                transform->velocity.y = 0;
                return;
            }
            
            // Get current state of mouse button
            int mouseX, mouseY;
            Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
            bool isMouseButtonDown = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
            
            // Check for state changes
            if (Game::gameState != previousGameState) {
                // Reset cooldown timer on state change
                gameStartTime = SDL_GetTicks();
                
                // If transitioning TO game state (e.g., from pause menu), require mouse release
                if (Game::gameState == STATE_GAME) {
                    requireMouseRelease = true;
                }
                
                previousGameState = Game::gameState;
            }
            
            // If mouse button was released, clear the requirement flag
            if (wasMouseButtonDown && !isMouseButtonDown) {
                requireMouseRelease = false;
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
            }
            else if (keyState[SDL_SCANCODE_S]) {
                transform->velocity.y = 1;
                isMoving = true;
                facingDown = true;
                facingUp = false;
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
                
                // If moving horizontally, reset vertical facing flags
                if (!keyState[SDL_SCANCODE_W] && !keyState[SDL_SCANCODE_S]) {
                    facingUp = false;
                    facingDown = false;
                }
            }
            
            // Convert mouse screen coordinates to world coordinates
            mouseX += Game::camera.x;
            mouseY += Game::camera.y;
            
            // Calculate differences between mouse and player center position
            int playerCenterX = transform->position.x + transform->width * transform->scale / 2;
            int playerCenterY = transform->position.y + transform->height * transform->scale / 2;
            int deltaX = mouseX - playerCenterX;
            int deltaY = mouseY - playerCenterY;
            
            // Determine shooting direction based on mouse position angle
            // Use atan2 to get the angle between player and mouse
            float angle = atan2(deltaY, deltaX) * 180.0f / M_PI;
            
            // Convert angle to 0-360 degrees for easier logic
            if (angle < 0) angle += 360.0f;
            
            // Determine shooting direction based on angle
            // Right: -45 to 45 degrees
            // Down: 45 to 135 degrees
            // Left: 135 to 225 degrees
            // Up: 225 to 315 degrees
            
            if ((angle >= 315.0f || angle < 45.0f)) {
                // Right
                velX = 3;
                velY = 0;
                offsetY = 25;
                offsetX = 65;
                bulletFlip = SDL_FLIP_NONE;
                playerFlip = SDL_FLIP_NONE; // Always face right when shooting right
                direction = "bulletHorizontal";
                lastPlayerDirection = "Shoot";
            }
            else if (angle >= 45.0f && angle < 135.0f) {
                // Down
                velX = 0;
                velY = 3;
                offsetY = 30;
                offsetX = 50; // Center offset
                bulletFlip = SDL_FLIP_VERTICAL;
                direction = "bulletVertical";
                lastPlayerDirection = "ShootDown";
                // For vertical shooting, never flip the sprite
                sprite->SetFlip(SDL_FLIP_NONE);
            }
            else if (angle >= 135.0f && angle < 225.0f) {
                // Left
                velX = -3;
                velY = 0;
                offsetY = 25;
                offsetX = -35;
                bulletFlip = SDL_FLIP_HORIZONTAL;
                playerFlip = SDL_FLIP_HORIZONTAL; // Always face left when shooting left
                direction = "bulletHorizontal";
                lastPlayerDirection = "Shoot";
            }
            else {
                // Up
                velX = 0;
                velY = -3;
                offsetY = 30;
                offsetX = 50; // Center offset
                bulletFlip = SDL_FLIP_NONE;
                direction = "bulletVertical";
                lastPlayerDirection = "ShootUp";
                // For vertical shooting, never flip the sprite
                sprite->SetFlip(SDL_FLIP_NONE);
            }

            // Handle shooting with left mouse button
            Uint32 currentTime = SDL_GetTicks();
            bool startCooldownPassed = (currentTime - gameStartTime >= gameStartCooldown);
            
            // Consider the cooldown passed when player starts moving
            if (Game::gameState == STATE_GAME && isMoving) {
                startCooldownPassed = true;
            }
            
            // Only allow shooting if all conditions are met
            bool canShootNow = Game::gameState == STATE_GAME && 
                              !Game::questionActive && 
                              !Game::gameOver && 
                              isMouseButtonDown && 
                              !requireMouseRelease && 
                              startCooldownPassed;
            
            // Check if this is a new press or if cooldown has passed
            bool cooldownPassed = (currentTime - lastShotTime >= shotCooldown);
            bool isNewPress = isMouseButtonDown && !wasMouseButtonDown;
            
            if (canShootNow && (isNewPress || cooldownPassed)) {
                bool hasAmmo = true;
                if (ammo != nullptr) {
                    hasAmmo = ammo->canShoot();
                }
                
                if (hasAmmo) {
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
                    lastShotTime = currentTime;
                }
            }
            
            // Update mouse button state for next frame
            wasMouseButtonDown = isMouseButtonDown;
            
            // Animation handling - update player appearance based on current state
            if (isShooting) {
                // Play shooting animation in the appropriate direction
                sprite->Play(lastPlayerDirection);
                // Flip was already set in the direction calculation for vertical shots
                if (lastPlayerDirection == "Shoot") {
                    sprite->SetFlip(playerFlip);
                }
            } 
            else if (isMoving) {
                // Always update player flip based on horizontal movement
                if (transform->velocity.x < 0) {
                    playerFlip = SDL_FLIP_HORIZONTAL;
                }
                else if (transform->velocity.x > 0) {
                    playerFlip = SDL_FLIP_NONE;
                }
                
                // Handle movement animations
                if (transform->velocity.y < 0) {
                    sprite->Play("WalkUp");
                    sprite->SetFlip(SDL_FLIP_NONE); // Never flip up animation
                } 
                else if (transform->velocity.y > 0) {
                    sprite->Play("WalkDown");
                    sprite->SetFlip(SDL_FLIP_NONE); // Never flip down animation
                }
                else if (transform->velocity.x != 0) {
                    sprite->Play("Walk");
                    sprite->SetFlip(playerFlip);
                }
            } 
            else if (wasShooting || (!isMoving && wasMoving)) {
                // Just stopped shooting or moving - return to idle
                sprite->Play("Idle");
                sprite->SetFlip(playerFlip);
            }
        }
};