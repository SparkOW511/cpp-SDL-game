#pragma once

#include "Components.hpp"
#include "SDL2/SDL.h"
#include "../TextureManager.hpp"
#include "../AssetManager.hpp"
#include "Animation.hpp"
#include <map>

class SpriteComponent : public Component {
    private:
        TransformComponent *transform;
        SDL_Texture *texture;
        SDL_Rect srcRect, destRect;
        std::string textureID;

        bool animated = false;
        int frames = 0;
        int speed = 100;
        
        // Alpha value for opacity (0-255)
        Uint8 alpha = 255;
        
        // Flag to determine if this is a clue (for proximity-based opacity)
        bool isClue = false;

    public:

        int animIndex = 0;
        std::map<const char*, Animation> animations;

        SDL_RendererFlip spriteFlip = SDL_FLIP_NONE;

        SpriteComponent() = default;
        SpriteComponent(std::string id) {
            textureID = id;
            setTex(id);
            
            // Set clue flag if this is a clue
            if (id == "clue") {
                isClue = true;
                alpha = 0; // Start fully transparent
            }
        }

        SpriteComponent(std::string id, bool isAnimated) {
            textureID = id;
            animated = isAnimated;

            Animation idle = Animation(0, 3, 100);
            Animation walk = Animation(1, 8, 100);
            Animation walkDown = Animation(2, 5, 100);
            Animation walkUp = Animation(3, 5, 100);

            Animation shoot = Animation(4, 4, 100);
            Animation shootDown = Animation(5, 4, 100);
            Animation shootUp = Animation(6, 4, 100);

            Animation clue = Animation(0, 7, 100);
            Animation magazine = Animation(0, 7, 100);
            Animation healthpotion = Animation(0, 8, 150);

            animations.emplace("Idle", idle);
            animations.emplace("Walk", walk);
            animations.emplace("WalkDown", walkDown);
            animations.emplace("WalkUp", walkUp);
            animations.emplace("Shoot", shoot);
            animations.emplace("ShootDown", shootDown);
            animations.emplace("ShootUp", shootUp);
            animations.emplace("Clue", clue);
            animations.emplace("Magazine", magazine);
            animations.emplace("Healthpotion", healthpotion);
            
            Play("Idle");

            setTex(id);
            
            // Set clue flag if this is a clue
            if (id == "clue") {
                isClue = true;
                alpha = 0; // Start fully transparent
                if (animated) {
                    Play("Clue");
                }
            }
        }
        ~SpriteComponent() {
            
        }
        void setTex(std::string id) {
            textureID = id;
            texture = Game::assets->GetTexture(id);
        }

        std::string getTexID() const {
            return textureID;
        }
        
        void setAlpha(Uint8 a) {
            alpha = a;
        }

        void init() override {

            transform = &entity->getComponent<TransformComponent>();

            srcRect.x = srcRect.y = 0;
            srcRect.w = transform->width;
            srcRect.h = transform->height;
        }

        void update() override {
            if(animated) {
                srcRect.x = srcRect.w * static_cast<int>((SDL_GetTicks() / speed) % frames);
            }

            srcRect.y = animIndex * transform->height;

            destRect.x = static_cast<int>(transform->position.x) - Game::camera.x;
            destRect.y = static_cast<int>(transform->position.y) - Game::camera.y;
            destRect.w = transform->width * transform->scale;
            destRect.h = transform->height * transform->scale;
            
            // Update alpha for clues based on player proximity if player exists
            if (isClue && player != nullptr) {
                // Calculate distance between player and clue
                Vector2D playerPos = player->getComponent<TransformComponent>().position;
                Vector2D cluePos = transform->position;
                
                // Calculate Euclidean distance
                float distance = sqrt(
                    pow(playerPos.x - cluePos.x, 2) + 
                    pow(playerPos.y - cluePos.y, 2)
                );
                
                // Define maximum and minimum distances for visibility
                const float MAX_VISIBLE_DISTANCE = 1000.0f; // Fully invisible beyond this distance
                const float MIN_VISIBLE_DISTANCE = 200.0f; // Fully visible at this distance
                
                // Calculate alpha based on distance (inverse relationship)
                if (distance >= MAX_VISIBLE_DISTANCE) {
                    alpha = 0; // Fully transparent if too far
                } else if (distance <= MIN_VISIBLE_DISTANCE) {
                    alpha = 255; // Fully opaque if close enough
                } else {
                    // Linear interpolation between 0 and 255 based on distance
                    alpha = static_cast<Uint8>(255 * (1.0f - (distance - MIN_VISIBLE_DISTANCE) / (MAX_VISIBLE_DISTANCE - MIN_VISIBLE_DISTANCE)));
                }
            }
        }

        void draw() override {
            if (isClue) {
                TextureManager::Draw(texture, srcRect, destRect, spriteFlip, alpha);
            } else {
                TextureManager::Draw(texture, srcRect, destRect, spriteFlip);
            }
        }

        void Play(const char* animName) {
            frames = animations[animName].frames;
            animIndex = animations[animName].index;
            speed = animations[animName].speed;
         }

         void SetFlip(SDL_RendererFlip flip) {
             spriteFlip = flip;
         }
};
