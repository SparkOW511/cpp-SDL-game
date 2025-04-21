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
        const Uint32 shotAnimationDuration = 350;
        int velX;
        int velY;
        int offsetY;
        int offsetX;
        SDL_RendererFlip bulletFlip;
        SDL_RendererFlip playerFlip;
        const char* direction;
        const char* lastPlayerDirection;
        bool isMoving;
        bool isShooting;
        bool wasShooting;
        bool facingUp;
        bool facingDown;
        bool enabled = true;
        
        Uint32 gameStartTime = 0;
        const Uint32 gameStartCooldown = 250;
        
        GameState previousGameState = STATE_MAIN_MENU;
        bool wasMouseButtonDown = false;
        bool requireMouseRelease = true;

        void init() override {
            transform = &entity->getComponent<TransformComponent>();
            sprite = &entity->getComponent<SpriteComponent>();
            
            if(entity->hasComponent<AmmoComponent>()) {
                ammo = &entity->getComponent<AmmoComponent>();
            } else {
                ammo = nullptr;
            }
            
            lastShotTime = 0;
            gameStartTime = SDL_GetTicks();
            isMoving = false;
            isShooting = false;
            wasShooting = false;
            facingUp = false;
            facingDown = false;
            lastPlayerDirection = "Idle";
            previousGameState = Game::gameState;
            wasMouseButtonDown = false;
            requireMouseRelease = true;
        }

        void update() override {
            if (!enabled) {
                transform->velocity.x = 0;
                transform->velocity.y = 0;
                return;
            }
            
            int mouseX, mouseY;
            Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
            bool isMouseButtonDown = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
            
            if (Game::gameState != previousGameState) {
                gameStartTime = SDL_GetTicks();
                
                if (Game::gameState == STATE_GAME) {
                    requireMouseRelease = true;
                }
                
                previousGameState = Game::gameState;
            }
            
            if (wasMouseButtonDown && !isMouseButtonDown) {
                requireMouseRelease = false;
            }
            
            const Uint8* keyState = SDL_GetKeyboardState(NULL);
            bool wasMoving = isMoving;
            wasShooting = isShooting;
            isMoving = false;
            
            if (isShooting && SDL_GetTicks() - lastShotTime >= shotAnimationDuration) {
                isShooting = false;
            }

            transform->velocity.x = 0;
            transform->velocity.y = 0;

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
                if (!isMoving) {
                    facingUp = false;
                    facingDown = false;
                }
            }
            
            if (keyState[SDL_SCANCODE_A]) {
                transform->velocity.x = -1;
                playerFlip = SDL_FLIP_HORIZONTAL;
                isMoving = true;
                
                if (!keyState[SDL_SCANCODE_W] && !keyState[SDL_SCANCODE_S]) {
                    facingUp = false;
                    facingDown = false;
                }
            }
            else if (keyState[SDL_SCANCODE_D]) {
                transform->velocity.x = 1;
                playerFlip = SDL_FLIP_NONE;
                isMoving = true;
                
                if (!keyState[SDL_SCANCODE_W] && !keyState[SDL_SCANCODE_S]) {
                    facingUp = false;
                    facingDown = false;
                }
            }
            
            mouseX += Game::camera.x;
            mouseY += Game::camera.y;
            
            int playerCenterX = transform->position.x + transform->width * transform->scale / 2;
            int playerCenterY = transform->position.y + transform->height * transform->scale / 2;
            int deltaX = mouseX - playerCenterX;
            int deltaY = mouseY - playerCenterY;
            
            float angle = atan2(deltaY, deltaX) * 180.0f / M_PI;
            
            if (angle < 0) angle += 360.0f;
            
            if ((angle >= 315.0f || angle < 45.0f)) {
                velX = 3;
                velY = 0;
                offsetY = 25;
                offsetX = 65;
                bulletFlip = SDL_FLIP_NONE;
                playerFlip = SDL_FLIP_NONE;
                direction = "bulletHorizontal";
                lastPlayerDirection = "Shoot";
            }
            else if (angle >= 45.0f && angle < 135.0f) {
                velX = 0;
                velY = 3;
                offsetY = 30;
                offsetX = 50;
                bulletFlip = SDL_FLIP_VERTICAL;
                direction = "bulletVertical";
                lastPlayerDirection = "ShootDown";
                sprite->SetFlip(SDL_FLIP_NONE);
            }
            else if (angle >= 135.0f && angle < 225.0f) {
                velX = -3;
                velY = 0;
                offsetY = 25;
                offsetX = -35;
                bulletFlip = SDL_FLIP_HORIZONTAL;
                playerFlip = SDL_FLIP_HORIZONTAL;
                direction = "bulletHorizontal";
                lastPlayerDirection = "Shoot";
            }
            else {
                velX = 0;
                velY = -3;
                offsetY = 30;
                offsetX = 50;
                bulletFlip = SDL_FLIP_NONE;
                direction = "bulletVertical";
                lastPlayerDirection = "ShootUp";
                sprite->SetFlip(SDL_FLIP_NONE);
            }

            Uint32 currentTime = SDL_GetTicks();
            bool startCooldownPassed = (currentTime - gameStartTime >= gameStartCooldown);
            
            if (Game::gameState == STATE_GAME && isMoving) {
                startCooldownPassed = true;
            }
            
            bool canShootNow = Game::gameState == STATE_GAME && 
                              !Game::questionActive && 
                              !Game::gameOver && 
                              isMouseButtonDown && 
                              !requireMouseRelease && 
                              startCooldownPassed;
            
            bool cooldownPassed = (currentTime - lastShotTime >= shotCooldown);
            bool isNewPress = isMouseButtonDown && !wasMouseButtonDown;
            
            if (canShootNow && (isNewPress || cooldownPassed)) {
                bool hasAmmo = true;
                if (ammo != nullptr) {
                    hasAmmo = ammo->canShoot();
                }
                
                if (hasAmmo) {
                    isShooting = true;
                    
                    Game::assets->CreateProjectile(
                        Vector2D(transform->position.x + offsetX, transform->position.y + offsetY),
                        Vector2D(velX, velY),
                        250,
                        1,
                        direction,
                        bulletFlip
                    );
                    
                    Game::assets->PlaySound("shoot", Game::volumeLevel);
                    
                    if(ammo != nullptr) {
                        ammo->shoot();
                    }
                    
                    lastShotTime = currentTime;
                }
            }
            
            wasMouseButtonDown = isMouseButtonDown;
            
            if (isShooting) {
                sprite->Play(lastPlayerDirection);
                if (lastPlayerDirection == "Shoot") {
                    sprite->SetFlip(playerFlip);
                }
            } 
            else if (isMoving) {
                if (transform->velocity.x < 0) {
                    playerFlip = SDL_FLIP_HORIZONTAL;
                }
                else if (transform->velocity.x > 0) {
                    playerFlip = SDL_FLIP_NONE;
                }
                
                if (transform->velocity.y < 0) {
                    sprite->Play("WalkUp");
                    sprite->SetFlip(SDL_FLIP_NONE);
                } 
                else if (transform->velocity.y > 0) {
                    sprite->Play("WalkDown");
                    sprite->SetFlip(SDL_FLIP_NONE);
                }
                else if (transform->velocity.x != 0) {
                    sprite->Play("Walk");
                    sprite->SetFlip(playerFlip);
                }
            } 
            else if (wasShooting || (!isMoving && wasMoving)) {
                sprite->Play("Idle");
                sprite->SetFlip(playerFlip);
            }
        }
};