#include "Game.hpp"
#include "TextureManager.hpp"
#include "Map.hpp"
#include "ECS/Components.hpp"
#include "Vector2D.hpp"
#include "Collision.hpp"
#include "AssetManager.hpp"
#include <sstream>
#include <iomanip>
#include <random>
#include <ctime>
#include <fstream>

// Global manager and entities
Map* map;
Manager manager;

// Global entity pointers instead of references
Entity* player = nullptr;
Entity* finalBoss = nullptr; // Final boss entity for level 4
Entity* healthbar = nullptr;
Entity* ammobar = nullptr;
Entity* gameover = nullptr;
Entity* clueCounter = nullptr;
Entity* feedbackLabel = nullptr;
Entity* scientist = nullptr; // Scientist NPC for level 4

// Global entity groups
std::vector<Entity*>* tiles;
std::vector<Entity*>* players;
std::vector<Entity*>* enemies;
std::vector<Entity*>* colliders;
std::vector<Entity*>* projectiles;
std::vector<Entity*>* objects;
std::vector<Entity*>* ui;

// Additional question variable to track if answer was submitted
bool answerSubmitted = false;

// Collision variables
float damageTimer = 1.0f;
const float damageCooldown = 0.3f;
float objectCollisionDelay = 1.0f;
bool objectCollisionsEnabled = false;

SDL_Renderer *Game::renderer = nullptr;
SDL_Event Game::event;
SDL_Rect Game::camera = {0, 0, 1920, 1080};
bool Game::isRunning = false;
AssetManager* Game::assets = nullptr;
int Game::totalClues = 3; // Total number of clues to collect
int Game::totalMagazines = 3; // Total number of magazines to collect
int Game::totalHealthPotions = 2; // Total number of health potions to collect
int Game::collectedClues = 0;
bool Game::gameOver = false;
bool Game::playerWon = false;
bool Game::questionActive = false;
Entity* Game::pendingClueEntity = nullptr;
bool Game::showFeedback = false;
Uint32 Game::feedbackStartTime = 0;
Entity* Game::feedbackLabel = nullptr;
int Game::currentLevel = 1;
int Game::maxLevels = 4; // Set the maximum number of levels here
bool Game::showingExitInstructions = false; // Initialize to false
GameState Game::gameState = STATE_MAIN_MENU; // Initialize to main menu
bool Game::level4MapChanged = false;
bool Game::finalBossDefeated = false;
bool Game::scientistRescued = false;
bool Game::canRescueScientist = false;
bool Game::needsRestart = false;
bool Game::returnToMainMenu = false;
Uint32 Game::gameStartTime = 0;
Uint32 Game::gameplayTime = 0;
Entity* Game::timerLabel = nullptr;
int Game::volumeLevel = 90; // Default volume level to 90%

// Replay related static variables
bool Game::isRecordingPositions = false;
bool Game::isReplaying = false;
int Game::replayPositionIndex = 0;
Vector2D Game::lastRecordedPosition = Vector2D(0, 0);
Uint32 Game::replayFrameTime = 20; // Target ~60 FPS playback (1000ms / 60)
Uint32 Game::lastReplayFrameTime = 0;

// Main menu variables
Entity* menuTitle = nullptr;
Entity* menuNewGameButton = nullptr;
Entity* menuLoadGameButton = nullptr;
Entity* menuSettingsButton = nullptr;  // Add settings button
Entity* menuLeaderboardButton = nullptr;  // Add leaderboard button
Entity* menuExitButton = nullptr;
int selectedMenuItem = MENU_NEW_GAME;
bool menuHighlightActive = false;
bool menuItemSelected = false;

// End screen variables
Entity* endTitle = nullptr;
Entity* endMessage = nullptr;
Entity* endRestartButton = nullptr;
Entity* endReplayButton = nullptr;  // Add replay button
Entity* endMenuButton = nullptr;
int selectedEndOption = END_RESTART;
bool endOptionSelected = false;
bool endHighlightActive = false;

// Pause menu variables
Entity* pauseTitle = nullptr;
Entity* pauseResumeButton = nullptr;
Entity* pauseSaveButton = nullptr;
Entity* pauseRestartButton = nullptr;
Entity* pauseSettingsButton = nullptr;
Entity* pauseMainMenuButton = nullptr;
Entity* pauseBackground = nullptr;
int selectedPauseItem = PAUSE_RESUME;
bool pauseHighlightActive = false;
bool pauseItemSelected = false;

// Settings menu variables
Entity* settingsTitle = nullptr;
Entity* volumeSlider = nullptr;
Entity* volumeLabel = nullptr;
Entity* keybindsLabel = nullptr;
Entity* settingsBackButton = nullptr;
Entity* settingsBackground = nullptr;
int selectedSettingsItem = SETTINGS_VOLUME;
bool settingsHighlightActive = false;
bool settingsItemSelected = false;
bool draggingVolumeSlider = false;

Game::Game()
{
    assets = new AssetManager(&manager);
    
    questions = {
        {"What year was the first video game created?", {"1958", "1972", "1985", "1962"}, 0},
        {"Which of these is NOT a programming language?", {"Java", "Python", "COBOL", "Panther"}, 3},
        {"What does CPU stand for?", {"Computer Processing Unit", "Central Processing Unit", "Core Processing Unit", "Central Program Unit"}, 1},
        {"What is the name of the Super Mario Bros. princess?", {"Daisy", "Rosalina", "Zelda", "Peach"}, 3},
        {"Which company created Tetris?", {"Nintendo", "Atari", "Electronic Arts", "Elektronorgtechnica"}, 3},
        {"What animal is Sonic the Hedgehog?", {"Fox", "Hedgehog", "Mouse", "Rabbit"}, 1},
        {"What year was the first iPhone released?", {"2005", "2006", "2007", "2008"}, 2},
        {"How many bits make up a byte?", {"4", "8", "16", "32"}, 1},
        {"What does HTML stand for?", {"HyperText Markup Language", "High Technical Modern Language", "HyperTransfer Markup Logic", "High Tech Multi Language"}, 0},
        {"Which of these is NOT a programming paradigm?", {"Object-Oriented", "Functional", "Procedural", "Dialectical"}, 3},
        
        {"What does RAM stand for?", {"Random Access Memory", "Read Access Memory", "Rapid Access Memory", "Runtime Access Memory"}, 0},
        {"Which data structure follows LIFO (Last In First Out) principle?", {"Queue", "Stack", "Linked List", "Graph"}, 1},
        {"What is the time complexity of a linear search algorithm?", {"O(1)", "O(log n)", "O(n)", "O(n^2)"}, 2},
        {"Which programming language is primarily used for iOS app development?", {"Java", "C#", "Swift", "Python"}, 2},
        {"What does SQL stand for?", {"Structured Query Language", "Simple Query Language", "Standard Query Logic", "System Query Loop"}, 0},
        {"Who is considered the father of computer science?", {"Steve Jobs", "Alan Turing", "Bill Gates", "Tim Berners-Lee"}, 1},
        {"Which of these is NOT a version control system?", {"Git", "Mercurial", "SVN", "Django"}, 3},
        {"What is the name of the first widely-used programming language?", {"FORTRAN", "COBOL", "C", "BASIC"}, 0},
        {"In binary, what is 1010 in decimal?", {"2", "8", "10", "12"}, 2},
        {"Which sorting algorithm has an average time complexity of O(n log n)?", {"Bubble Sort", "Quick Sort", "Selection Sort", "Insertion Sort"}, 1},
        
        {"Which console was released first?", {"PlayStation", "Nintendo 64", "Xbox", "Sega Genesis"}, 3},
        {"What programming language was Minecraft originally written in?", {"C++", "Java", "Python", "C#"}, 1},
        {"Who is the creator of Linux?", {"Steve Wozniak", "Linus Torvalds", "Bill Gates", "Mark Zuckerberg"}, 1},
        {"Which video game franchise features a protagonist named Master Chief?", {"Call of Duty", "Halo", "Gears of War", "Destiny"}, 1},
        {"What was the first popular web browser?", {"Internet Explorer", "Firefox", "Netscape Navigator", "Chrome"}, 2},
        {"Which company developed the game Fortnite?", {"Valve", "Epic Games", "Blizzard", "EA"}, 1},
        {"What does GPU stand for?", {"General Processing Unit", "Graphics Processing Unit", "Gaming Performance Utility", "Global Processing Unit"}, 1},
        {"Which of these is NOT one of the original three starter Pokemon?", {"Bulbasaur", "Charmander", "Pikachu", "Squirtle"}, 2},
        {"Which company created the first commercially successful mouse?", {"Microsoft", "Apple", "IBM", "Logitech"}, 1},
        
        {"What is the result of 1 + 1 in binary?", {"0", "1", "10", "11"}, 2},
        {"What is the value of pi rounded to two decimal places?", {"3.41", "3.14", "3.50", "3.16"}, 1},
        {"In Boolean algebra, what is the result of TRUE AND FALSE?", {"TRUE", "FALSE", "NULL", "ERROR"}, 1},
        {"What is the solution to x in the equation 2x + 5 = 15?", {"5", "10", "7.5", "20"}, 0},
        {"What is the square root of 144?", {"10", "12", "14", "16"}, 1},
        {"Which of these is NOT a prime number?", {"13", "17", "21", "23"}, 2},
        {"What is the perimeter of a square with sides of length 6?", {"12", "18", "24", "36"}, 2},
        {"What is the next number in the sequence: 2, 4, 8, 16, ...?", {"24", "30", "32", "42"}, 2},
        {"What is the area of a circle with radius 5?", {"25pi", "10pi", "15pi", "20pi"}, 0},
        {"What does the ^ operator typically represent in programming?", {"Multiplication", "Division", "Exponentiation", "Bitwise XOR"}, 3}
    };
}

Game::~Game()
{
    delete assets;
}

void Game::initEntities() {
    // Create entities
    player = &manager.addEntity();
    healthbar = &manager.addEntity();
    ammobar = &manager.addEntity();
    gameover = &manager.addEntity();
    clueCounter = &manager.addEntity();
    feedbackLabel = &manager.addEntity();
    transitionLabel = &manager.addEntity();
    timerLabel = &manager.addEntity();
    
    questionLabel = &manager.addEntity();
    answer1Label = &manager.addEntity();
    answer2Label = &manager.addEntity();
    answer3Label = &manager.addEntity();
    answer4Label = &manager.addEntity();
    questionBackground = &manager.addEntity();
    
    // Get group references
    tiles = &manager.getGroup(Game::groupMap);
    players = &manager.getGroup(Game::groupPlayers);
    enemies = &manager.getGroup(Game::groupEnemies);
    colliders = &manager.getGroup(Game::groupColliders);
    projectiles = &manager.getGroup(Game::groupProjectiles);
    objects = &manager.getGroup(Game::groupObjects);
    ui = &manager.getGroup(Game::groupUI);
    
    // Create clues at random locations
    for (int i = 0; i < totalClues; i++) {
        Vector2D cluePos = positionManager.findRandomCluePosition(currentLevel);
        assets->CreateObject(cluePos.x, cluePos.y, "clue");
    }
    
    // Create magazine pickups at random locations using magazine-specific function
    for (int i = 0; i < totalMagazines; i++) {
        Vector2D magazinePos = positionManager.findRandomMagazinePosition();
        assets->CreateObject(magazinePos.x, magazinePos.y, "magazine");
    }
    
    // Create health potion pickups at random locations using potion-specific function
    for (int i = 0; i < totalHealthPotions; i++) {
        Vector2D potionPos = positionManager.findRandomHealthPotionPosition();
        assets->CreateObject(potionPos.x, potionPos.y, "healthpotion");
    }
    
    // Find a random valid spawn position for the player
    Vector2D playerSpawnPos = positionManager.findRandomSpawnPosition(currentLevel);
    
    // Setup player entity with random position
    player->addComponent<TransformComponent>(playerSpawnPos.x, playerSpawnPos.y, 32, 32, 3);
    player->addComponent<SpriteComponent>("player", true);
    // Use the specific constructor for player collider dimensions and offset (scaled)
    player->addComponent<ColliderComponent>("player", 21 * 3, 29 * 3, 6 * 3, 4 * 3);
    player->addComponent<HealthComponent>(100);
    player->addComponent<AmmoComponent>(30, 10);
    player->addComponent<KeyboardController>();
    player->addGroup(Game::groupPlayers);

    // Create multiple enemies at random positions - number based on level
    int numEnemies = 3; // Default for level 1
    if (currentLevel == 2) {
        numEnemies = 8; // More enemies in level 2
    } else if (currentLevel == 3) {
        numEnemies = 13; // Even more enemies in level 3
    } else if (currentLevel == 4) {
        numEnemies = 8; // Fewer enemies in level 4 to compensate for the boss
    }
    
    // Create the final boss for level 4
    if (currentLevel == 4) {
        finalBoss = &manager.addEntity();
        
        // Use the exact coordinates from the map (position 4 at 34,15)
        Vector2D bossPos = {34*64, 15*64};
        
        // Make the boss larger and stronger than regular enemies
        // The sprite size is 32x32 but we use a larger scale and collider
        finalBoss->addComponent<TransformComponent>(bossPos.x, bossPos.y, 32, 32, 4); // Use 32x32 at scale 4
        finalBoss->addComponent<SpriteComponent>("boss", true); // Use boss sprite
        finalBoss->addComponent<ColliderComponent>("boss");
        finalBoss->addComponent<HealthComponent>(500); // Much more health
        finalBoss->addComponent<EnemyAIComponent>(manager);
        finalBoss->getComponent<EnemyAIComponent>().setSpeed(0.5f); // Slower but stronger
        finalBoss->addGroup(Game::groupEnemies);
        
        // Create the scientist at position (35,5)
        scientist = &manager.addEntity();
        Vector2D scientistPos = {34*64, 3*64};
        scientist->addComponent<TransformComponent>(scientistPos.x, scientistPos.y, 32, 32, 3);
        scientist->addComponent<SpriteComponent>("scientist", true);
        scientist->addComponent<ColliderComponent>("scientist");
        scientist->getComponent<SpriteComponent>().Play("Locked"); // Use regular Idle animation since we don't have specific scientist animations
        scientist->addGroup(Game::groupObjects); // Add to objects group so it's visible
        
    } else {
        finalBoss = nullptr; // Clear final boss pointer for other levels
        scientist = nullptr; // Clear scientist pointer for other levels
    }
    
    for (int i = 0; i < numEnemies; i++) {
        Entity& enemy = manager.addEntity();
        
        Vector2D enemyPos = positionManager.findRandomEnemyPosition();
        enemy.addComponent<TransformComponent>(enemyPos.x, enemyPos.y, 32, 32, 3);
        enemy.addComponent<SpriteComponent>("enemy", true);
        enemy.addComponent<ColliderComponent>("enemy");
        enemy.addComponent<HealthComponent>(100);
        enemy.addComponent<EnemyAIComponent>(manager);
        enemy.addGroup(Game::groupEnemies);
    }

    // Setup UI entities with improved positioning for larger fonts
    healthbar->addComponent<UILabel>(20, 20, "Test", "font1", white);
    ammobar->addComponent<UILabel>(20, 60, "Test", "font1", white);
    gameover->addComponent<UILabel>(0, 0, "", "font2", white);
    clueCounter->addComponent<UILabel>(20, 100, "Clues: 0/" + std::to_string(totalClues), "font1", white);
    timerLabel->addComponent<UILabel>((1920/2) - 100, 20, "Time: 00:00", "font1", white);
    
    questionLabel->addComponent<UILabel>(0, 300, "", "font1", white);
    answer1Label->addComponent<UILabel>(0, 340, "", "font1", white);
    answer2Label->addComponent<UILabel>(0, 380, "", "font1", white);
    answer3Label->addComponent<UILabel>(0, 420, "", "font1", white);
    answer4Label->addComponent<UILabel>(0, 460, "", "font1", white);
    feedbackLabel->addComponent<UILabel>(0, 650, "", "font2", white);
    transitionLabel->addComponent<UILabel>(0, 0, "", "font2", white);
    
    // No need to manually add UI entities to groupUI as the UILabel component does this automatically
}

void Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen)
{
    int flags = 0;
    if(fullscreen) {
        flags = SDL_WINDOW_FULLSCREEN;
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
        window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
        renderer = SDL_CreateRenderer(window, -1, 0);

        if(renderer) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }

        isRunning = true;
    }

    if(TTF_Init() == -1) {
        std::cout << "Error initializing SDL_ttf" << std::endl;
    }

    // Clean up previous asset manager if it exists
    if(assets != nullptr) {
        delete assets;
    }

    assets = new AssetManager(&manager);

    // Load textures
    assets->AddTexture("terrainlvl1", "./assets/lvl1/TerrainTexturesLevel1.png");
    assets->AddTexture("terrainlvl2", "./assets/lvl2/TerrainTexturesLevel2.png");
    assets->AddTexture("terrainlvl3", "./assets/lvl3/TerrainTexturesLevel3.png");
    assets->AddTexture("terrainlvl4", "./assets/lvl4/TerrainTexturesLevel4.png");

    assets->AddTexture("player", "./assets/entities/playeranimations.png");
    assets->AddTexture("enemy", "./assets/entities/enemyanimations.png");
    assets->AddTexture("boss", "./assets/entities/finalbossanimations.png");
    assets->AddTexture("scientist", "./assets/entities/scientistanimations.png");

    assets->AddTexture("clue", "./assets/objects/clue.png");
    assets->AddTexture("magazine", "./assets/objects/magazine.png");
    assets->AddTexture("bulletHorizontal", "./assets/projectiles/bulletHorizontal.png");
    assets->AddTexture("bulletVertical", "./assets/projectiles/bulletVertical.png");
    assets->AddTexture("healthpotion", "./assets/objects/healthpotion.png");
    assets->AddTexture("cactus", "./assets/objects/cactus.png");
    
    // Load fonts
    assets->AddFont("font1", "./assets/MINECRAFT.TTF", 32);
    assets->AddFont("font2", "./assets/MINECRAFT.TTF", 72);

    // Initialize random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    // Initialize transition manager globally
    transitionManager.init(this, &manager);
    
    // Create transition label for transitions between levels
    transitionLabel = &manager.addEntity();
    transitionLabel->addComponent<UILabel>(0, 0, "", "font2", white);
    transitionManager.mTransitionLabel = transitionLabel;
    
    // Set initial game state
    gameState = STATE_MAIN_MENU;
    
    // Initialize the appropriate game elements based on state
    if (gameState == STATE_MAIN_MENU) {
        initMainMenu();
    } else {
        // Initialize the game entities and map for gameplay
    initEntities();
        
        // Load first level
        loadLevel(currentLevel);
    }
}

