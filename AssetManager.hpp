#pragma once

#include <map>
#include <string>
#include "TextureManager.hpp"
#include "Vector2D.hpp"
#include "ECS/ECS.hpp"
#include "src/include/SDL2/SDL_ttf.h"

class AssetManager {
    public:
        AssetManager(Manager* man);
        ~AssetManager();

        void CreateProjectile(Vector2D pos, Vector2D vel, int range, int speed, std::string id, SDL_RendererFlip bulletFlip);

        void CreateObject(int posX, int posY, std::string id);
        //Texture
        void AddTexture(std::string id, const char* path);
        SDL_Texture* GetTexture(std::string id);

        void AddFont(std::string id, std::string path, int fontSize);
        TTF_Font* GetFont(std::string id);
    
    private:

        Manager* manager;
        std::map<std::string, SDL_Texture*> textures;
        std::map<std::string, TTF_Font*> fonts;
};