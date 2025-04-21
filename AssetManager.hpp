#pragma once

#include <map>
#include <string>
#include "TextureManager.hpp"
#include "Vector2D.hpp"
#include "ECS/ECS.hpp"
#include "src/include/SDL2/SDL_ttf.h"
#include "src/include/SDL2/SDL_mixer.h"

class AssetManager {
    public:
        AssetManager(Manager* man);
        ~AssetManager();

        void CreateProjectile(Vector2D pos, Vector2D vel, int range, int speed, std::string id, SDL_RendererFlip bulletFlip);

        void CreateObject(int x, int y, std::string id);
        //Texture
        void AddTexture(std::string id, const char* path);
        SDL_Texture* GetTexture(std::string id);

        void AddFont(std::string id, std::string path, int fontSize);
        TTF_Font* GetFont(std::string id);
        
        // Sound functions
        void AddSound(std::string id, const char* path);
        Mix_Chunk* GetSound(std::string id);
        void PlaySound(std::string id, int volume);
        
        // Music functions
        void AddMusic(std::string id, const char* path);
        Mix_Music* GetMusic(std::string id);
        void PlayMusic(std::string id, int volume, int loops = -1);
        void StopMusic();
        void PauseMusic();
        void ResumeMusic();
        
        // Set global volume
        void SetMasterVolume(int volume);
    
    private:

        Manager* manager;
        std::map<std::string, SDL_Texture*> textures;
        std::map<std::string, TTF_Font*> fonts;
        std::map<std::string, Mix_Chunk*> sounds;
        std::map<std::string, Mix_Music*> music;
        std::string currentMusic; // Track which music is currently playing
};