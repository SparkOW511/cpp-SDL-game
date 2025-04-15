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
#include "RandomPositionManager.hpp"
#include "TransitionManager.hpp"

class ColliderComponent;
class AssetManager;
class Map;

// Extern declarations for global entities
extern Entity* player;
extern Entity* finalBoss; // Final boss entity for level 4
extern Entity* scientist; // Scientist NPC for level 4

// Game state enumeration 
enum GameState {
    STATE_MAIN_MENU,
    STATE_GAME,
    STATE_PAUSE,
    STATE_GAME_OVER,
    STATE_END_SCREEN
};

// Menu item enumeration
enum MenuItem {
    MENU_NEW_GAME,
    MENU_LOAD_GAME,
    MENU_EXIT,
    MENU_ITEMS_COUNT
};

// End screen options
enum EndScreenOption {
    END_RESTART,
    END_MAIN_MENU,
    END_OPTIONS_COUNT
};

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
        
        void showQuestion(Entity* clueEntity);
        void checkAnswer(int selectedAnswer);
        void closeQuestion();

        // Main menu methods
        void initMainMenu();
        void updateMainMenu();
        void renderMainMenu();
        void startGame();
        void loadGame(); // Not implemented functionality

        // Level management methods
        void loadLevel(int levelNum);
        void advanceToNextLevel();
        
        // End screen methods
        void initEndScreen(bool victory);
        void updateEndScreen();
        void renderEndScreen();

        static SDL_Renderer *renderer;
        static SDL_Event event;
        static bool isRunning;
        static SDL_Rect camera;
        static AssetManager* assets;
        static int totalClues;         // Now set per level in loadLevel method
        static int totalMagazines;     // Now set per level in loadLevel method
        static int totalHealthPotions; // Now set per level in loadLevel method
        static int collectedClues;
        static bool gameOver;
        static bool playerWon;
        static bool questionActive;
        static Entity* pendingClueEntity;
        static bool showFeedback;
        static Uint32 feedbackStartTime;
        static Entity* feedbackLabel;
        static int currentLevel;
        static int maxLevels;
        static bool showingExitInstructions; // Tracks if exit instructions have been shown
        static GameState gameState;          // Current game state
        static bool level4MapChanged;        // Tracks if Level 4 map has been changed
        static bool finalBossDefeated;       // Tracks if the final boss has been defeated
        static bool scientistRescued;        // Tracks if the scientist has been rescued
        static bool canRescueScientist;      // Tracks if player can interact with scientist
        static bool needsRestart;            // Flag to indicate game needs restart
        static bool returnToMainMenu;        // Flag to indicate return to main menu

        // Timer related methods and variables
        static Uint32 gameStartTime;
        static Uint32 gameplayTime;
        static Entity* timerLabel;

        // Track which questions have been used
        std::set<int> usedQuestions;
        int currentQuestion = -1; // Track the currently selected question

        enum groupLabels : std::size_t {
            groupMap,
            groupPlayers,
            groupEnemies,
            groupColliders,
            groupProjectiles,
            groupObjects,
            groupUI,
            groupNPCs
        };

    private:
        int count = 0;
        Uint32 lastTime = 0;
        SDL_Window* window;
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color green = {0, 255, 0, 255};   // Color for correct answers
        SDL_Color red = {255, 0, 0, 255};     // Color for incorrect answers
        SDL_Color yellow = {255, 255, 0, 255}; // Color for selected menu items
        
        bool isAnswerCorrect = false;         // Was the last answer correct
        const Uint32 feedbackDuration = 1500; // How long to show feedback (1.5 seconds)
        
        // Main menu entities
        Entity* menuTitle = nullptr;
        Entity* menuNewGameButton = nullptr;
        Entity* menuLoadGameButton = nullptr;  
        Entity* menuExitButton = nullptr;
        Entity* menuBackground = nullptr;
        
        // Menu state tracking
        int selectedMenuItem = MENU_NEW_GAME; // Default to New Game
        bool menuItemSelected = false;
        bool menuHighlightActive = false;     // Flag to track if menu highlights should be active
        
        // End Screen state tracking
        int selectedEndOption = END_RESTART;  // Default to restart
        bool endOptionSelected = false;
        bool endHighlightActive = false;      // Track if highlights should be active
        
        // Entities for end screen
        Entity* endTitle = nullptr;
        Entity* endMessage = nullptr;
        Entity* endRestartButton = nullptr;
        Entity* endMenuButton = nullptr;
        
        // Managers
        RandomPositionManager positionManager;
        TransitionManager transitionManager;
        
        struct Question {
            std::string question;
            std::string answers[4];
            int correctAnswer;
        };
        
        std::vector<Question> questions;
        
        Entity* questionLabel = nullptr;
        Entity* answer1Label = nullptr;
        Entity* answer2Label = nullptr;
        Entity* answer3Label = nullptr;
        Entity* answer4Label = nullptr;
        Entity* questionBackground = nullptr;
        Entity* transitionLabel = nullptr;

        // Reset question tracking when restarting or changing levels
        void resetUsedQuestions() { usedQuestions.clear(); }
};
#endif
