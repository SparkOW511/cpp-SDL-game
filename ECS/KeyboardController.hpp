#pragma once

#include "../Game.hpp"
#include "ECS.hpp"
#include "Components.hpp"

class KeyboardController : public Component {
    public:
        TransformComponent *transform;
        SpriteComponent *sprite;
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

        void init() override {
            transform = &entity->getComponent<TransformComponent>();
            sprite = &entity->getComponent<SpriteComponent>();
            lastShotTime = 0;
        }

        void update() override {
            if(Game::event.type == SDL_KEYDOWN) {
                switch (Game::event.key.keysym.sym)
                {
                case SDLK_w:
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
                    break;

                case SDLK_a:
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
                    break;
                case SDLK_s:
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
                    break;
                case SDLK_d:
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
                    break;
                case SDLK_SPACE:
                    if (SDL_GetTicks() - lastShotTime >= shotCooldown) {
                        sprite->Play(playerDirection);
                        sprite->SetFlip(playerFlip);
                        Game::assets->CreateProjectile(
                            Vector2D(transform->position.x + offsetX, transform->position.y + offsetY),
                            Vector2D(velX, velY),
                            250,
                            1,
                            direction,
                            bulletFlip
                        );
                        lastShotTime = SDL_GetTicks();
                    }
                    break;
                default:
                    break;
                }
            }
            if(Game::event.type == SDL_KEYUP) {
                switch (Game::event.key.keysym.sym)
                {
                case SDLK_w:
                    transform->velocity.y = 0;
                    sprite->Play("Idle");
                    break;
                case SDLK_a:
                    transform->velocity.x = 0;
                    sprite->Play("Idle");
                    sprite->spriteFlip = SDL_FLIP_NONE;
                    break;
                case SDLK_s:
                    transform->velocity.y = 0;
                    sprite->Play("Idle");
                    break;
                case SDLK_d:
                    transform->velocity.x = 0;
                    sprite->Play("Idle");
                    break;
                case SDLK_SPACE:
                    sprite->Play("Idle");
                    break;
                case SDLK_ESCAPE:
                    Game::isRunning = false;
                    break;
                default:
                    break;
                }
            }
        }
};