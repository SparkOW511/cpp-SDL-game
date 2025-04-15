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
bool Game::level4MapChanged = false; // Initialize level4MapChanged
bool Game::finalBossDefeated = false; // Initialize finalBossDefeated
bool Game::scientistRescued = false; // Initialize scientistRescued
bool Game::canRescueScientist = false; // Initialize canRescueScientist
GameState Game::gameState = STATE_MAIN_MENU; // Start in main menu

// Initialize timer variables
Uint32 Game::gameStartTime = 0;
Uint32 Game::gameplayTime = 0;
Entity* Game::timerLabel = nullptr;

// Main menu variables
Entity* menuTitle = nullptr;
Entity* menuNewGameButton = nullptr;
Entity* menuLoadGameButton = nullptr;
Entity* menuExitButton = nullptr;
int selectedMenuItem = MENU_NEW_GAME;
bool menuHighlightActive = false;
bool menuItemSelected = false;

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
        {"What is the value of π (pi) rounded to two decimal places?", {"3.41", "3.14", "3.50", "3.16"}, 1},
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
    player->addComponent<ColliderComponent>("player");
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
        
        std::cout << "Scientist created at position: " << scientistPos.x/64 << "," << scientistPos.y/64 << std::endl;
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
                    case SDLK_ESCAPE:
                        closeQuestion();
                        break;
                    default:
                        break;
                }
            }
            else if (gameState == STATE_GAME) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    // Return to main menu
                    gameState = STATE_MAIN_MENU;
                    
                    // Clean up game entities
                    if (map != nullptr) {
                        delete map;
                        map = nullptr;
                    }
                    
                    // Clear all entities
                    manager.clear();
                    
                    // Initialize main menu
                    initMainMenu();
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
                        
                        // Show game completion screen
                        gameover->getComponent<UILabel>().SetLabelText("MISSION COMPLETE! Scientist rescued! Press R to restart or ESC to exit", "font2");
                        
                        // Center the win text
                        int textWidth = gameover->getComponent<UILabel>().GetWidth();
                        int textHeight = gameover->getComponent<UILabel>().GetHeight();
                        
                        int xPos = (1920 - textWidth) / 2;
                        int yPos = (1080 - textHeight) / 2;
                        
                        gameover->getComponent<UILabel>().SetPosition(xPos, yPos);
                        
                        // Hide UI elements except game over message
                        healthbar->getComponent<UILabel>().SetLabelText("", "font1");
                        ammobar->getComponent<UILabel>().SetLabelText("", "font1");
                        clueCounter->getComponent<UILabel>().SetLabelText("", "font1");
                        
                        // Reset all enemy animations to idle
                        for(auto& e : *enemies) {
                            if(e->hasComponent<SpriteComponent>()) {
                                e->getComponent<SpriteComponent>().Play("Idle");
                            }
                        }
                        
                        gameOver = true;
                        playerWon = true;
                    }
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
                    
                    if (menuExitButton && menuExitButton->hasComponent<UILabel>()) {
                        menuExitButton->getComponent<UILabel>().HandleEvent(event);
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
                
                if (menuExitButton && menuExitButton->hasComponent<UILabel>()) {
                    menuExitButton->getComponent<UILabel>().HandleEvent(event);
                }
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
    
    // Handle updates based on current game state
    switch (gameState) {
        case STATE_MAIN_MENU:
            // Nothing to update in the main menu except UI
            break;
            
        case STATE_GAME:
            // Update gameplay timer
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
            
            // Always refresh entity manager for proper lifecycle management
            manager.refresh();
            
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
    
    // Continue with regular game update logic - only if player exists
    if (player != nullptr && player->isActive() && gameState == STATE_GAME) {
        Vector2D playerPos = player->getComponent<TransformComponent>().position;
        
        manager.update();
        
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
            
            // Check if collision happened
            if(Collision::AABB(cCol, playerCol)) {
                // Simply restore player to previous position before movement
                player->getComponent<TransformComponent>().position = playerPos;
                // Update collider position
                player->getComponent<ColliderComponent>().update();
            }
        }

        // Enemy collision with projectiles and player
        for(auto& e : *enemies) {
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
                    feedbackLabel->getComponent<UILabel>().SetLabelText("BOSS DEFEATED! The path is revealed! Find and rescue the scientist!", "font1", {255, 215, 0, 255});
                    
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
            // Show game over message
            gameover->getComponent<UILabel>().SetLabelText("GAME OVER! Press R to restart or ESC to exit", "font2");
            
            // Center the game over text
            int textWidth = gameover->getComponent<UILabel>().GetWidth();
            int textHeight = gameover->getComponent<UILabel>().GetHeight();
            
            int xPos = (1920 - textWidth) / 2;
            int yPos = (1080 - textHeight) / 2;
            
            gameover->getComponent<UILabel>().SetPosition(xPos, yPos);
            
            // Hide UI elements except game over message
            healthbar->getComponent<UILabel>().SetLabelText("", "font1");
            ammobar->getComponent<UILabel>().SetLabelText("", "font1");
            clueCounter->getComponent<UILabel>().SetLabelText("", "font1");
            
            // Reset all enemy animations to idle
            for(auto& e : *enemies) {
                if(e->hasComponent<SpriteComponent>()) {
                    e->getComponent<SpriteComponent>().Play("Idle");
                }
            }
            
            player->destroy();
            gameOver = true;
            playerWon = false;
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
                    if (currentLevel < maxLevels) {
                        // Advance to next level
                        advanceToNextLevel();
                    } else {
                        // Show final win message if all levels completed
                        gameover->getComponent<UILabel>().SetLabelText("YOU WIN! Press R to restart or ESC to exit", "font2");
                        
                        // Center the win text
                        int textWidth = gameover->getComponent<UILabel>().GetWidth();
                        int textHeight = gameover->getComponent<UILabel>().GetHeight();
                        
                        int xPos = (1920 - textWidth) / 2;
                        int yPos = (1080 - textHeight) / 2;
                        
                        gameover->getComponent<UILabel>().SetPosition(xPos, yPos);
                        
                        // Hide UI elements except game over message
                        healthbar->getComponent<UILabel>().SetLabelText("", "font1");
                        ammobar->getComponent<UILabel>().SetLabelText("", "font1");
                        clueCounter->getComponent<UILabel>().SetLabelText("", "font1");
                        
                        // Reset all enemy animations to idle
                        for(auto& e : *enemies) {
                            if(e->hasComponent<SpriteComponent>()) {
                                e->getComponent<SpriteComponent>().Play("Idle");
                            }
                        }
                        
                        player->destroy();
                        gameOver = true;
                        playerWon = true;
                    }
                }
            } else if (currentLevel == 4 && level4MapChanged) {
                // For level 4, check if player has reached the new exit at a specific position
                float playerX = player->getComponent<TransformComponent>().position.x / 64;
                float playerY = player->getComponent<TransformComponent>().position.y / 64;
                
                // Check if player is at the new exit (adjust coordinates as needed)
                if ((playerX >= 28 && playerX <= 32) && (playerY >= 15 && playerY <= 17)) {
                    // Player has reached the secret exit, show final win message
                    gameover->getComponent<UILabel>().SetLabelText("CONGRATULATIONS! You found the secret exit! Press R to restart or ESC to exit", "font2");
                    
                    // Center the win text
                    int textWidth = gameover->getComponent<UILabel>().GetWidth();
                    int textHeight = gameover->getComponent<UILabel>().GetHeight();
                    
                    int xPos = (1920 - textWidth) / 2;
                    int yPos = (1080 - textHeight) / 2;
                    
                    gameover->getComponent<UILabel>().SetPosition(xPos, yPos);
                    
                    // Hide UI elements except game over message
                    healthbar->getComponent<UILabel>().SetLabelText("", "font1");
                    ammobar->getComponent<UILabel>().SetLabelText("", "font1");
                    clueCounter->getComponent<UILabel>().SetLabelText("", "font1");
                    
                    // Reset all enemy animations to idle
                    for(auto& e : *enemies) {
                        if(e->hasComponent<SpriteComponent>()) {
                            e->getComponent<SpriteComponent>().Play("Idle");
                        }
                    }
                    
                    player->destroy();
                    gameOver = true;
                    playerWon = true;
                }
            } else {
                // For other levels, check if player has gone far enough north
                if (player->getComponent<TransformComponent>().position.y < 100) {
                    // Player has gone north enough, proceed to next level
                    if (currentLevel < maxLevels) {
                        // Advance to next level
                        advanceToNextLevel();
                    } else {
                        // Show final win message if all levels completed
                        gameover->getComponent<UILabel>().SetLabelText("YOU WIN! Press R to restart or ESC to exit", "font2");
                        
                        // Center the win text
                        int textWidth = gameover->getComponent<UILabel>().GetWidth();
                        int textHeight = gameover->getComponent<UILabel>().GetHeight();
                        
                        int xPos = (1920 - textWidth) / 2;
                        int yPos = (1080 - textHeight) / 2;
                        
                        gameover->getComponent<UILabel>().SetPosition(xPos, yPos);
                        
                        // Hide UI elements except game over message
                        healthbar->getComponent<UILabel>().SetLabelText("", "font1");
                        ammobar->getComponent<UILabel>().SetLabelText("", "font1");
                        clueCounter->getComponent<UILabel>().SetLabelText("", "font1");
                        
                        // Reset all enemy animations to idle
                        for(auto& e : *enemies) {
                            if(e->hasComponent<SpriteComponent>()) {
                                e->getComponent<SpriteComponent>().Play("Idle");
                            }
                        }
                        
                        player->destroy();
                        gameOver = true;
                        playerWon = true;
                    }
                }
            }
        }
            }
            break;
            
        default:
            break;
    }
}