void Game::handleEvents()
{
    SDL_PollEvent(&event);
    
    switch(event.type) {
        case SDL_QUIT:
            isRunning = false;
            break;
            
        case SDL_KEYDOWN:
            // Handle replay mode ESC key to exit
            if (gameState == STATE_REPLAY && event.key.keysym.sym == SDLK_ESCAPE) {
                // Stop replay and return to main menu
                isReplaying = false;
                returnToMainMenu = true;
                break;
            }
            
            if (gameState == STATE_MAIN_MENU) {
                // Handle menu navigation
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                        selectedMenuItem = (selectedMenuItem - 1 + MENU_ITEMS_COUNT) % MENU_ITEMS_COUNT;
                        menuHighlightActive = true; // Activate highlighting when using keyboard
                        // Force update menu items and reset all hover states
                        updateMainMenu();
                        break;
                    case SDLK_DOWN:
                        selectedMenuItem = (selectedMenuItem + 1) % MENU_ITEMS_COUNT;
                        menuHighlightActive = true; // Activate highlighting when using keyboard
                        // Force update menu items and reset all hover states
                        updateMainMenu();
                        break;
                    case SDLK_RETURN:
                    case SDLK_SPACE:
                        // Handle menu selection
                        switch(selectedMenuItem) {
                            case MENU_NEW_GAME:
                                startGame();
                                break;
                            case MENU_LOAD_GAME:
                                loadGame();
                                break;
                            case MENU_SETTINGS:
                                gameState = STATE_SETTINGS;
                                initSettingsMenu();
                                break;
                            case MENU_LEADERBOARD:
                                // Leaderboard function currently does nothing
                                // Will be implemented later
                                break;
                            case MENU_EXIT:
                                isRunning = false;
                                break;
                        }
                        break;
                    case SDLK_ESCAPE:
                        isRunning = false;
                        break;
                }
            }
            else if (gameOver) {
                if (event.key.keysym.sym == SDLK_r) {
                    // Restart the game
                    restart();
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    // Exit the game
                    isRunning = false;
                }
            }
            else if (questionActive) {
                switch(event.key.keysym.sym) {
                    case SDLK_1:
                    case SDLK_KP_1:
                        checkAnswer(0);
                        break;
                    case SDLK_2:
                    case SDLK_KP_2:
                        checkAnswer(1);
                        break;
                    case SDLK_3:
                    case SDLK_KP_3:
                        checkAnswer(2);
                        break;
                    case SDLK_4:
                    case SDLK_KP_4:
                        checkAnswer(3);
                        break;
                    case SDLK_e:
                        closeQuestion();
                        break;
                    default:
                        break;
                }
            }
            else if (gameState == STATE_GAME) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    // Toggle pause menu instead of returning to main menu
                    togglePause();
                }
                // Handle 'E' key press for scientist interaction
                else if (event.key.keysym.sym == SDLK_e && currentLevel == 4 && canRescueScientist && !scientistRescued && scientist != nullptr) {
                    // Check if player is close enough to scientist
                    Vector2D playerPos = player->getComponent<TransformComponent>().position;
                    Vector2D scientistPos = scientist->getComponent<TransformComponent>().position;
                    float distance = sqrt(pow(playerPos.x - scientistPos.x, 2) + pow(playerPos.y - scientistPos.y, 2));
                    
                    if (distance <= 100) { // Within interaction range
                        // Change scientist animation to idle
                        scientist->getComponent<SpriteComponent>().Play("Idle");
                        scientistRescued = true;
                        
                        // Initialize the end screen with victory
                        initEndScreen(true);
                    }
                }
            }
            else if (gameState == STATE_PAUSE) {
                // Handle navigation in pause menu
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                        selectedPauseItem = (selectedPauseItem - 1 + PAUSE_ITEMS_COUNT) % PAUSE_ITEMS_COUNT;
                        pauseHighlightActive = true;
                        updatePauseMenu();
                        break;
                    case SDLK_DOWN:
                        selectedPauseItem = (selectedPauseItem + 1) % PAUSE_ITEMS_COUNT;
                        pauseHighlightActive = true;
                        updatePauseMenu();
                        break;
                    case SDLK_RETURN:
                    case SDLK_SPACE:
                        // Handle selection
                        switch(selectedPauseItem) {
                            case PAUSE_RESUME:
                                togglePause(); // Resume game
                                break;
                            case PAUSE_SAVE:
                                saveGame(); // Save game (stub)
                                break;
                            case PAUSE_RESTART:
                                // Set restart flag and unpause
                                needsRestart = true;
                                gameState = STATE_GAME;
                                break;
                            case PAUSE_SETTINGS:
                                // Go to settings menu
                                gameState = STATE_SETTINGS;
                                initSettingsMenu();
                                break;
                            case PAUSE_MAIN_MENU:
                                // Return to main menu
                                returnToMainMenu = true;
                                break;
                        }
                        break;
                    case SDLK_ESCAPE:
                        // Resume game when pressing ESC in pause menu
                        togglePause();
                        break;
                }
            }
            else if (gameState == STATE_SETTINGS) {
                // Handle navigation in settings menu
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_DOWN:
                        // Toggle between volume and back button
                        selectedSettingsItem = (selectedSettingsItem == SETTINGS_VOLUME) ? 
                                              SETTINGS_BACK : SETTINGS_VOLUME;
                        settingsHighlightActive = true;
                        updateSettingsMenu();
                        break;
                    case SDLK_LEFT:
                        if (selectedSettingsItem == SETTINGS_VOLUME) {
                            // Decrease volume
                            volumeLevel = std::max(0, volumeLevel - 5);
                            updateSettingsMenu();
                        }
                        break;
                    case SDLK_RIGHT:
                        if (selectedSettingsItem == SETTINGS_VOLUME) {
                            // Increase volume
                            volumeLevel = std::min(100, volumeLevel + 5);
                            updateSettingsMenu();
                        }
                        break;
                    case SDLK_RETURN:
                    case SDLK_SPACE:
                        if (selectedSettingsItem == SETTINGS_BACK) {
                            // Return to previous state (main menu or pause)
                            gameState = previousState;
                            applySettings();
                        }
                        break;
                    case SDLK_ESCAPE:
                        // Return to previous state
                        gameState = previousState;
                        applySettings();
                        break;
                }
            }
            else if (gameState == STATE_END_SCREEN) {
                // Handle navigation in end screen
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_DOWN:
                        // Toggle between the two options
                        selectedEndOption = (selectedEndOption == END_RESTART) ? END_MAIN_MENU : END_RESTART;
                        endHighlightActive = true;
                        updateEndScreen();
                        break;
                    case SDLK_RETURN:
                    case SDLK_SPACE:
                        // Handle selection directly
                        if (selectedEndOption == END_RESTART) {
                            // Set variables for restart
                            gameOver = false;
                            playerWon = false;
                            collectedClues = 0;
                            damageTimer = 1.0f;
                            objectCollisionDelay = 1.0f;
                            objectCollisionsEnabled = false;
                            questionActive = false;
                            pendingClueEntity = nullptr;
                            showFeedback = false;
                            showingExitInstructions = false; // Reset exit instructions flag
                            level4MapChanged = false; // Reset level 4 map state
                            finalBossDefeated = false; // Reset final boss state
                            scientistRescued = false; // Reset scientist state
                            canRescueScientist = false; // Reset scientist interaction flag
                            currentLevel = 1;
                            
                            // Reset used questions
                            resetUsedQuestions();
                            
                            // Set clue count for level 1
                            totalClues = 3;
                            
                            // Reset position tracking
                            positionManager.resetPositions();
                            
                            // Reset gameplay timer
                            gameStartTime = SDL_GetTicks();
                            gameplayTime = 0;
                            
                            // Set flag for deferred restart
                            needsRestart = true;
                        } else if (selectedEndOption == END_MAIN_MENU) {
                            // Set flag for deferred return to main menu
                            returnToMainMenu = true;
                        }
                        break;
                    case SDLK_ESCAPE:
                        // Go to main menu directly
                        returnToMainMenu = true;
                        break;
                }
            }
            break;
            
        // Handle mouse events for clickable UI elements
        case SDL_MOUSEMOTION:
            if (gameState == STATE_MAIN_MENU) {
                // First force reset hover states on all menu items before handling new hover
                bool anyHovered = false;
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
                
                // Handle individual hover events
                if (menuNewGameButton && menuNewGameButton->hasComponent<UILabel>()) {
                    bool isOver = menuNewGameButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedMenuItem = MENU_NEW_GAME;
                        menuHighlightActive = true; // Enable highlighting when hovering
                    }
                }
                
                if (menuLoadGameButton && menuLoadGameButton->hasComponent<UILabel>()) {
                    bool isOver = menuLoadGameButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedMenuItem = MENU_LOAD_GAME;
                        menuHighlightActive = true; // Enable highlighting when hovering
                    }
                }
                
                if (menuSettingsButton && menuSettingsButton->hasComponent<UILabel>()) {
                    bool isOver = menuSettingsButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedMenuItem = MENU_SETTINGS;
                        menuHighlightActive = true; // Enable highlighting when hovering
                    }
                }
                
                if (menuLeaderboardButton && menuLeaderboardButton->hasComponent<UILabel>()) {
                    bool isOver = menuLeaderboardButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedMenuItem = MENU_LEADERBOARD;
                        menuHighlightActive = true; // Enable highlighting when hovering
                    }
                }
                
                if (menuExitButton && menuExitButton->hasComponent<UILabel>()) {
                    bool isOver = menuExitButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedMenuItem = MENU_EXIT;
                        menuHighlightActive = true; // Enable highlighting when hovering
                    }
                }
                
                // If no items are being hovered and we previously had highlights active
                if (!anyHovered && menuHighlightActive) {
                    menuHighlightActive = false; // Disable highlights when not hovering
                    updateMainMenu(); // Update to show no highlights
                } else if (anyHovered) {
                    // Process hover effects
                    if (menuNewGameButton && menuNewGameButton->hasComponent<UILabel>()) {
                        menuNewGameButton->getComponent<UILabel>().HandleEvent(event);
                    }
                    
                    if (menuLoadGameButton && menuLoadGameButton->hasComponent<UILabel>()) {
                        menuLoadGameButton->getComponent<UILabel>().HandleEvent(event);
                    }
                    
                    if (menuSettingsButton && menuSettingsButton->hasComponent<UILabel>()) {
                        menuSettingsButton->getComponent<UILabel>().HandleEvent(event);
                    }
                    
                    if (menuLeaderboardButton && menuLeaderboardButton->hasComponent<UILabel>()) {
                        menuLeaderboardButton->getComponent<UILabel>().HandleEvent(event);
                    }
                    
                    if (menuExitButton && menuExitButton->hasComponent<UILabel>()) {
                        menuExitButton->getComponent<UILabel>().HandleEvent(event);
                    }
                }
            }
            else if (gameState == STATE_PAUSE) {
                // Handle pause menu mouse hover
                bool anyHovered = false;
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
                
                // Check each pause menu button for hover
                if (pauseResumeButton && pauseResumeButton->hasComponent<UILabel>()) {
                    bool isOver = pauseResumeButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedPauseItem = PAUSE_RESUME;
                        pauseHighlightActive = true;
                    }
                }
                
                if (pauseSaveButton && pauseSaveButton->hasComponent<UILabel>()) {
                    bool isOver = pauseSaveButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedPauseItem = PAUSE_SAVE;
                        pauseHighlightActive = true;
                    }
                }
                
                if (pauseRestartButton && pauseRestartButton->hasComponent<UILabel>()) {
                    bool isOver = pauseRestartButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedPauseItem = PAUSE_RESTART;
                        pauseHighlightActive = true;
                    }
                }
                
                if (pauseSettingsButton && pauseSettingsButton->hasComponent<UILabel>()) {
                    bool isOver = pauseSettingsButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedPauseItem = PAUSE_SETTINGS;
                        pauseHighlightActive = true;
                    }
                }
                
                if (pauseMainMenuButton && pauseMainMenuButton->hasComponent<UILabel>()) {
                    bool isOver = pauseMainMenuButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedPauseItem = PAUSE_MAIN_MENU;
                        pauseHighlightActive = true;
                    }
                }
                
                // Update hover states
                if (!anyHovered && pauseHighlightActive) {
                    pauseHighlightActive = false;
                    updatePauseMenu();
                } else if (anyHovered) {
                    updatePauseMenu();
                }
            }
            else if (gameState == STATE_SETTINGS) {
                // Handle settings menu mouse hover
                bool anyHovered = false;
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
                
                // Check for volume slider hover and drag
                int screenCenter = 1920 / 2;
                int sliderWidth = 400;
                int sliderHeight = 20;
                int sliderX = screenCenter - (sliderWidth / 2);
                int sliderY = 340;
                
                SDL_Rect sliderArea = {sliderX - 10, sliderY - 10, sliderWidth + 20, sliderHeight + 20};
                bool isOverSlider = mouseX >= sliderArea.x && mouseX <= sliderArea.x + sliderArea.w &&
                                  mouseY >= sliderArea.y && mouseY <= sliderArea.y + sliderArea.h;
                
                if (isOverSlider || draggingVolumeSlider) {
                    anyHovered = true;
                    selectedSettingsItem = SETTINGS_VOLUME;
                    settingsHighlightActive = true;
                }
                
                if (settingsBackButton && settingsBackButton->hasComponent<UILabel>()) {
                    bool isOver = settingsBackButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedSettingsItem = SETTINGS_BACK;
                        settingsHighlightActive = true;
                    }
                    
                    // Process hover effect for back button
                    settingsBackButton->getComponent<UILabel>().HandleEvent(event);
                }
                
                // Update volume if dragging
                if (draggingVolumeSlider) {
                    // Calculate volume based on mouse X position relative to slider
                    if (mouseX < sliderX) {
                        volumeLevel = 0;
                    } else if (mouseX > sliderX + sliderWidth) {
                        volumeLevel = 100;
                    } else {
                        volumeLevel = ((mouseX - sliderX) * 100) / sliderWidth;
                    }
                    
                    // Update the volume label
                    if (volumeLabel && volumeLabel->hasComponent<UILabel>()) {
                        std::stringstream volSS;
                        volSS << "Volume: " << volumeLevel << "%";
                        volumeLabel->getComponent<UILabel>().SetLabelText(volSS.str(), "font1");
                        
                        // Center the updated label
                        int volLabelWidth = volumeLabel->getComponent<UILabel>().GetWidth();
                        int volLabelX = (1920 - volLabelWidth) / 2;
                        volumeLabel->getComponent<UILabel>().SetPosition(volLabelX, 300);
                    }
                }
                
                // Update hover states
                if (!anyHovered && settingsHighlightActive && !draggingVolumeSlider) {
                    settingsHighlightActive = false;
                }
            }
            else if (gameState == STATE_END_SCREEN) {
                // First reset hover states on all end screen items
                bool anyHovered = false;
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
                
                // Handle individual hover events
                if (endRestartButton && endRestartButton->hasComponent<UILabel>()) {
                    bool isOver = endRestartButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedEndOption = END_RESTART;
                        endHighlightActive = true; // Enable highlighting when hovering
                    }
                }
                
                if (endReplayButton && endReplayButton->hasComponent<UILabel>()) {
                    bool isOver = endReplayButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedEndOption = END_REPLAY;
                        endHighlightActive = true; // Enable highlighting when hovering
                    }
                }
                
                if (endMenuButton && endMenuButton->hasComponent<UILabel>()) {
                    bool isOver = endMenuButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedEndOption = END_MAIN_MENU;
                        endHighlightActive = true; // Enable highlighting when hovering
                    }
                }
                
                // If no items are being hovered and we previously had highlights active
                if (!anyHovered && endHighlightActive) {
                    endHighlightActive = false; // Disable highlights when not hovering
                    updateEndScreen(); // Update to show no highlights
                } else if (anyHovered) {
                    // Process hover effects
                    if (endRestartButton && endRestartButton->hasComponent<UILabel>()) {
                        endRestartButton->getComponent<UILabel>().HandleEvent(event);
                    }
                    
                    if (endReplayButton && endReplayButton->hasComponent<UILabel>()) {
                        endReplayButton->getComponent<UILabel>().HandleEvent(event);
                    }
                    
                    if (endMenuButton && endMenuButton->hasComponent<UILabel>()) {
                        endMenuButton->getComponent<UILabel>().HandleEvent(event);
                    }
                }
            }
            break;
            
        case SDL_MOUSEBUTTONDOWN:
            if (gameState == STATE_MAIN_MENU) {
                // Handle mouse clicks for menu buttons
                if (menuNewGameButton && menuNewGameButton->hasComponent<UILabel>()) {
                    menuNewGameButton->getComponent<UILabel>().HandleEvent(event);
                }
                
                if (menuLoadGameButton && menuLoadGameButton->hasComponent<UILabel>()) {
                    menuLoadGameButton->getComponent<UILabel>().HandleEvent(event);
                }
                
                if (menuSettingsButton && menuSettingsButton->hasComponent<UILabel>()) {
                    menuSettingsButton->getComponent<UILabel>().HandleEvent(event);
                }
                
                if (menuLeaderboardButton && menuLeaderboardButton->hasComponent<UILabel>()) {
                    menuLeaderboardButton->getComponent<UILabel>().HandleEvent(event);
                }
                
                if (menuExitButton && menuExitButton->hasComponent<UILabel>()) {
                    menuExitButton->getComponent<UILabel>().HandleEvent(event);
                }
            }
            else if (gameState == STATE_PAUSE) {
                // Handle mouse clicks for pause menu buttons
                if (pauseResumeButton && pauseResumeButton->hasComponent<UILabel>()) {
                    pauseResumeButton->getComponent<UILabel>().HandleEvent(event);
                }
                
                if (pauseSaveButton && pauseSaveButton->hasComponent<UILabel>()) {
                    pauseSaveButton->getComponent<UILabel>().HandleEvent(event);
                }
                
                if (pauseRestartButton && pauseRestartButton->hasComponent<UILabel>()) {
                    pauseRestartButton->getComponent<UILabel>().HandleEvent(event);
                }
                
                if (pauseSettingsButton && pauseSettingsButton->hasComponent<UILabel>()) {
                    pauseSettingsButton->getComponent<UILabel>().HandleEvent(event);
                }
                
                if (pauseMainMenuButton && pauseMainMenuButton->hasComponent<UILabel>()) {
                    pauseMainMenuButton->getComponent<UILabel>().HandleEvent(event);
                }
            }
            else if (gameState == STATE_SETTINGS) {
                // Handle mouse clicks for settings menu
                int mouseX = event.button.x;
                int mouseY = event.button.y;
                
                // Check if clicking on volume slider
                int screenCenter = 1920 / 2;
                int sliderWidth = 400;
                int sliderHeight = 20;
                int sliderX = screenCenter - (sliderWidth / 2);
                int sliderY = 340;
                
                SDL_Rect sliderArea = {sliderX - 10, sliderY - 10, sliderWidth + 20, sliderHeight + 20};
                bool isOverSlider = mouseX >= sliderArea.x && mouseX <= sliderArea.x + sliderArea.w &&
                                  mouseY >= sliderArea.y && mouseY <= sliderArea.y + sliderArea.h;
                
                if (isOverSlider) {
                    draggingVolumeSlider = true;
                    
                    // Set volume immediately based on click position
                    if (mouseX < sliderX) {
                        volumeLevel = 0;
                    } else if (mouseX > sliderX + sliderWidth) {
                        volumeLevel = 100;
                    } else {
                        volumeLevel = ((mouseX - sliderX) * 100) / sliderWidth;
                    }
                    
                    // Update the volume label
                    if (volumeLabel && volumeLabel->hasComponent<UILabel>()) {
                        std::stringstream volSS;
                        volSS << "Volume: " << volumeLevel << "%";
                        volumeLabel->getComponent<UILabel>().SetLabelText(volSS.str(), "font1");
                        
                        // Center the updated label
                        int volLabelWidth = volumeLabel->getComponent<UILabel>().GetWidth();
                        int volLabelX = (1920 - volLabelWidth) / 2;
                        volumeLabel->getComponent<UILabel>().SetPosition(volLabelX, 300);
                    }
                }
                
                if (settingsBackButton && settingsBackButton->hasComponent<UILabel>()) {
                    settingsBackButton->getComponent<UILabel>().HandleEvent(event);
                }
            }
            else if (gameState == STATE_END_SCREEN) {
                // Handle mouse clicks for end screen buttons
                if (endRestartButton && endRestartButton->hasComponent<UILabel>()) {
                    endRestartButton->getComponent<UILabel>().HandleEvent(event);
                }
                
                if (endReplayButton && endReplayButton->hasComponent<UILabel>()) {
                    endReplayButton->getComponent<UILabel>().HandleEvent(event);
                }
                
                if (endMenuButton && endMenuButton->hasComponent<UILabel>()) {
                    endMenuButton->getComponent<UILabel>().HandleEvent(event);
                }
            }
            break;
            
        case SDL_MOUSEBUTTONUP:
            if (gameState == STATE_SETTINGS && draggingVolumeSlider) {
                draggingVolumeSlider = false;
            }
            break;
            
        default:
            break;
    }
}

