#ifndef Game_hpp
#define Game_hpp

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include "ECS/ECS.hpp"
#include "Vector2D.hpp"

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
        Vector2D findRandomSpawnPosition();
        Vector2D findRandomCluePosition();
        Vector2D findRandomMagazinePosition();
        Vector2D findRandomHealthPotionPosition();
        Vector2D findRandomEnemyPosition();
        bool running() { return isRunning; }
        
        void showQuestion(Entity* clueEntity);
        void checkAnswer(int selectedAnswer);
        void closeQuestion();

        
        static SDL_Renderer *renderer;
        static SDL_Event event;
        static bool isRunning;
        static SDL_Rect camera;
        static AssetManager* assets;
        static int totalClues;
        static int totalMagazines;
        static int totalHealthPotions;
        static int collectedClues;
        static bool gameOver;
        static bool playerWon;
        static bool questionActive;
        static Entity* pendingClueEntity;
        static bool showFeedback;
        static Uint32 feedbackStartTime;
        static Entity* feedbackLabel;

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
        SDL_Color green = {0, 255, 0, 255};   // Color for correct answers
        SDL_Color red = {255, 0, 0, 255};     // Color for incorrect answers
        
        bool isAnswerCorrect = false;         // Was the last answer correct
        const Uint32 feedbackDuration = 1500; // How long to show feedback (1.5 seconds)
        
        // Track used spawn positions to prevent duplicates
        std::set<Vector2D> usedCluePositions;
        std::set<Vector2D> usedMagazinePositions;
        std::set<Vector2D> usedHealthPotionPositions;
        std::set<Vector2D> usedEnemyPositions;
        
        struct Question {
            std::string question;
            std::string answers[4];
            int correctAnswer;
        };
        
        std::vector<Question> questions;
        int currentQuestion;
        
        Entity* questionLabel = nullptr;
        Entity* answer1Label = nullptr;
        Entity* answer2Label = nullptr;
        Entity* answer3Label = nullptr;
        Entity* answer4Label = nullptr;
        Entity* questionBackground = nullptr;
};
#endif
