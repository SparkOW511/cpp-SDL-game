#ifndef Game_hpp
#define Game_hpp

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <iostream>
#include <vector>
#include "ECS/ECS.hpp"

class ColliderComponent;
class AssetManager;
class Map;

class Game {

    public:
        Game();
        ~Game();

        void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
        void handleEvents();
        void update();
        void render();
        void clean();
        void restart();
        void initEntities();
        bool running() { return isRunning; }

        
        static SDL_Renderer *renderer;
        static SDL_Event event;
        static bool isRunning;
        static SDL_Rect camera;
        static AssetManager* assets;
        static int totalClues;
        static int collectedClues;
        static bool gameOver;
        static bool playerWon;

        enum groupLabels : std::size_t {
            groupMap,
            groupPlayers,
            groupEnemies,
            groupColliders,
            groupProjectiles,
            groupObjects,
            groupUI
        };

    private:
        int count = 0;
        Uint32 lastTime = 0;
        SDL_Window* window;
        SDL_Color white = {255, 255, 255, 255};
};
#endif
