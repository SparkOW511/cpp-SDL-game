#pragma once

#include <string>
#include "SDL2/SDL.h"
#include "Components.hpp"
#include "../TextureManager.hpp"

class ColliderComponent : public Component {
    public:
    
    SDL_Rect collider;
    std::string tag;
    int offsetX = 0;
    int offsetY = 0;

    SDL_Texture* tex;
    SDL_Rect srcR, destR;

    TransformComponent* transform;

    ColliderComponent(std::string t){
        tag = t;
        // Dimensions will be set from transform in init
        collider.w = collider.h = 0;
        offsetX = 0;
        offsetY = 0;
        tex = nullptr;
    }

    ColliderComponent(std::string t, int xpos, int ypos, int size) {
        tag = t;
        collider.x = xpos;
        collider.y = ypos;
        collider.w = size;
        collider.h = size;
        tex = nullptr;
    }

    ColliderComponent(std::string t, int w, int h, int offX, int offY) {
        tag = t;
        collider.w = w; // Use provided width
        collider.h = h; // Use provided height
        offsetX = offX; // Use provided offset X
        offsetY = offY; // Use provided offset Y
        tex = nullptr;
    }

    ~ColliderComponent() {
        if (tex != nullptr) {
            SDL_DestroyTexture(tex);
            tex = nullptr;
        }
    }

    void init() override {
        if(!entity->hasComponent<TransformComponent>()) {
            entity->addComponent<TransformComponent>();
        }
        transform = &entity->getComponent<TransformComponent>();
        
        // If collider dimensions weren't set by a specific constructor, get them from transform
        if (collider.w == 0 && collider.h == 0 && tag != "terrain") {
             collider.w = transform->width * transform->scale;
             collider.h = transform->height * transform->scale;
             // Ensure offsets are 0 if using transform dimensions
             offsetX = 0;
             offsetY = 0;
        }

        // Set initial position based on transform and potentially provided offsets
        if (tag != "terrain") {
            collider.x = static_cast<int>(transform->position.x) + offsetX;
            collider.y = static_cast<int>(transform->position.y) + offsetY;
        }

        tex = TextureManager::LoadTexture("assets/colliderTex.png");
        srcR = { 0, 0, 32, 32};
        destR = { collider.x, collider.y, collider.w, collider.h };
    }

    void update() override {
        if(tag != "terrain") {
            collider.x = static_cast<int>(transform->position.x) + offsetX;
            collider.y = static_cast<int>(transform->position.y) + offsetY;
        }
        
        destR.x = collider.x - Game::camera.x;
        destR.y = collider.y - Game::camera.y;
        destR.w = collider.w;
        destR.h = collider.h;
    }

    void draw() override {
        // Make colliders visible for debugging
        // TextureManager::Draw(tex, srcR, destR, SDL_FLIP_NONE);
    }
};