void Game::update()
{
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    
    // Handle deferred restart and main menu return
    if (needsRestart) {
        needsRestart = false; // Reset flag
        
        // Clean up game entities
        if (map != nullptr) {
            delete map;
            map = nullptr;
        }
        
        // Clear all entities including UI
        manager.clear();
        
        // Set game state
        gameState = STATE_GAME;
        
        // Load first level
        loadLevel(currentLevel);
        
        // Reinitialize all game entities
        initEntities();
        
        // Return early to avoid processing other game logic
        return;
    }
    
    if (returnToMainMenu) {
        returnToMainMenu = false; // Reset flag
        
        // Reset all game state variables just like when restarting
        gameOver = false;
        playerWon = false;
        collectedClues = 0;
        damageTimer = 1.0f;
        objectCollisionDelay = 1.0f;
        objectCollisionsEnabled = false;
        questionActive = false;
        pendingClueEntity = nullptr;
        showFeedback = false;
        showingExitInstructions = false;
        level4MapChanged = false;
        finalBossDefeated = false;
        scientistRescued = false;
        canRescueScientist = false;
        currentLevel = 1;
        
        // Reset used questions
        resetUsedQuestions();
        
        // Set clue count for level 1
        totalClues = 3;
        
        // Reset position tracking
        positionManager.resetPositions();
        
        // Reset gameplay timer
        gameStartTime = 0;
        gameplayTime = 0;
        
        // Reset all entity pointers
        player = nullptr;
        finalBoss = nullptr;
        healthbar = nullptr;
        ammobar = nullptr;
        gameover = nullptr;
        clueCounter = nullptr;
        feedbackLabel = nullptr;
        scientist = nullptr;
        questionLabel = nullptr;
        answer1Label = nullptr;
        answer2Label = nullptr;
        answer3Label = nullptr;
        answer4Label = nullptr;
        questionBackground = nullptr;
        
        // Clean up game entities
        if (map != nullptr) {
            delete map;
            map = nullptr;
        }
        
        // Clear all entities
        manager.clear();
        
        // Set the game state
        gameState = STATE_MAIN_MENU;
        
        // Initialize main menu
        initMainMenu();
        
        // Return early to avoid processing other game logic
        return;
    }
    
    // Handle updates based on current game state
    switch (gameState) {
        case STATE_MAIN_MENU:
            // Nothing to update in the main menu except UI
            break;
            
        case STATE_END_SCREEN:
            // Update end screen elements
            updateEndScreen();
            break;
            
        case STATE_PAUSE:
            // Update pause menu elements
            updatePauseMenu();
            break;
            
        case STATE_SETTINGS:
            // Update settings menu elements
            updateSettingsMenu();
            break;
            
        case STATE_REPLAY:
            // Update replay ONLY
            updateReplay();
            // Skip manager.refresh() and manager.update() for replay state
            // Refresh is not needed as replay handles its entity directly
            // Update is skipped to prevent interference with replay positioning
            break; 
            
        case STATE_GAME:
            // Always refresh entity manager first in GAME state
            manager.refresh();
            
            // Record player position if recording is enabled and player exists
            if (player && player->isActive() && !isReplaying) { // Ensure not replaying
                recordPlayerPosition();
            }
            
            // Update timer
            if (gameStartTime > 0 && !gameOver) {
                gameplayTime = currentTime - gameStartTime;
                
                // Format time as MM:SS
                Uint32 totalSeconds = gameplayTime / 1000;
                Uint32 minutes = totalSeconds / 60;
                Uint32 seconds = totalSeconds % 60;
                
                std::stringstream timeSS;
                timeSS << "Time: " << std::setfill('0') << std::setw(2) << minutes 
                       << ":" << std::setfill('0') << std::setw(2) << seconds;
                
                if (timerLabel != nullptr && timerLabel->hasComponent<UILabel>()) {
                    timerLabel->getComponent<UILabel>().SetLabelText(timeSS.str(), "font1");
                    
                    // Get the width of the updated text and recenter it
                    int timerWidth = timerLabel->getComponent<UILabel>().GetWidth();
                    int xPos = (1920 - timerWidth) / 2; // Center horizontally
                    timerLabel->getComponent<UILabel>().SetPosition(xPos, 20); // Keep at top
                }
            }
            
            // Handle transition if active
            if (transitionManager.isTransitioning()) {
                if (transitionManager.updateTransition()) {
                    // Transition is complete, proceed with level change
                    // Reset game state variables for next level
                    collectedClues = 0;
                    damageTimer = 1.0f;
                    objectCollisionDelay = 1.0f;
                    objectCollisionsEnabled = false;
                    questionActive = false;
                    pendingClueEntity = nullptr;
                    showFeedback = false;
                    showingExitInstructions = false; // Reset exit instructions flag
                    
                    // Reset position tracking
                    positionManager.resetPositions();
                    
                    // Store current level before incrementing
                    int nextLevel = currentLevel + 1;
                    currentLevel = nextLevel;
                    
                    // Clear all entities including UI
                    manager.clear();
                    
                    // Make sure map is properly deallocated
                    if (map != nullptr) {
                        delete map;
                        map = nullptr;
                    }
                    
                    // Load the new level map
                    loadLevel(currentLevel);
                    
                    // Re-initialize all game entities and UI
                    initEntities();
                    
                    // Recreate transition label after entity initialization
                    transitionLabel = &manager.addEntity();
                    transitionLabel->addComponent<UILabel>(0, 0, "", "font2", white);
                    
                    // Reinitialize the transition manager after entities are created
                    transitionManager.mTransitionLabel = transitionLabel;
                    transitionManager.init(this, &manager);
                }
        return;
    }
    
    // Handle animations for all entities regardless of game state
    for(auto& p : *players) {
        if (p->hasComponent<SpriteComponent>()) {
            p->getComponent<SpriteComponent>().update();
        }
    }
    
    for(auto& e : *enemies) {
        if (e->hasComponent<SpriteComponent>()) {
            e->getComponent<SpriteComponent>().update();
        }
        // Continue enemy movement animations if not in question mode
        if (!questionActive && !gameOver && e->hasComponent<EnemyAIComponent>()) {
            e->getComponent<EnemyAIComponent>().update();
        } else if ((questionActive || gameOver) && e->hasComponent<SpriteComponent>()) {
            // Ensure enemies stay in idle animation during question mode or game over
            e->getComponent<SpriteComponent>().Play("Idle");
        }
    }
    
    for(auto& p : *projectiles) {
        if (p->hasComponent<SpriteComponent>()) {
            p->getComponent<SpriteComponent>().update();
        }
    }
    
    for(auto& o : *objects) {
        if(o->hasComponent<SpriteComponent>()) {
            o->getComponent<SpriteComponent>().update();
        }
    }
    
    // Only update player-related UI if the player exists and game is not over
    if (player != nullptr && player->isActive() && !gameOver) {
        // Keep game state updated for UI elements
        int health = player->getComponent<HealthComponent>().health;
        std::stringstream healthSS;
        healthSS << "Health: " << health;
        healthbar->getComponent<UILabel>().SetLabelText(healthSS.str(), "font1");
        
        int ammo = player->getComponent<AmmoComponent>().currentAmmo;
        std::stringstream ammoSS;
        ammoSS << "Ammo: " << ammo;
        ammobar->getComponent<UILabel>().SetLabelText(ammoSS.str(), "font1");

        std::stringstream clueSS;
        clueSS << "Clues: " << collectedClues << "/" << totalClues;
        clueCounter->getComponent<UILabel>().SetLabelText(clueSS.str(), "font1");
    }

    // Check if feedback timer has expired
    if (showFeedback) {
        // For regular feedback from questions
        if (!showingExitInstructions && (currentTime - feedbackStartTime > 1500)) {
            closeQuestion();
        } 
        // For exit instructions, keep visible until player reaches exit
        // Don't hide the "head north" message
    }

    // If game is in question mode or game over, skip gameplay logic but continue animations
    if (questionActive || gameOver) {
        // Ensure player doesn't move during questions if player exists
        if (questionActive && player != nullptr && player->isActive() && 
            player->hasComponent<TransformComponent>()) {
            player->getComponent<TransformComponent>().velocity.x = 0;
            player->getComponent<TransformComponent>().velocity.y = 0;
        }
        return;
    }
    
    // Run manager update ONLY for STATE_GAME when not paused/game over
    manager.update();
    
    // Continue with regular game update logic - only if player exists
    if (player != nullptr && player->isActive() && gameState == STATE_GAME) {
        Vector2D playerPos = player->getComponent<TransformComponent>().position;
        
        // Get updated player position after manager.update()
        TransformComponent& playerTransform = player->getComponent<TransformComponent>();
        
        // Define world boundaries
        int worldWidth = 60 * 32 * 2;  // Assuming map size based on Map::LoadMap usage
        int worldHeight = 34 * 32 * 2;
        
        // Player dimensions
        int playerWidth = playerTransform.width * playerTransform.scale;
        int playerHeight = playerTransform.height * playerTransform.scale;
        
        // Clamp player position to world boundaries
        if (playerTransform.position.x < 0) {
            playerTransform.position.x = 0;
        }
        if (playerTransform.position.y < 0) {
            playerTransform.position.y = 0;
        }
        if (playerTransform.position.x + playerWidth > worldWidth) {
            playerTransform.position.x = worldWidth - playerWidth;
        }
        if (playerTransform.position.y + playerHeight > worldHeight) {
            playerTransform.position.y = worldHeight - playerHeight;
        }

        // Update player collider after potential position clamping
        player->getComponent<ColliderComponent>().update();
        SDL_Rect playerCol = player->getComponent<ColliderComponent>().collider;
        
        damageTimer -= 1.0f/60.0f;
        
        if (!objectCollisionsEnabled) {
            objectCollisionDelay -= 1.0f/60.0f;
            if (objectCollisionDelay <= 0.0f) {
                objectCollisionsEnabled = true;
            }
        }

        // Handle object collisions
        if (objectCollisionsEnabled) {
            for (auto& o : *objects) {
                if (Collision::AABB(player->getComponent<ColliderComponent>().collider,
                                o->getComponent<ColliderComponent>().collider)) {
                    if (o->getComponent<ColliderComponent>().tag == "clue") {
                        showQuestion(o);
                    }
                    else if (o->getComponent<ColliderComponent>().tag == "magazine") {
                        player->getComponent<AmmoComponent>().addAmmo();
                        o->destroy();
                    }
                    else if (o->getComponent<ColliderComponent>().tag == "healthpotion") {
                        player->getComponent<HealthComponent>().heal(20);
                        o->destroy();
                    }
                }
            }
        }

        // Player collision with terrain
        for(auto& c : *colliders) {
            SDL_Rect cCol = c->getComponent<ColliderComponent>().collider;
            SDL_Rect updatedPlayerCol = player->getComponent<ColliderComponent>().collider; // Get player collider AFTER movement
            
            if(Collision::AABB(cCol, updatedPlayerCol)) {
                // Calculate overlap
                float overlapX = 0.0f;
                float overlapY = 0.0f;

                // Calculate the distance between centers
                float dx = (updatedPlayerCol.x + updatedPlayerCol.w / 2.0f) - (cCol.x + cCol.w / 2.0f);
                float dy = (updatedPlayerCol.y + updatedPlayerCol.h / 2.0f) - (cCol.y + cCol.h / 2.0f);

                // Calculate the minimum non-overlapping distances
                float combinedHalfWidths = (updatedPlayerCol.w / 2.0f) + (cCol.w / 2.0f);
                float combinedHalfHeights = (updatedPlayerCol.h / 2.0f) + (cCol.h / 2.0f);

                // Calculate overlap on each axis
                overlapX = combinedHalfWidths - std::abs(dx);
                overlapY = combinedHalfHeights - std::abs(dy);

                // Resolve collision based on the axis with the smallest overlap
                if (overlapX < overlapY) {
                    // Push horizontally
                    if (dx > 0) { // Player is to the right of collider
                        playerTransform.position.x += overlapX;
                    } else { // Player is to the left of collider
                        playerTransform.position.x -= overlapX;
                    }
                    // playerTransform.velocity.x = 0; // REMOVED: Allow input to control velocity
                } else {
                    // Push vertically
                    if (dy > 0) { // Player is below collider
                        playerTransform.position.y += overlapY;
                    } else { // Player is above collider
                        playerTransform.position.y -= overlapY;
                    }
                    // playerTransform.velocity.y = 0; // REMOVED: Allow input to control velocity
                }

                // Update player's collider component immediately after position change
                player->getComponent<ColliderComponent>().update(); 
            }
        }

        // Enemy collision with projectiles and player
        for(auto& e : *enemies) {
            // Enemy needs access to its transform for collision response
            TransformComponent& enemyTransform = e->getComponent<TransformComponent>();

            // --- Enemy Terrain Collision Check ---
            for (auto& c : *colliders) {
                SDL_Rect cCol = c->getComponent<ColliderComponent>().collider;
                SDL_Rect enemyCol = e->getComponent<ColliderComponent>().collider; // Get enemy collider AFTER its movement

                if (Collision::AABB(cCol, enemyCol)) {
                    // Calculate overlap (similar to player)
                    float overlapX = 0.0f;
                    float overlapY = 0.0f;
                    float dx = (enemyCol.x + enemyCol.w / 2.0f) - (cCol.x + cCol.w / 2.0f);
                    float dy = (enemyCol.y + enemyCol.h / 2.0f) - (cCol.y + cCol.h / 2.0f);
                    float combinedHalfWidths = (enemyCol.w / 2.0f) + (cCol.w / 2.0f);
                    float combinedHalfHeights = (enemyCol.h / 2.0f) + (cCol.h / 2.0f);
                    overlapX = combinedHalfWidths - std::abs(dx);
                    overlapY = combinedHalfHeights - std::abs(dy);

                    bool collidedHorizontally = false;
                    bool collidedVertically = false;

                    // Resolve collision (Push enemy back)
                    if (overlapX < overlapY) {
                        collidedHorizontally = true;
                        if (dx > 0) enemyTransform.position.x += overlapX;
                        else enemyTransform.position.x -= overlapX;
                        enemyTransform.velocity.x = 0; // Stop horizontal movement *for this frame* 
                    } else {
                        collidedVertically = true;
                        if (dy > 0) enemyTransform.position.y += overlapY;
                        else enemyTransform.position.y -= overlapY;
                        enemyTransform.velocity.y = 0; // Stop vertical movement *for this frame*
                    }
                    
                    // Notify AI component about the collision
                    if (e->hasComponent<EnemyAIComponent>()) {
                        e->getComponent<EnemyAIComponent>().notifyTerrainCollision(collidedHorizontally, collidedVertically);
                    }

                    // Update enemy's collider component immediately
                    e->getComponent<ColliderComponent>().update();
                }
            }
            // --- End Enemy Terrain Collision Check ---

            // Projectile collision
            for(auto& p : *projectiles) {
                if(Collision::AABB(e->getComponent<ColliderComponent>().collider, 
                                p->getComponent<ColliderComponent>().collider)) {
                    e->getComponent<HealthComponent>().takeDamage(25);
                    p->destroy();
                }
            }

            // Player collision with damage cooldown
            SDL_Rect updatedPlayerCol = player->getComponent<ColliderComponent>().collider;
            if(Collision::AABB(updatedPlayerCol, e->getComponent<ColliderComponent>().collider) && damageTimer <= 0) {
                // Boss deals more damage
                if (currentLevel == 4 && e == finalBoss) {
                    player->getComponent<HealthComponent>().takeDamage(10); // Boss deals more damage
                } else {
                    player->getComponent<HealthComponent>().takeDamage(5);
                }
                damageTimer = damageCooldown;
            }

            // Destroy dead enemies
            if(e->getComponent<HealthComponent>().health <= 0) {
                // Check if the enemy is the final boss
                if (currentLevel == 4 && e == finalBoss) {
                    finalBossDefeated = true;
                    // Show boss defeated message
                    feedbackLabel->getComponent<UILabel>().SetLabelText("BOSS DEFEATED! The path is revealed! Find and rescue the SUPERUM!", "font1", {255, 215, 0, 255});
                    
                    // Position the feedback at the bottom of the screen
                    int feedbackWidth = feedbackLabel->getComponent<UILabel>().GetWidth();
                    int xPos = (1920 - feedbackWidth) / 2;
                    feedbackLabel->getComponent<UILabel>().SetPosition(xPos, 950);
                    
                    // Show feedback
                    showFeedback = true;
                    feedbackStartTime = SDL_GetTicks();
                    
                    // Enable scientist rescue interaction
                    canRescueScientist = true;
                    
                    // Change the map to reveal the exit
                    level4MapChanged = true;
                    
                    // First, destroy all collider entities to avoid stale colliders
                    for (auto c : *colliders) {
                        c->destroy();
                    }
                    
                    // Reload the map with the new file
                    if (map != nullptr) {
                        delete map;
                        map = nullptr;
                    }
                    
                    // Keep the same texture but load the "after" map
                    std::string terrainTexture = "terrainlvl4";
                    std::string mapPath = "./assets/lvl4/Level4MapAfter.map";
                    
                    // Create and load the new map
                    map = new Map(terrainTexture, 2, 32, manager);
                    map->LoadMap(mapPath, 60, 34);
                    
                    // Allow the manager to do a refresh to properly clean up destroyed entities
                    manager.refresh();
                }
                
                e->destroy();
            }
        }

        // Check if player is dead
        if(player->getComponent<HealthComponent>().health <= 0) {
            // Destroy player
            player->destroy();
            
            // Reset all enemy animations to idle
            for(auto& e : *enemies) {
                if(e->hasComponent<SpriteComponent>()) {
                    e->getComponent<SpriteComponent>().Play("Idle");
                }
            }
            
            // Mark game as over
            gameOver = true;
            playerWon = false;
            
            // Show end screen with defeat
            initEndScreen(false);
            return;
        }

        // Center camera on player
        camera.x = player->getComponent<TransformComponent>().position.x - (camera.w / 2);
        camera.y = player->getComponent<TransformComponent>().position.y - (camera.h / 2);

        // Camera bounds
        if(camera.x < 0) camera.x = 0;
        if(camera.y < 0) camera.y = 0;
        if(camera.x > worldWidth - camera.w) camera.x = worldWidth - camera.w;
        if(camera.y > worldHeight - camera.h) camera.y = worldHeight - camera.h;
        
        // Check win condition
        if ((currentLevel != 4 && collectedClues >= totalClues) || 
            (currentLevel == 4 && finalBossDefeated)) {
            // All clues collected or boss defeated, but still require player to go north/exit
            if (!showingExitInstructions) {
                // Show instructions to player only once
                if (currentLevel == 3) {
                    feedbackLabel->getComponent<UILabel>().SetLabelText("All clues collected! Enter the pyramid to see what lies ahead.", "font1", {255, 215, 0, 255});
                } else if (currentLevel == 4 && level4MapChanged) {
                    // Boss already defeated, path revealed message shown previously
                    // Just enable exit instructions mode
                    showingExitInstructions = true;
                } else {
                    feedbackLabel->getComponent<UILabel>().SetLabelText("All clues collected! Head NORTH to exit the level.", "font1", {255, 215, 0, 255});
                }
                
                // Position the feedback at the bottom of the screen
                int feedbackWidth = feedbackLabel->getComponent<UILabel>().GetWidth();
                int xPos = (1920 - feedbackWidth) / 2;
                feedbackLabel->getComponent<UILabel>().SetPosition(xPos, 950); // Bottom of screen
                
                // Show feedback
                showFeedback = true;
                feedbackStartTime = SDL_GetTicks();
                showingExitInstructions = true;
            }
            
            // Check win condition based on level
            if (currentLevel == 3) {
                // For level 3, check if player has entered the pyramid at tiles 20 and 21 (x=25,y=20 and x=26,y=20)
                float playerX = player->getComponent<TransformComponent>().position.x / 64;
                float playerY = player->getComponent<TransformComponent>().position.y / 64;
                
                // Check if player is at the pyramid entrance
                if ((playerX >= 24 && playerX <= 26) && (playerY >= 19 && playerY <= 20)) {
                    // Player has reached the pyramid, proceed to next level or win
                    advanceToNextLevel();
                }
            } else {
                // For other levels, check if player has gone far enough north
                if (player->getComponent<TransformComponent>().position.y < 100) {
                        advanceToNextLevel();
                }
            }
        }
            }
            break;
            
        default:
            break;
    }
}

