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
        collider.w = w;
        collider.h = h;
        offsetX = offX;
        offsetY = offY;
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
        
        if (collider.w == 0 && collider.h == 0 && tag != "terrain") {
             collider.w = transform->width * transform->scale;
             collider.h = transform->height * transform->scale;
             offsetX = 0;
             offsetY = 0;
        }

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
        // TextureManager::Draw(tex, srcR, destR, SDL_FLIP_NONE);
    }
};