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
#include <map>

class ColliderComponent;
class AssetManager;
class Map;

extern Entity* player;
extern Entity* finalBoss;
extern Entity* scientist;

enum GameState {
    STATE_MAIN_MENU,
    STATE_GAME,
    STATE_PAUSE,
    STATE_SETTINGS,
    STATE_GAME_OVER,
    STATE_END_SCREEN,
    STATE_REPLAY,
    STATE_LEADERBOARD
};

enum MenuItem {
    MENU_NEW_GAME,
    MENU_LOAD_GAME,
    MENU_SETTINGS,
    MENU_LEADERBOARD,
    MENU_EXIT,
    MENU_ITEMS_COUNT
};

enum PauseMenuItem {
    PAUSE_RESUME,
    PAUSE_SAVE,
    PAUSE_RESTART,
    PAUSE_SETTINGS,
    PAUSE_MAIN_MENU,
    PAUSE_ITEMS_COUNT
};

enum SettingsMenuItem {
    SETTINGS_VOLUME,
    SETTINGS_BACK,
    SETTINGS_ITEMS_COUNT
};

enum EndScreenOption {
    END_RESTART,
    END_REPLAY,
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
        void replay();
        void initEntities();
        bool running() { return isRunning; }
        
        void showQuestion(Entity* clueEntity);
        void checkAnswer(int selectedAnswer);
        void closeQuestion();

        void initMainMenu();
        void updateMainMenu();
        void renderMainMenu();
        void startGame();
        void loadGame();

        void loadLevel(int levelNum);
        void advanceToNextLevel();
        
        void initEndScreen(bool victory);
        void updateEndScreen();
        void renderEndScreen();

        void initPauseMenu();
        void updatePauseMenu();
        void renderPauseMenu();
        void togglePause();
        void saveGame();
        
        void initSettingsMenu();
        void updateSettingsMenu();
        void renderSettingsMenu();
        void applySettings();

        void initLeaderboard();
        void renderLeaderboard();
        void updateLeaderboard();
        void saveToLeaderboard(const std::string& playerName, Uint32 gameTime);
        void promptPlayerName();

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
        static int currentLevel;
        static int maxLevels;
        static bool showingExitInstructions;
        static GameState gameState;
        static bool level4MapChanged;
        static bool finalBossDefeated;
        static bool bossMusicPlaying;
        static bool scientistRescued;
        static bool canRescueScientist;
        static bool returnToMainMenu;
        static bool hasSavedDuringExitInstructions;
        static std::string savedExitInstructionsText;

        static bool isRecordingPositions;
        static bool isReplaying;
        static int replayPositionIndex;
        static Vector2D lastRecordedPosition;
        static Uint32 replayFrameTime;
        static Uint32 lastReplayFrameTime;

        static Uint32 gameStartTime;
        static Uint32 gameplayTime;
        static Entity* timerLabel;

        std::set<int> usedQuestions;
        int currentQuestion = -1;

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

        static int volumeLevel;
        
        static std::string currentMusic;

        Entity* replayEntity = nullptr;
        std::map<int, std::vector<Vector2D>> allReplayPositionsByLevel;
        int currentReplayLevel = 1;

    private:
        int count = 0;
        Uint32 lastTime = 0;
        SDL_Window* window;
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color green = {0, 255, 0, 255};
        SDL_Color red = {255, 0, 0, 255};
        SDL_Color yellow = {255, 255, 0, 255};
        
        bool isAnswerCorrect = false;
        const Uint32 feedbackDuration = 800;
        
        float hurtSoundTimer = 0.0f;
        const float hurtSoundCooldown = 0.5f;
        
        Entity* menuTitle = nullptr;
        Entity* menuNewGameButton = nullptr;
        Entity* menuLoadGameButton = nullptr;
        Entity* menuSettingsButton = nullptr;
        Entity* menuLeaderboardButton = nullptr;
        Entity* menuExitButton = nullptr;
        Entity* menuBackground = nullptr;
        
        int selectedMenuItem = MENU_NEW_GAME;
        bool menuItemSelected = false;
        bool menuHighlightActive = false;
        
        int selectedEndOption = END_RESTART;
        bool endOptionSelected = false;
        bool endHighlightActive = false;
        
        Entity* endTitle = nullptr;
        Entity* endMessage = nullptr;
        Entity* endRestartButton = nullptr;
        Entity* endReplayButton = nullptr;
        Entity* endMenuButton = nullptr;
        
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

        void resetUsedQuestions() { usedQuestions.clear(); }
        
        void recordPlayerPosition();
        void readAllPositionsFromFile();
        void updateReplay();
        void renderReplay();

        Entity* pauseTitle = nullptr;
        Entity* pauseResumeButton = nullptr;
        Entity* pauseSaveButton = nullptr;
        Entity* pauseRestartButton = nullptr;
        Entity* pauseSettingsButton = nullptr;
        Entity* pauseMainMenuButton = nullptr;
        Entity* pauseBackground = nullptr;
        
        int selectedPauseItem = PAUSE_RESUME;
        bool pauseItemSelected = false;
        bool pauseHighlightActive = false;
        
        Entity* settingsTitle = nullptr;
        Entity* volumeLabel = nullptr;
        Entity* keybindsLabel = nullptr;
        Entity* settingsBackButton = nullptr;
        Entity* settingsBackground = nullptr;
        
        int selectedSettingsItem = SETTINGS_VOLUME;
        bool settingsItemSelected = false;
        bool settingsHighlightActive = false;
        bool draggingVolumeSlider = false;
        
        struct KeybindInfo {
            std::string action;
            std::string key;
        };
        
        std::vector<KeybindInfo> keybinds = {
            {"Move Up", "W"},
            {"Move Down", "S"},
            {"Move Left", "A"},
            {"Move Right", "D"},
            {"Shoot", "Left Mouse"},
            {"Interact/Skip Question", "E"},
            {"Pause", "ESC"}
        };

        GameState previousState = STATE_MAIN_MENU;

        std::string playerName = "";
        bool waitingForPlayerName = false;
        bool shouldSaveToLeaderboard = false;
        std::vector<std::pair<std::string, std::string>> leaderboardEntries;
        
        Entity* leaderboardTitle = nullptr;
        Entity* leaderboardEntryLabels[5] = {nullptr};
        Entity* leaderboardBackButton = nullptr;
};
#endif