void Game::render() {
    // Render based on current game state
    switch (gameState) {
        case STATE_MAIN_MENU:
            renderMainMenu();
            break;
            
        case STATE_END_SCREEN:
            renderEndScreen();
            break;
            
        case STATE_PAUSE:
            renderPauseMenu();
            break;
            
        case STATE_SETTINGS:
            renderSettingsMenu();
            break;
            
        case STATE_REPLAY:
            renderReplay();
            break;
            
        case STATE_GAME:
            SDL_RenderClear(renderer);
    
    // If transitioning, only render the transition screen
            if (transitionManager.isTransitioning()) {
                transitionManager.renderTransition();
        SDL_RenderPresent(renderer);
        return;
    }
    
    // Always render game elements
    for(auto& t : *tiles) t->draw();
    for(auto& p : *players) p->draw();
    for(auto& e : *enemies) e->draw();
    for(auto& o : *objects) o->draw();
    for(auto& p : *projectiles) p->draw();
    
    // Ensure scientist is drawn if it exists
    if (scientist != nullptr && scientist->isActive()) {
        scientist->draw();
    }
    
    // Always render UI elements
    healthbar->draw();
    ammobar->draw();
    clueCounter->draw();
    timerLabel->draw();
    gameover->draw();
    
    // Show interaction prompt for scientist if player is close
    if (currentLevel == 4 && canRescueScientist && !scientistRescued && 
        player != nullptr && scientist != nullptr) {
        Vector2D playerPos = player->getComponent<TransformComponent>().position;
        Vector2D scientistPos = scientist->getComponent<TransformComponent>().position;
        float distance = sqrt(pow(playerPos.x - scientistPos.x, 2) + pow(playerPos.y - scientistPos.y, 2));
        
        if (distance <= 100) { // Within interaction range
            // Draw interaction prompt with lower opacity background
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120); // Reduced opacity (120 instead of 200)
            
            // Calculate prompt position (centered above scientist)
            int promptX = static_cast<int>(scientistPos.x) - Game::camera.x + (32 * 3) / 2; // Center on sprite (32px width * 3 scale)
            int promptY = static_cast<int>(scientistPos.y) - Game::camera.y - 50; // Moved up a bit more
            
            // Create a wider background rect for prompt
            SDL_Rect promptRect = {promptX - 75, promptY - 15, 150, 35}; // Wider by 30px (15px on each side)
            SDL_RenderFillRect(renderer, &promptRect);
            
            // Add a subtle border to the prompt
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 80); // Very transparent white
            SDL_RenderDrawRect(renderer, &promptRect); // Just the outline
            
            // Set up temporary label for "Press E"
            static Entity* promptLabel = nullptr;
            if (promptLabel == nullptr) {
                promptLabel = &manager.addEntity();
                SDL_Color dimWhite = {220, 220, 220, 255}; // Slightly dimmer text
                promptLabel->addComponent<UILabel>(0, 0, "Press E", "font1", dimWhite);
            }
            
            // Position the prompt
            int labelWidth = promptLabel->getComponent<UILabel>().GetWidth();
            promptLabel->getComponent<UILabel>().SetPosition(promptX - labelWidth/2, promptY - 10);
            promptLabel->draw();
            
            // Reset draw color
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }
    }
    
    // Draw feedback if active (for both question feedback and exit instructions)
    if (showFeedback && feedbackLabel != nullptr) {
        // Create special background for feedback only for regular feedback, not exit instructions
        if (!showingExitInstructions) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 230); // Darker background for feedback
            SDL_Rect feedbackBg = {1920/4, 630, 1920/2, 100};
            SDL_RenderFillRect(renderer, &feedbackBg);
        }
        
        // Draw the feedback text
        feedbackLabel->draw();
        
        // Reset draw color
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }
    
    // Render question UI on top if active
    if (questionActive) {
        // Create a semi-transparent background for the question
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200); // Semi-transparent black
        SDL_Rect questionBg = {1920/6, 250, 1920*2/3, 350}; // Wider and taller background
        SDL_RenderFillRect(renderer, &questionBg);
        
        // Draw question elements
        questionLabel->draw();
        answer1Label->draw();
        answer2Label->draw();
        answer3Label->draw();
        answer4Label->draw();
        
        // Reset draw color
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }

    SDL_RenderPresent(renderer);
            break;
            
        default:
            break;
    }
}

void Game::clean()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    std::cout << "Game cleaned" << std::endl;
}

void Game::restart() {
    // Reset game state variables
    gameOver = false;
    playerWon = false;
    collectedClues = 0;
    damageTimer = 1.0f;
    objectCollisionDelay = 1.0f;
    objectCollisionsEnabled = false;
    questionActive = false;
    pendingClueEntity = nullptr;
    showFeedback = false;
    showingExitInstructions = false; // Reset exit instructions flag
    level4MapChanged = false; // Reset level 4 map state
    finalBossDefeated = false; // Reset final boss state
    scientistRescued = false; // Reset scientist state
    canRescueScientist = false; // Reset scientist interaction flag
    currentLevel = 1;
    gameState = STATE_GAME; // Ensure we're in game state
    
    // Reset used questions when restarting the game
    resetUsedQuestions();
    
    // Set clue count correctly for level 1
    totalClues = 3; // Reset to default value, loadLevel will adjust if needed
    
    // Reset position tracking
    positionManager.resetPositions();
    
    // Clear all entities including UI
    manager.clear();
    
    // Make sure map is properly deallocated
    if (map != nullptr) {
        delete map;
        map = nullptr;
    }
    
    // Reset gameplay timer
    gameStartTime = SDL_GetTicks();
    gameplayTime = 0;
    
    // Load first level
    loadLevel(currentLevel);
    
    // Reinitialize all game entities
    initEntities();
}

void Game::showQuestion(Entity* clueEntity) {
    if (questionActive) return;
    
    questionActive = true;
    answerSubmitted = false; // Reset submission status for new question
    pendingClueEntity = clueEntity;
    
    // Set player to idle animation when entering question mode
    if (player != nullptr && player->isActive()) {
        player->getComponent<SpriteComponent>().Play("Idle");
        
        // Disable keyboard controls during question mode
        if (player->hasComponent<KeyboardController>()) {
            player->getComponent<KeyboardController>().enabled = false;
        }
    }
    
    // Set all enemies to idle animation
    for(auto& e : *enemies) {
        e->getComponent<SpriteComponent>().Play("Idle");
    }
    
    // Check if we've used all questions
    if (usedQuestions.size() >= questions.size()) {
        // All questions have been used, reset the set to allow reuse
        usedQuestions.clear();
    }
    
    // Use proper seeded random for question selection
    static std::mt19937 rng(std::time(nullptr));
    
    // Select only from unused questions
    std::vector<int> availableQuestions;
    for (size_t i = 0; i < questions.size(); i++) {
        if (usedQuestions.find(i) == usedQuestions.end()) {
            availableQuestions.push_back(i);
        }
    }
    
    // Select a random question from available ones
    int randomIndex = rng() % availableQuestions.size();
    currentQuestion = availableQuestions[randomIndex];
    
    // Mark this question as used
    usedQuestions.insert(currentQuestion);
    
    Question q = questions[currentQuestion];
    
    // Make the question more visible with better formatting
    SDL_Color questionColor = {255, 255, 0, 255}; // Yellow for questions
    SDL_Color answerColor = {255, 255, 255, 255}; // White for answers
    
    // Format the question and answers for better visibility
    questionLabel->getComponent<UILabel>().SetLabelText(q.question, "font1", questionColor);
    answer1Label->getComponent<UILabel>().SetLabelText("1: " + q.answers[0], "font1", answerColor);
    answer2Label->getComponent<UILabel>().SetLabelText("2: " + q.answers[1], "font1", answerColor);
    answer3Label->getComponent<UILabel>().SetLabelText("3: " + q.answers[2], "font1", answerColor);
    answer4Label->getComponent<UILabel>().SetLabelText("4: " + q.answers[3], "font1", answerColor);
    
    // Center the question text horizontally
    int questionWidth = questionLabel->getComponent<UILabel>().GetWidth();
    int xPos = (1920 - questionWidth) / 2;
    
    // Position all question elements centered on screen
    questionLabel->getComponent<UILabel>().SetPosition(xPos, 300);
    
    // Align answers below the question
    int xPosAnswers = 1920 / 3; // Move answers a bit to the left
    answer1Label->getComponent<UILabel>().SetPosition(xPosAnswers, 360);
    answer2Label->getComponent<UILabel>().SetPosition(xPosAnswers, 410);
    answer3Label->getComponent<UILabel>().SetPosition(xPosAnswers, 460);
    answer4Label->getComponent<UILabel>().SetPosition(xPosAnswers, 510);
}

void Game::checkAnswer(int selectedAnswer) {
    if (!questionActive || answerSubmitted) return;
    
    // Mark that an answer has been submitted to prevent multiple attempts
    answerSubmitted = true;
    
    isAnswerCorrect = (selectedAnswer == questions[currentQuestion].correctAnswer);
    
    if (isAnswerCorrect) {
        collectedClues++;
        pendingClueEntity->destroy();
        feedbackLabel->getComponent<UILabel>().SetLabelText("CORRECT!", "font2", green);
    } else {
        feedbackLabel->getComponent<UILabel>().SetLabelText("INCORRECT!", "font2", red);
    }
    
    // Position the feedback centered horizontally and lower on the screen
    int feedbackWidth = feedbackLabel->getComponent<UILabel>().GetWidth();
    int xPos = (1920 - feedbackWidth) / 2;
    feedbackLabel->getComponent<UILabel>().SetPosition(xPos, 650);
    
    // Show feedback for a short time
    showFeedback = true;
    feedbackStartTime = SDL_GetTicks();
}

void Game::closeQuestion() {
    questionActive = false;
    answerSubmitted = false; // Reset for next question
    pendingClueEntity = nullptr;
    showFeedback = false;
    
    // Re-enable keyboard controls if player exists
    if (player != nullptr && player->isActive() && player->hasComponent<KeyboardController>()) {
        player->getComponent<KeyboardController>().enabled = true;
    }
    
    questionLabel->getComponent<UILabel>().SetLabelText("", "font1");
    answer1Label->getComponent<UILabel>().SetLabelText("", "font1");
    answer2Label->getComponent<UILabel>().SetLabelText("", "font1");
    answer3Label->getComponent<UILabel>().SetLabelText("", "font1");
    answer4Label->getComponent<UILabel>().SetLabelText("", "font1");
    feedbackLabel->getComponent<UILabel>().SetLabelText("", "font2");
}

void Game::loadLevel(int levelNum) {
    // Make sure to clean up the previous map
    if (map != nullptr) {
        delete map;
        map = nullptr;
    }
    
    // Set level-specific properties
    currentLevel = levelNum;
    collectedClues = 0;
    showingExitInstructions = false; // Reset exit instructions flag for new level
    
    // Set level-specific quantities
    if (currentLevel == 1) {
        totalClues = 3;
        totalMagazines = 3;
        totalHealthPotions = 2;
    } else if (currentLevel == 2) {
        totalClues = 5;
        totalMagazines = 9;  
        totalHealthPotions = 9;
    } else if (currentLevel == 3) {
        totalClues = 7;
        totalMagazines = 15;  
        totalHealthPotions = 15;
    } else if (currentLevel == 4) {
        totalClues = 0; // No clues in level 4, boss battle instead
        totalMagazines = 7;
        totalHealthPotions = 9;
        level4MapChanged = false; // Reset the map change flag for level 4
        finalBossDefeated = false; // Reset the boss defeated flag
    }
    
    // Create appropriate map based on level number
    std::string terrainTexture = "terrainlvl" + std::to_string(levelNum);
    std::string mapPath;
    
    if (currentLevel == 4 && level4MapChanged) {
        mapPath = "./assets/lvl4/Level4MapAfter.map";
    } else {
        mapPath = "./assets/lvl" + std::to_string(levelNum) + "/Level" + std::to_string(levelNum) + "Map.map";
    }
    
    // Ensure texture is loaded
    if (Game::assets->GetTexture(terrainTexture) == nullptr) {
        return;
    }
    
    // Create the map
    map = new Map(terrainTexture, 2, 32, manager);
    
    // Load the map data
    map->LoadMap(mapPath, 60, 34);
}