void Game::render()
{
    // Render based on current game state
    switch (gameState) {
        case STATE_MAIN_MENU:
            renderMainMenu();
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
            // Draw interaction prompt
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            
            // Calculate prompt position (above scientist)
            int promptX = static_cast<int>(scientistPos.x) - Game::camera.x + 16;
            int promptY = static_cast<int>(scientistPos.y) - Game::camera.y - 30;
            
            // Create temporary label for prompt
            SDL_Rect promptRect = {promptX - 50, promptY - 15, 100, 30};
            SDL_RenderFillRect(renderer, &promptRect);
            
            // Set up temporary label for "Press E"
            static Entity* promptLabel = nullptr;
            if (promptLabel == nullptr) {
                promptLabel = &manager.addEntity();
                promptLabel->addComponent<UILabel>(0, 0, "Press E", "font1", white);
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
    menuExitButton = &manager.addEntity();
    
    // Set up UI components
    menuTitle->addComponent<UILabel>(0, 200, "Dejte mi RPA 3 prosm", "font2", white);
    menuNewGameButton->addComponent<UILabel>(0, 400, "NEW GAME", "font1", white);
    menuLoadGameButton->addComponent<UILabel>(0, 480, "LOAD GAME", "font1", white);
    menuExitButton->addComponent<UILabel>(0, 560, "EXIT", "font1", white);
    
    // Center the menu items horizontally
    int titleWidth = menuTitle->getComponent<UILabel>().GetWidth();
    int newGameWidth = menuNewGameButton->getComponent<UILabel>().GetWidth();
    int loadGameWidth = menuLoadGameButton->getComponent<UILabel>().GetWidth();
    int exitWidth = menuExitButton->getComponent<UILabel>().GetWidth();
    
    int titleX = (1920 - titleWidth) / 2;
    int newGameX = (1920 - newGameWidth) / 2;
    int loadGameX = (1920 - loadGameWidth) / 2;
    int exitX = (1920 - exitWidth) / 2;
    
    menuTitle->getComponent<UILabel>().SetPosition(titleX, 200);
    menuNewGameButton->getComponent<UILabel>().SetPosition(newGameX, 400);
    menuLoadGameButton->getComponent<UILabel>().SetPosition(loadGameX, 480);
    menuExitButton->getComponent<UILabel>().SetPosition(exitX, 560);
    
    // Make menu items clickable but ensure they're not hovered initially
    menuNewGameButton->getComponent<UILabel>().SetClickable(true);
    menuNewGameButton->getComponent<UILabel>().SetOnClick([this]() { startGame(); });
    menuNewGameButton->getComponent<UILabel>().SetHoverColor(yellow);
    menuNewGameButton->getComponent<UILabel>().ResetHoverState();
    
    menuLoadGameButton->getComponent<UILabel>().SetClickable(true);
    menuLoadGameButton->getComponent<UILabel>().SetOnClick([this]() { loadGame(); });
    menuLoadGameButton->getComponent<UILabel>().SetHoverColor(yellow);
    menuLoadGameButton->getComponent<UILabel>().ResetHoverState();
    
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
    menuExitButton->draw();
    
    SDL_RenderPresent(renderer);
}

void Game::startGame() {
    // Change game state
    gameState = STATE_GAME;
    
    // Start gameplay timer
    gameStartTime = SDL_GetTicks();
    gameplayTime = 0;
    
    // Clear menu entities
    menuTitle = nullptr;
    menuNewGameButton = nullptr;
    menuLoadGameButton = nullptr;
    menuExitButton = nullptr;
    
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