void Game::advanceToNextLevel() {
    // Ensure we're in GAME state when starting a transition
    gameState = STATE_GAME;
    
    // Start transition sequence using the transition manager
    transitionManager.startTransition(currentLevel, currentLevel + 1);
    
    // Make sure the transition label is properly initialized
    if (transitionLabel == nullptr) {
        transitionLabel = &manager.addEntity();
        transitionLabel->addComponent<UILabel>(0, 0, "", "font2", white);
    }
    
    // Continue recording positions in the next level
    // Reset last recorded position for the next level
    lastRecordedPosition = Vector2D(0, 0);
}

void Game::initMainMenu() {
    // Reset selection and hover states
    selectedMenuItem = MENU_NEW_GAME; // Default selection
    menuItemSelected = false;
    menuHighlightActive = false; // Start with no highlights
    
    // Create menu entity objects
    menuTitle = &manager.addEntity();
    menuNewGameButton = &manager.addEntity();
    menuLoadGameButton = &manager.addEntity();
    menuSettingsButton = &manager.addEntity();  // Add settings button
    menuLeaderboardButton = &manager.addEntity();  // Add leaderboard button
    menuExitButton = &manager.addEntity();
    
    // Set up UI components
    menuTitle->addComponent<UILabel>(0, 200, "Dejte mi RPA 3 prosm", "font2", white);
    
    // Increased spacing between menu buttons
    menuNewGameButton->addComponent<UILabel>(0, 400, "NEW GAME", "font1", white);
    menuLoadGameButton->addComponent<UILabel>(0, 470, "LOAD GAME", "font1", white);  // From 450 to 470
    menuSettingsButton->addComponent<UILabel>(0, 540, "SETTINGS", "font1", white);   // From 500 to 540
    menuLeaderboardButton->addComponent<UILabel>(0, 610, "LEADERBOARD", "font1", white); // From 550 to 610
    menuExitButton->addComponent<UILabel>(0, 680, "EXIT", "font1", white);  // From 600 to 680
    
    // Center the menu items horizontally
    int titleWidth = menuTitle->getComponent<UILabel>().GetWidth();
    int newGameWidth = menuNewGameButton->getComponent<UILabel>().GetWidth();
    int loadGameWidth = menuLoadGameButton->getComponent<UILabel>().GetWidth();
    int settingsWidth = menuSettingsButton->getComponent<UILabel>().GetWidth();  // Get settings width
    int leaderboardWidth = menuLeaderboardButton->getComponent<UILabel>().GetWidth();  // Get leaderboard width
    int exitWidth = menuExitButton->getComponent<UILabel>().GetWidth();
    
    int titleX = (1920 - titleWidth) / 2;
    int newGameX = (1920 - newGameWidth) / 2;
    int loadGameX = (1920 - loadGameWidth) / 2;
    int settingsX = (1920 - settingsWidth) / 2;  // Calculate settings position
    int leaderboardX = (1920 - leaderboardWidth) / 2;  // Calculate leaderboard position
    int exitX = (1920 - exitWidth) / 2;
    
    menuTitle->getComponent<UILabel>().SetPosition(titleX, 200);
    menuNewGameButton->getComponent<UILabel>().SetPosition(newGameX, 400);
    menuLoadGameButton->getComponent<UILabel>().SetPosition(loadGameX, 470);  // From 450 to 470
    menuSettingsButton->getComponent<UILabel>().SetPosition(settingsX, 540);  // From 500 to 540
    menuLeaderboardButton->getComponent<UILabel>().SetPosition(leaderboardX, 610);  // From 550 to 610
    menuExitButton->getComponent<UILabel>().SetPosition(exitX, 680);  // From 600 to 680
    
    // Make menu items clickable but ensure they're not hovered initially
    menuNewGameButton->getComponent<UILabel>().SetClickable(true);
    menuNewGameButton->getComponent<UILabel>().SetOnClick([this]() { startGame(); });
    menuNewGameButton->getComponent<UILabel>().SetHoverColor(yellow);
    menuNewGameButton->getComponent<UILabel>().ResetHoverState();
    
    menuLoadGameButton->getComponent<UILabel>().SetClickable(true);
    menuLoadGameButton->getComponent<UILabel>().SetOnClick([this]() { loadGame(); });
    menuLoadGameButton->getComponent<UILabel>().SetHoverColor(yellow);
    menuLoadGameButton->getComponent<UILabel>().ResetHoverState();
    
    // Add settings button functionality
    menuSettingsButton->getComponent<UILabel>().SetClickable(true);
    menuSettingsButton->getComponent<UILabel>().SetOnClick([this]() { 
        previousState = STATE_MAIN_MENU; // Store that we came from main menu
        gameState = STATE_SETTINGS;
        initSettingsMenu();
    });
    menuSettingsButton->getComponent<UILabel>().SetHoverColor(yellow);
    menuSettingsButton->getComponent<UILabel>().ResetHoverState();
    
    // Add leaderboard button functionality
    menuLeaderboardButton->getComponent<UILabel>().SetClickable(true);
    menuLeaderboardButton->getComponent<UILabel>().SetOnClick([this]() { 
        // This function currently does nothing
        // Will be implemented later for the leaderboard
    });
    menuLeaderboardButton->getComponent<UILabel>().SetHoverColor(yellow);
    menuLeaderboardButton->getComponent<UILabel>().ResetHoverState();
    
    menuExitButton->getComponent<UILabel>().SetClickable(true);
    menuExitButton->getComponent<UILabel>().SetOnClick([this]() { isRunning = false; });
    menuExitButton->getComponent<UILabel>().SetHoverColor(yellow);
    menuExitButton->getComponent<UILabel>().ResetHoverState();
    
    // No need to call updateMainMenu() since menuHighlightActive is false
}

void Game::updateMainMenu() {
    // Reset all menu items to default state
    if (menuNewGameButton && menuNewGameButton->hasComponent<UILabel>()) {
        menuNewGameButton->getComponent<UILabel>().ResetHoverState();
    }
    
    if (menuLoadGameButton && menuLoadGameButton->hasComponent<UILabel>()) {
        menuLoadGameButton->getComponent<UILabel>().ResetHoverState();
    }
    
    if (menuSettingsButton && menuSettingsButton->hasComponent<UILabel>()) {
        menuSettingsButton->getComponent<UILabel>().ResetHoverState();
    }
    
    if (menuLeaderboardButton && menuLeaderboardButton->hasComponent<UILabel>()) {
        menuLeaderboardButton->getComponent<UILabel>().ResetHoverState();
    }
    
    if (menuExitButton && menuExitButton->hasComponent<UILabel>()) {
        menuExitButton->getComponent<UILabel>().ResetHoverState();
    }
    
    // Only highlight the selected item if highlight is active
    if (menuHighlightActive) {
        switch (selectedMenuItem) {
            case MENU_NEW_GAME:
                if (menuNewGameButton && menuNewGameButton->hasComponent<UILabel>()) {
                    menuNewGameButton->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            case MENU_LOAD_GAME:
                if (menuLoadGameButton && menuLoadGameButton->hasComponent<UILabel>()) {
                    menuLoadGameButton->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            case MENU_SETTINGS:
                if (menuSettingsButton && menuSettingsButton->hasComponent<UILabel>()) {
                    menuSettingsButton->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            case MENU_LEADERBOARD:
                if (menuLeaderboardButton && menuLeaderboardButton->hasComponent<UILabel>()) {
                    menuLeaderboardButton->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            case MENU_EXIT:
                if (menuExitButton && menuExitButton->hasComponent<UILabel>()) {
                    menuExitButton->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            default:
                break;
        }
    }
}

void Game::renderMainMenu() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);
    
    // Draw menu elements
    menuTitle->draw();
    menuNewGameButton->draw();
    menuLoadGameButton->draw();
    menuSettingsButton->draw();    // Draw settings button
    menuLeaderboardButton->draw(); // Draw leaderboard button
    menuExitButton->draw();
    
    SDL_RenderPresent(renderer);
}

void Game::startGame() {
    // Reset all game state variables for a fresh start
    gameOver = false;
    playerWon = false;
    collectedClues = 0;
    damageTimer = 1.0f;
    objectCollisionDelay = 1.0f;
    objectCollisionsEnabled = false;
    questionActive = false;
    pendingClueEntity = nullptr;
    showFeedback = false;
    showingExitInstructions = false;
    level4MapChanged = false;
    finalBossDefeated = false;
    scientistRescued = false;
    canRescueScientist = false;
    
    // Reset used questions when starting a new game
    resetUsedQuestions();
    
    // Clear previous recorded positions and enable recording
    std::ofstream positionFile("assets/position.txt", std::ios::trunc);
    positionFile.close();
    isRecordingPositions = true;
    lastRecordedPosition = Vector2D(0, 0);
    
    // Change game state
    gameState = STATE_GAME;
    
    // Start gameplay timer
    gameStartTime = SDL_GetTicks();
    gameplayTime = 0;
    
    // Clear menu entities
    menuTitle = nullptr;
    menuNewGameButton = nullptr;
    menuLoadGameButton = nullptr;
    menuSettingsButton = nullptr;
    menuLeaderboardButton = nullptr;
    menuExitButton = nullptr;
    
    // Reset all entity pointers
    player = nullptr;
    finalBoss = nullptr;
    healthbar = nullptr;
    ammobar = nullptr;
    gameover = nullptr;
    clueCounter = nullptr;
    feedbackLabel = nullptr;
    scientist = nullptr;
    questionLabel = nullptr;
    answer1Label = nullptr;
    answer2Label = nullptr;
    answer3Label = nullptr;
    answer4Label = nullptr;
    questionBackground = nullptr;
    
    // Clear all existing entities
    manager.clear();
    
    // Ensure transition label is recreated
    transitionLabel = &manager.addEntity();
    transitionLabel->addComponent<UILabel>(0, 0, "", "font2", white);
    transitionManager.mTransitionLabel = transitionLabel;
    
    // Re-initialize transition manager with new entity manager state
    transitionManager.init(this, &manager);
    
    // Start with level 1
    currentLevel = 1;
    
    // Reset position tracking
    positionManager.resetPositions();
    
    // Load the level and initialize game entities
    loadLevel(currentLevel);
    initEntities();
}

void Game::loadGame() {
    // This is a stub for future implementation
    // For now, it will just start a new game
    // In a real implementation, this would load saved game data
    std::cout << "Load game not implemented yet. Starting new game instead." << std::endl;
    startGame();
}

void Game::initEndScreen(bool victory) {
    // Reset selection and hover states
    selectedEndOption = END_RESTART; // Default selection
    endOptionSelected = false;
    endHighlightActive = false; // Start with no highlights
    
    // Create end screen entity objects
    endTitle = &manager.addEntity();
    endMessage = &manager.addEntity();
    endRestartButton = &manager.addEntity();
    endReplayButton = &manager.addEntity();  // Add replay button
    endMenuButton = &manager.addEntity();
    
    // Set up UI components with appropriate text
    endTitle->addComponent<UILabel>(0, 150, victory ? "VICTORY!" : "GAME OVER", "font2", victory ? green : red);
    
    std::string message = victory ? 
        "You have rescued the scientist and completed your mission!" : 
        "You failed to complete your mission. Better luck next time!";
    endMessage->addComponent<UILabel>(0, 300, message, "font1", white);
    
    endRestartButton->addComponent<UILabel>(0, 450, "RESTART GAME", "font1", white);
    endReplayButton->addComponent<UILabel>(0, 500, "REPLAY (NAJBOLJ NEPOTREBNA FUNKCIJA, KI NE DELA)", "font1", white);  // Add replay button label
    endMenuButton->addComponent<UILabel>(0, 550, "MAIN MENU", "font1", white);  // Adjust position
    
    // Center all elements horizontally
    int titleWidth = endTitle->getComponent<UILabel>().GetWidth();
    int messageWidth = endMessage->getComponent<UILabel>().GetWidth();
    int restartWidth = endRestartButton->getComponent<UILabel>().GetWidth();
    int replayWidth = endReplayButton->getComponent<UILabel>().GetWidth();  // Get replay button width
    int menuWidth = endMenuButton->getComponent<UILabel>().GetWidth();
    
    int titleX = (1920 - titleWidth) / 2;
    int messageX = (1920 - messageWidth) / 2;
    int restartX = (1920 - restartWidth) / 2;
    int replayX = (1920 - replayWidth) / 2;  // Calculate replay button position
    int menuX = (1920 - menuWidth) / 2;
    
    endTitle->getComponent<UILabel>().SetPosition(titleX, 150);
    endMessage->getComponent<UILabel>().SetPosition(messageX, 300);
    endRestartButton->getComponent<UILabel>().SetPosition(restartX, 450);
    endReplayButton->getComponent<UILabel>().SetPosition(replayX, 500);  // Position replay button
    endMenuButton->getComponent<UILabel>().SetPosition(menuX, 550);  // Adjust position
    
    // Make buttons clickable
    endRestartButton->getComponent<UILabel>().SetClickable(true);
    endRestartButton->getComponent<UILabel>().SetOnClick([this]() { 
        // Store gameState before clearing entities
        GameState nextState = STATE_GAME;
        
        // First change all needed variables before destroying entities
        gameOver = false;
        playerWon = false;
        collectedClues = 0;
        damageTimer = 1.0f;
        objectCollisionDelay = 1.0f;
        objectCollisionsEnabled = false;
        questionActive = false;
        pendingClueEntity = nullptr;
        showFeedback = false;
        showingExitInstructions = false; // Reset exit instructions flag
        level4MapChanged = false; // Reset level 4 map state
        finalBossDefeated = false; // Reset final boss state
        scientistRescued = false; // Reset scientist state
        canRescueScientist = false; // Reset scientist interaction flag
        currentLevel = 1;
        
        // Reset used questions when restarting the game
        resetUsedQuestions();
        
        // Set clue count correctly for level 1
        totalClues = 3; // Reset to default value, loadLevel will adjust if needed
        
        // Reset position tracking
        positionManager.resetPositions();
        
        // Reset gameplay timer
        gameStartTime = SDL_GetTicks();
        gameplayTime = 0;
        
        // Use a simple flag to defer cleanup and state change
        needsRestart = true;
    });
    endRestartButton->getComponent<UILabel>().SetHoverColor(yellow);
    endRestartButton->getComponent<UILabel>().ResetHoverState();
    
    // Make replay button clickable
    endReplayButton->getComponent<UILabel>().SetClickable(true);
    endReplayButton->getComponent<UILabel>().SetOnClick([this]() { 
        // Call replay function
        replay();
    });
    endReplayButton->getComponent<UILabel>().SetHoverColor(yellow);
    endReplayButton->getComponent<UILabel>().ResetHoverState();
    
    endMenuButton->getComponent<UILabel>().SetClickable(true);
    endMenuButton->getComponent<UILabel>().SetOnClick([this]() { 
        // Set a flag to return to main menu after this frame
        returnToMainMenu = true;
    });
    endMenuButton->getComponent<UILabel>().SetHoverColor(yellow);
    endMenuButton->getComponent<UILabel>().ResetHoverState();
    
    // Change game state to end screen
    gameState = STATE_END_SCREEN;
}

void Game::updateEndScreen() {
    // Update button states based on selected option
    if (endRestartButton && endRestartButton->hasComponent<UILabel>()) {
        endRestartButton->getComponent<UILabel>().ResetHoverState();
    }
    
    if (endReplayButton && endReplayButton->hasComponent<UILabel>()) {
        endReplayButton->getComponent<UILabel>().ResetHoverState();
    }
    
    if (endMenuButton && endMenuButton->hasComponent<UILabel>()) {
        endMenuButton->getComponent<UILabel>().ResetHoverState();
    }
    
    // Only highlight the selected item if highlight is active
    if (endHighlightActive) {
        switch (selectedEndOption) {
            case END_RESTART:
                if (endRestartButton && endRestartButton->hasComponent<UILabel>()) {
                    endRestartButton->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            case END_REPLAY:
                if (endReplayButton && endReplayButton->hasComponent<UILabel>()) {
                    endReplayButton->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            case END_MAIN_MENU:
                if (endMenuButton && endMenuButton->hasComponent<UILabel>()) {
                    endMenuButton->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            default:
                break;
        }
    }
}

void Game::renderEndScreen() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);
    
    // Draw end screen elements
    endTitle->draw();
    endMessage->draw();
    endRestartButton->draw();
    endReplayButton->draw();
    endMenuButton->draw();
    
    SDL_RenderPresent(renderer);
}

void Game::togglePause() {
    if (gameState == STATE_GAME) {
        // Pause the game
        gameState = STATE_PAUSE;
        
        // Initialize the pause menu
        initPauseMenu();
    } else if (gameState == STATE_PAUSE) {
        // Resume the game
        gameState = STATE_GAME;
        
        // Clean up pause menu entities
        if (pauseTitle) pauseTitle->destroy();
        if (pauseResumeButton) pauseResumeButton->destroy();
        if (pauseSaveButton) pauseSaveButton->destroy();
        if (pauseRestartButton) pauseRestartButton->destroy();
        if (pauseSettingsButton) pauseSettingsButton->destroy();
        if (pauseMainMenuButton) pauseMainMenuButton->destroy();
        
        // Reset pointers
        pauseTitle = nullptr;
        pauseResumeButton = nullptr;
        pauseSaveButton = nullptr;
        pauseRestartButton = nullptr;
        pauseSettingsButton = nullptr;
        pauseMainMenuButton = nullptr;
        
        // Reset state
        selectedPauseItem = PAUSE_RESUME;
        pauseHighlightActive = false;
        pauseItemSelected = false;
        
        // Force the player's KeyboardController to require a mouse release before shooting
        if (player != nullptr && player->hasComponent<KeyboardController>()) {
            KeyboardController& controller = player->getComponent<KeyboardController>();
            controller.requireMouseRelease = true;
            controller.gameStartTime = SDL_GetTicks(); // Reset the cooldown timer
        }
    }
}

void Game::initPauseMenu() {
    // Reset selection and hover states
    selectedPauseItem = PAUSE_RESUME; // Default selection
    pauseItemSelected = false;
    pauseHighlightActive = true; // Start with highlights active
    
    // Create pause menu entity objects
    pauseTitle = &manager.addEntity();
    pauseResumeButton = &manager.addEntity();
    pauseSaveButton = &manager.addEntity();
    pauseRestartButton = &manager.addEntity();
    pauseSettingsButton = &manager.addEntity();
    pauseMainMenuButton = &manager.addEntity();
    
    // Set up UI components with appropriate text
    pauseTitle->addComponent<UILabel>(0, 150, "PAUSED", "font2", white);
    pauseResumeButton->addComponent<UILabel>(0, 300, "RESUME", "font1", white);
    pauseSaveButton->addComponent<UILabel>(0, 360, "SAVE GAME", "font1", white);
    pauseRestartButton->addComponent<UILabel>(0, 420, "RESTART", "font1", white);
    pauseSettingsButton->addComponent<UILabel>(0, 480, "SETTINGS", "font1", white);
    pauseMainMenuButton->addComponent<UILabel>(0, 540, "MAIN MENU", "font1", white);
    
    // Center all elements horizontally
    int titleWidth = pauseTitle->getComponent<UILabel>().GetWidth();
    int resumeWidth = pauseResumeButton->getComponent<UILabel>().GetWidth();
    int saveWidth = pauseSaveButton->getComponent<UILabel>().GetWidth();
    int restartWidth = pauseRestartButton->getComponent<UILabel>().GetWidth();
    int settingsWidth = pauseSettingsButton->getComponent<UILabel>().GetWidth();
    int menuWidth = pauseMainMenuButton->getComponent<UILabel>().GetWidth();
    
    int titleX = (1920 - titleWidth) / 2;
    int resumeX = (1920 - resumeWidth) / 2;
    int saveX = (1920 - saveWidth) / 2;
    int restartX = (1920 - restartWidth) / 2;
    int settingsX = (1920 - settingsWidth) / 2;
    int menuX = (1920 - menuWidth) / 2;
    
    pauseTitle->getComponent<UILabel>().SetPosition(titleX, 150);
    pauseResumeButton->getComponent<UILabel>().SetPosition(resumeX, 300);
    pauseSaveButton->getComponent<UILabel>().SetPosition(saveX, 360);
    pauseRestartButton->getComponent<UILabel>().SetPosition(restartX, 420);
    pauseSettingsButton->getComponent<UILabel>().SetPosition(settingsX, 480);
    pauseMainMenuButton->getComponent<UILabel>().SetPosition(menuX, 540);
    
    // Make buttons clickable
    pauseResumeButton->getComponent<UILabel>().SetClickable(true);
    pauseResumeButton->getComponent<UILabel>().SetOnClick([this]() { togglePause(); });
    pauseResumeButton->getComponent<UILabel>().SetHoverColor(yellow);
    
    pauseSaveButton->getComponent<UILabel>().SetClickable(true);
    pauseSaveButton->getComponent<UILabel>().SetOnClick([this]() { saveGame(); });
    pauseSaveButton->getComponent<UILabel>().SetHoverColor(yellow);
    
    pauseRestartButton->getComponent<UILabel>().SetClickable(true);
    pauseRestartButton->getComponent<UILabel>().SetOnClick([this]() { 
        needsRestart = true;
        gameState = STATE_GAME;
    });
    pauseRestartButton->getComponent<UILabel>().SetHoverColor(yellow);
    
    pauseSettingsButton->getComponent<UILabel>().SetClickable(true);
    pauseSettingsButton->getComponent<UILabel>().SetOnClick([this]() { 
        previousState = STATE_PAUSE; // Store that we came from pause menu
        gameState = STATE_SETTINGS;
        initSettingsMenu();
    });
    pauseSettingsButton->getComponent<UILabel>().SetHoverColor(yellow);
    
    pauseMainMenuButton->getComponent<UILabel>().SetClickable(true);
    pauseMainMenuButton->getComponent<UILabel>().SetOnClick([this]() { returnToMainMenu = true; });
    pauseMainMenuButton->getComponent<UILabel>().SetHoverColor(yellow);
    
    // Update the pause menu to apply initial highlighting
    updatePauseMenu();
}

void Game::updatePauseMenu() {
    // Reset all buttons to default state
    if (pauseResumeButton && pauseResumeButton->hasComponent<UILabel>()) {
        pauseResumeButton->getComponent<UILabel>().ResetHoverState();
    }
    
    if (pauseSaveButton && pauseSaveButton->hasComponent<UILabel>()) {
        pauseSaveButton->getComponent<UILabel>().ResetHoverState();
    }
    
    if (pauseRestartButton && pauseRestartButton->hasComponent<UILabel>()) {
        pauseRestartButton->getComponent<UILabel>().ResetHoverState();
    }
    
    if (pauseSettingsButton && pauseSettingsButton->hasComponent<UILabel>()) {
        pauseSettingsButton->getComponent<UILabel>().ResetHoverState();
    }
    
    if (pauseMainMenuButton && pauseMainMenuButton->hasComponent<UILabel>()) {
        pauseMainMenuButton->getComponent<UILabel>().ResetHoverState();
    }
    
    // Only highlight the selected item if highlight is active
    if (pauseHighlightActive) {
        switch (selectedPauseItem) {
            case PAUSE_RESUME:
                if (pauseResumeButton && pauseResumeButton->hasComponent<UILabel>()) {
                    pauseResumeButton->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            case PAUSE_SAVE:
                if (pauseSaveButton && pauseSaveButton->hasComponent<UILabel>()) {
                    pauseSaveButton->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            case PAUSE_RESTART:
                if (pauseRestartButton && pauseRestartButton->hasComponent<UILabel>()) {
                    pauseRestartButton->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            case PAUSE_SETTINGS:
                if (pauseSettingsButton && pauseSettingsButton->hasComponent<UILabel>()) {
                    pauseSettingsButton->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            case PAUSE_MAIN_MENU:
                if (pauseMainMenuButton && pauseMainMenuButton->hasComponent<UILabel>()) {
                    pauseMainMenuButton->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            default:
                break;
        }
    }
}

void Game::renderPauseMenu() {
    // First render the game in the background
    for(auto& t : *tiles) t->draw();
    for(auto& p : *players) p->draw();
    for(auto& e : *enemies) e->draw();
    for(auto& o : *objects) o->draw();
    for(auto& p : *projectiles) p->draw();
    
    // Ensure scientist is drawn if it exists
    if (scientist != nullptr && scientist->isActive()) {
        scientist->draw();
    }
    
    // Draw normal UI elements in the background
    healthbar->draw();
    ammobar->draw();
    clueCounter->draw();
    timerLabel->draw();
    
    // Add a semi-transparent black overlay for the entire screen
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); // Black with 70% opacity
    SDL_Rect fullscreen = {0, 0, 1920, 1080};
    SDL_RenderFillRect(renderer, &fullscreen);
    
    // Now draw the pause menu elements on top
    if (pauseTitle) pauseTitle->draw();
    if (pauseResumeButton) pauseResumeButton->draw();
    if (pauseSaveButton) pauseSaveButton->draw();
    if (pauseRestartButton) pauseRestartButton->draw();
    if (pauseSettingsButton) pauseSettingsButton->draw();
    if (pauseMainMenuButton) pauseMainMenuButton->draw();
    
    SDL_RenderPresent(renderer);
}

void Game::saveGame() {
    // This is a stub function for now
    // In a real implementation, this would save the game state to a file
    
    std::cout << "Save game feature not implemented yet." << std::endl;
    
    // Show a temporary notification that game was saved
    // Create a notification at the bottom of the screen
    Entity* notification = &manager.addEntity();
    notification->addComponent<UILabel>(0, 600, "Game saved!", "font1", green);
    
    // Center the notification
    int notifyWidth = notification->getComponent<UILabel>().GetWidth();
    int notifyX = (1920 - notifyWidth) / 2;
    notification->getComponent<UILabel>().SetPosition(notifyX, 600);
    
    // Set a timer to destroy the notification after 2 seconds
    // In a real implementation, we'd use a proper timer system
    // For this demo, we'll just leave it there
}

void Game::initSettingsMenu() {
    // Reset selection and hover states
    selectedSettingsItem = SETTINGS_VOLUME;
    settingsItemSelected = false;
    settingsHighlightActive = true;
    draggingVolumeSlider = false;
    
    // Create settings menu entity objects
    settingsTitle = &manager.addEntity();
    volumeSlider = &manager.addEntity();
    volumeLabel = &manager.addEntity();
    keybindsLabel = &manager.addEntity();
    settingsBackButton = &manager.addEntity();
    
    // Set up UI components with appropriate text
    settingsTitle->addComponent<UILabel>(0, 150, "SETTINGS", "font2", white);
    
    // Create volume slider and label
    std::stringstream volSS;
    volSS << "Volume: " << volumeLevel << "%";
    volumeLabel->addComponent<UILabel>(0, 300, volSS.str(), "font1", white);
    
    // Create a fixed-width slider with equal-sized segments
    std::string sliderText = "[";
    int sliderLength = 40;
    int filledAmount = (volumeLevel * sliderLength) / 100;
    
    for (int i = 0; i < sliderLength; i++) {
        if (i < filledAmount) {
            sliderText += "=";
        } else {
            sliderText += " ";
        }
    }
    sliderText += "]";
    
    // Use a monospace font if available, or a fixed-width representation
    volumeSlider->addComponent<UILabel>(0, 340, sliderText, "font1", white);
    
    // Keybinds section - just the title
    keybindsLabel->addComponent<UILabel>(0, 400, "CONTROLS", "font1", white);
    
    // Back button - changing Y position from 720 to an even lower position
    settingsBackButton->addComponent<UILabel>(0, 800, "BACK", "font1", white);
    
    // Center all elements horizontally
    int titleWidth = settingsTitle->getComponent<UILabel>().GetWidth();
    int volLabelWidth = volumeLabel->getComponent<UILabel>().GetWidth();
    int sliderWidth = volumeSlider->getComponent<UILabel>().GetWidth();
    int keybindsWidth = keybindsLabel->getComponent<UILabel>().GetWidth();
    int backWidth = settingsBackButton->getComponent<UILabel>().GetWidth();
    
    int titleX = (1920 - titleWidth) / 2;
    int volLabelX = (1920 - volLabelWidth) / 2;
    int sliderX = (1920 - sliderWidth) / 2;
    int keybindsX = (1920 - keybindsWidth) / 2;
    int backX = (1920 - backWidth) / 2;
    
    settingsTitle->getComponent<UILabel>().SetPosition(titleX, 150);
    volumeLabel->getComponent<UILabel>().SetPosition(volLabelX, 300);
    volumeSlider->getComponent<UILabel>().SetPosition(sliderX, 340);
    keybindsLabel->getComponent<UILabel>().SetPosition(keybindsX, 400);
    settingsBackButton->getComponent<UILabel>().SetPosition(backX, 800);
    
    // Make back button clickable - return to correct previous state
    settingsBackButton->getComponent<UILabel>().SetClickable(true);
    settingsBackButton->getComponent<UILabel>().SetOnClick([this]() { 
        gameState = previousState; // Return to previous state (main menu or pause)
        applySettings();
    });
    settingsBackButton->getComponent<UILabel>().SetHoverColor(yellow);
    
    // Make slider clickable
    volumeSlider->getComponent<UILabel>().SetClickable(true);
    volumeSlider->getComponent<UILabel>().SetHoverColor(yellow);
    
    // Update the settings menu to apply initial highlighting
    updateSettingsMenu();
}

void Game::updateSettingsMenu() {
    // Update volume label based on current volume level
    if (volumeLabel && volumeLabel->hasComponent<UILabel>()) {
        std::stringstream volSS;
        volSS << "Volume: " << volumeLevel << "%";
        volumeLabel->getComponent<UILabel>().SetLabelText(volSS.str(), "font1");
        
        // Center the updated label
        int volLabelWidth = volumeLabel->getComponent<UILabel>().GetWidth();
        int volLabelX = (1920 - volLabelWidth) / 2;
        volumeLabel->getComponent<UILabel>().SetPosition(volLabelX, 300);
    }
    
    // Update slider visual - recreate the entire slider to maintain fixed width
    if (volumeSlider && volumeSlider->hasComponent<UILabel>()) {
        std::string sliderText = "[";
        int sliderLength = 40;
        int filledAmount = (volumeLevel * sliderLength) / 100;
        
        for (int i = 0; i < sliderLength; i++) {
            if (i < filledAmount) {
                sliderText += "=";
            } else {
                sliderText += " ";
            }
        }
        sliderText += "]";
        
        // Update slider text but maintain position
        volumeSlider->getComponent<UILabel>().SetLabelText(sliderText, "font1");
        
        // Re-center the slider 
        int sliderWidth = volumeSlider->getComponent<UILabel>().GetWidth();
        int sliderX = (1920 - sliderWidth) / 2;
        volumeSlider->getComponent<UILabel>().SetPosition(sliderX, 340);
    }
    
    // Reset all items to default state
    if (volumeSlider && volumeSlider->hasComponent<UILabel>()) {
        volumeSlider->getComponent<UILabel>().ResetHoverState();
    }
    
    if (settingsBackButton && settingsBackButton->hasComponent<UILabel>()) {
        settingsBackButton->getComponent<UILabel>().ResetHoverState();
    }
    
    // Only highlight the selected item if highlight is active
    if (settingsHighlightActive) {
        switch (selectedSettingsItem) {
            case SETTINGS_VOLUME:
                if (volumeSlider && volumeSlider->hasComponent<UILabel>()) {
                    volumeSlider->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            case SETTINGS_BACK:
                if (settingsBackButton && settingsBackButton->hasComponent<UILabel>()) {
                    settingsBackButton->getComponent<UILabel>().SetTextColor(yellow);
                }
                break;
            default:
                break;
        }
    }
}

void Game::renderSettingsMenu() {
    // Determine the background based on where we came from
    if (previousState == STATE_PAUSE) {
        // When coming from pause menu, draw the game in the background
        for(auto& t : *tiles) t->draw();
        for(auto& p : *players) p->draw();
        for(auto& e : *enemies) e->draw();
        for(auto& o : *objects) o->draw();
        for(auto& p : *projectiles) p->draw();
        
        // Ensure scientist is drawn if it exists
        if (scientist != nullptr && scientist->isActive()) {
            scientist->draw();
        }
        
        // Draw normal UI elements in the background
        healthbar->draw();
        ammobar->draw();
        clueCounter->draw();
        timerLabel->draw();
    } else {
        // When coming from main menu, just draw a black background
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(renderer);
    }
    
    // Add a semi-transparent black overlay for the entire screen
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); // Black with 70% opacity
    SDL_Rect fullscreen = {0, 0, 1920, 1080};
    SDL_RenderFillRect(renderer, &fullscreen);
    
    // Now draw the settings menu elements on top
    if (settingsTitle) settingsTitle->draw();
    if (volumeLabel) volumeLabel->draw();
    
    // Draw a graphical volume slider instead of text-based one
    int screenCenter = 1920 / 2;
    int sliderY = 340;
    int sliderWidth = 400;
    int sliderHeight = 20;
    int sliderX = screenCenter - (sliderWidth / 2);
    
    // Draw slider background
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255); // Dark gray
    SDL_Rect sliderBg = {sliderX, sliderY, sliderWidth, sliderHeight};
    SDL_RenderFillRect(renderer, &sliderBg);
    
    // Draw filled portion based on volume
    int filledWidth = (volumeLevel * sliderWidth) / 100;
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Light gray
    SDL_Rect filledRect = {sliderX, sliderY, filledWidth, sliderHeight};
    SDL_RenderFillRect(renderer, &filledRect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
    SDL_RenderDrawRect(renderer, &sliderBg);
    
    // Draw slider knob/handle
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
    int knobX = sliderX + filledWidth - 4;
    SDL_Rect knob = {knobX, sliderY - 5, 8, sliderHeight + 10};
    SDL_RenderFillRect(renderer, &knob);
    
    // Highlight slider if selected
    if (settingsHighlightActive && selectedSettingsItem == SETTINGS_VOLUME) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow
        SDL_Rect highlightRect = {sliderX - 2, sliderY - 2, sliderWidth + 4, sliderHeight + 4};
        SDL_RenderDrawRect(renderer, &highlightRect);
    }
    
    // Draw keybinds manually with increased spacing
    if (keybindsLabel) {
        keybindsLabel->draw();
        
        // Manually draw each keybind entry
        TTF_Font* font = assets->GetFont("font1");
        if (font) {
            int yOffset = 450; // Increased starting position
            int screenCenter = 1920 / 2;
            
            for (const auto& keybind : keybinds) {
                std::string bindText = keybind.action + ": " + keybind.key;
                
                // Render text
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, bindText.c_str(), white);
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    if (textTexture) {
                        // Center the text
                        int textWidth = textSurface->w;
                        int textHeight = textSurface->h;
                        int xPos = screenCenter - (textWidth / 2);
                        
                        SDL_Rect destRect = {xPos, yOffset, textWidth, textHeight};
                        SDL_RenderCopy(renderer, textTexture, NULL, &destRect);
                        
                        SDL_DestroyTexture(textTexture);
                    }
                    SDL_FreeSurface(textSurface);
                }
                
                yOffset += 45; // Even more spacing between controls (was 40)
            }
        }
    }
    
    // Draw the back button
    if (settingsBackButton) settingsBackButton->draw();
    
    SDL_RenderPresent(renderer);
}

void Game::applySettings() {
    // Here you would typically apply the settings to the game
    // For this demo, we'll just clean up the settings menu entities
    
    // Clean up settings menu entities
    if (settingsTitle) settingsTitle->destroy();
    if (volumeSlider) volumeSlider->destroy();
    if (volumeLabel) volumeLabel->destroy();
    if (keybindsLabel) keybindsLabel->destroy();
    if (settingsBackButton) settingsBackButton->destroy();
    
    // Reset pointers
    settingsTitle = nullptr;
    volumeSlider = nullptr;
    volumeLabel = nullptr;
    keybindsLabel = nullptr;
    settingsBackButton = nullptr;
    
    // Reset state
    selectedSettingsItem = SETTINGS_VOLUME;
    settingsHighlightActive = false;
    settingsItemSelected = false;
    draggingVolumeSlider = false;
}

// Add this at the end of the file

void Game::replay() {
    // Reset game state for replay
    gameState = STATE_REPLAY;
    
    // Always start replay from level 1 regardless of current level
    readAllPositionsFromFile();
}

void Game::readAllPositionsFromFile() {
    allReplayPositionsByLevel.clear(); 
    replayPositionIndex = 0;           
    currentReplayLevel = 1;            

    std::ifstream posFile("assets/position.txt");
    if (!posFile.is_open()) {
        std::cerr << "Failed to open position file for replay!" << std::endl;
        isReplaying = false; 
        return;
    }

    std::string line;
    int minLevelFound = -1; 

    while (std::getline(posFile, line)) {
        std::stringstream ss(line);
        std::string xStr, yStr, lvlStr;
        if (std::getline(ss, xStr, ',') && std::getline(ss, yStr, ',') && std::getline(ss, lvlStr, ',')) {
            try {
                int x = std::stoi(xStr); int y = std::stoi(yStr); int lvl = std::stoi(lvlStr);
                allReplayPositionsByLevel[lvl].push_back(Vector2D(x, y));
                if (minLevelFound == -1 || lvl < minLevelFound) { minLevelFound = lvl; }
            } catch (const std::exception&) { /* Skip */ }
        } else {
             std::stringstream oldFormat(line); int x, y, lvl;
             if (oldFormat >> x >> y >> lvl) {
                 allReplayPositionsByLevel[lvl].push_back(Vector2D(x, y));
                 if (minLevelFound == -1 || lvl < minLevelFound) { minLevelFound = lvl; }
             }
        }
    }
    posFile.close();

    if (allReplayPositionsByLevel.empty()) {
        std::cerr << "No valid positions found in position file." << std::endl;
        isReplaying = false; return;
    }

    if (minLevelFound != -1 && allReplayPositionsByLevel.count(minLevelFound) > 0 && !allReplayPositionsByLevel[minLevelFound].empty()) {
         currentReplayLevel = minLevelFound;
    } else {
        for (const auto& pair : allReplayPositionsByLevel) { if (!pair.second.empty()) { currentReplayLevel = pair.first; break; } }
        if (allReplayPositionsByLevel[currentReplayLevel].empty()) {
             std::cerr << "No level with valid positions found to start replay." << std::endl;
             isReplaying = false; return;
        }
    }

    isReplaying = true;
    lastReplayFrameTime = SDL_GetTicks();
    gameState = STATE_REPLAY; 
    loadLevel(currentReplayLevel);
    Vector2D startPos = allReplayPositionsByLevel[currentReplayLevel][0];
    // Removed debug print

    if (replayEntity) { replayEntity->destroy(); replayEntity = nullptr; }
    replayEntity = &manager.addEntity();
    replayEntity->addComponent<TransformComponent>(startPos.x, startPos.y, 32, 32, 3);
    replayEntity->addComponent<SpriteComponent>("player", true);
    replayEntity->addGroup(Game::groupPlayers);
    // Removed debug print

    if (timerLabel) { timerLabel->destroy(); timerLabel = nullptr; }
    timerLabel = &manager.addEntity();
    timerLabel->addComponent<UILabel>(20, 20, " ", "font1", white); 
    timerLabel->addGroup(Game::groupUI);
    replayEntity->getComponent<SpriteComponent>().Play("Idle");
}

void Game::updateReplay() {
    if (!isReplaying || !replayEntity || !replayEntity->hasComponent<TransformComponent>()) return;
    // Removed debug print

    if (replayEntity->hasComponent<SpriteComponent>()) { replayEntity->getComponent<SpriteComponent>().update(); }
    Uint32 currentTime = SDL_GetTicks();
    if (replayEntity) { /* Camera logic ... */ }

    if (allReplayPositionsByLevel.find(currentReplayLevel) == allReplayPositionsByLevel.end() || allReplayPositionsByLevel[currentReplayLevel].empty()) { /* End replay logic ... */ return; }
    const auto& currentLevelPositions = allReplayPositionsByLevel[currentReplayLevel];
    if (timerLabel) { /* UI update logic ... */ }

    if (currentTime - lastReplayFrameTime >= replayFrameTime) {
        if (replayPositionIndex >= currentLevelPositions.size()) { /* Level transition logic ... */ return; }

        auto& transform = replayEntity->getComponent<TransformComponent>();
        float currentX = transform.position.x; float currentY = transform.position.y;
        Vector2D nextPos = currentLevelPositions[replayPositionIndex];
        float deltaX = nextPos.x - currentX; float deltaY = nextPos.y - currentY;

        transform.position.x = nextPos.x; transform.position.y = nextPos.y;
        transform.velocity.x = 0; transform.velocity.y = 0;

        bool isFinalPositionInLevel = (replayPositionIndex == currentLevelPositions.size() - 1);
        if (isFinalPositionInLevel) { /* Idle animation logic ... */ }
        else { /* Movement animation logic ... */ }

        replayPositionIndex++;
        lastReplayFrameTime = currentTime;
    }
}

void Game::renderReplay() {
    SDL_RenderClear(renderer);
    for(auto& t : *tiles) t->draw();

    if (replayEntity && replayEntity->hasComponent<TransformComponent>()) {
        // Camera is updated in updateReplay now, just use it
        // Render the replay entity
        replayEntity->draw();
    }

    if (timerLabel) {
        // Position is updated in updateReplay
        timerLabel->draw();
    }

    SDL_RenderPresent(renderer);
}

void Game::recordPlayerPosition() {
    // Only record positions if a player exists and recording is enabled
    if (player && isRecordingPositions && gameState == STATE_GAME) {
        // Get current player position
        Vector2D currentPosition = player->getComponent<TransformComponent>().position;
        
        // Record if position has changed (more sensitive threshold)
        const float recordThreshold = 0.1f;
        if (std::abs(currentPosition.x - lastRecordedPosition.x) >= recordThreshold || 
            std::abs(currentPosition.y - lastRecordedPosition.y) >= recordThreshold) {
            
            // Open file in append mode
            std::ofstream positionFile("assets/position.txt", std::ios::app);
            
            if (positionFile.is_open()) {
                // Format: x,y,level with commas as separators for clarity
                positionFile << static_cast<int>(currentPosition.x) << "," 
                             << static_cast<int>(currentPosition.y) << "," 
                             << currentLevel << std::endl;
                
                // Update last recorded position
                lastRecordedPosition = currentPosition;
            }
            positionFile.close();
        }
    }
}
