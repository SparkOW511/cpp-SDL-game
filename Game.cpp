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
#include <vector>
#include <set>
#include <string>
#include <utility>

Map* map;
Manager manager;

Entity* player = nullptr;
Entity* finalBoss = nullptr;
Entity* healthbar = nullptr;
Entity* ammobar = nullptr;
Entity* gameover = nullptr;
Entity* clueCounter = nullptr;
Entity* feedbackLabel = nullptr;
Entity* scientist = nullptr;

std::vector<Entity*>* tiles;
std::vector<Entity*>* players;
std::vector<Entity*>* enemies;
std::vector<Entity*>* colliders;
std::vector<Entity*>* projectiles;
std::vector<Entity*>* objects;
std::vector<Entity*>* ui;

bool answerSubmitted = false;

float damageTimer = 1.0f;
const float damageCooldown = 0.3f;
float objectCollisionDelay = 1.0f;
bool objectCollisionsEnabled = false;

SDL_Renderer *Game::renderer = nullptr;
SDL_Event Game::event;
SDL_Rect Game::camera = {0, 0, 1920, 1080};
bool Game::isRunning = false;
AssetManager* Game::assets = nullptr;
int Game::totalClues = 3;
int Game::totalMagazines = 3;
int Game::totalHealthPotions = 2;
int Game::collectedClues = 0;
bool Game::gameOver = false;
bool Game::playerWon = false;
bool Game::questionActive = false;
Entity* Game::pendingClueEntity = nullptr;
bool Game::showFeedback = false;
Uint32 Game::feedbackStartTime = 0;
Entity* Game::feedbackLabel = nullptr;
int Game::currentLevel = 1;
int Game::maxLevels = 4;
bool Game::showingExitInstructions = false;
GameState Game::gameState = STATE_MAIN_MENU;
bool Game::level4MapChanged = false;
bool Game::finalBossDefeated = false;
bool Game::bossMusicPlaying = false;
bool Game::scientistRescued = false;
bool Game::canRescueScientist = false;
bool Game::returnToMainMenu = false;
Uint32 Game::gameStartTime = 0;
Uint32 Game::gameplayTime = 0;
Entity* Game::timerLabel = nullptr;
int Game::volumeLevel = 75;

std::string Game::currentMusic = "";

bool Game::isRecordingPositions = false;
bool Game::isReplaying = false;
int Game::replayPositionIndex = 0;
Vector2D Game::lastRecordedPosition = Vector2D(0, 0);
Uint32 Game::replayFrameTime = 20;
Uint32 Game::lastReplayFrameTime = 0;

Entity* menuTitle = nullptr;
Entity* menuNewGameButton = nullptr;
Entity* menuLoadGameButton = nullptr;
Entity* menuSettingsButton = nullptr;
Entity* menuLeaderboardButton = nullptr;
Entity* menuExitButton = nullptr;
int selectedMenuItem = MENU_NEW_GAME;
bool menuHighlightActive = false;
bool menuItemSelected = false;

Entity* endTitle = nullptr;
Entity* endMessage = nullptr;
Entity* endRestartButton = nullptr;
Entity* endReplayButton = nullptr;
Entity* endMenuButton = nullptr;
int selectedEndOption = END_RESTART;
bool endOptionSelected = false;
bool endHighlightActive = false;

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

Entity* settingsTitle = nullptr;
Entity* volumeLabel = nullptr;
Entity* keybindsLabel = nullptr;
Entity* settingsBackButton = nullptr;
Entity* settingsBackground = nullptr;
int selectedSettingsItem = SETTINGS_VOLUME;
bool settingsHighlightActive = false;
bool settingsItemSelected = false;
bool draggingVolumeSlider = false;

bool Game::hasSavedDuringExitInstructions = false;
std::string Game::savedExitInstructionsText = "";

void writeString(std::ofstream& file, const std::string& str) {
    size_t len = str.length();
    file.write(reinterpret_cast<const char*>(&len), sizeof(len));
    file.write(str.c_str(), len);
}

bool readString(std::ifstream& file, std::string& str) {
    size_t len;
    file.read(reinterpret_cast<char*>(&len), sizeof(len));
    if (!file || file.gcount() != sizeof(len)) return false;

    if (len > 1024) {
        return false;
    }

    str.resize(len);
    if (str.length() != len) {
         return false;
    }

    if (len > 0) {
        file.read(&str[0], len);
        if (!file || file.gcount() != len) return false;
    }
    return true;
}

struct EnemySaveData {
    float x, y;
    int health;
    bool isBoss;
};

struct ObjectSaveData {
    float x, y;
    std::string type;
};

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
        {"What does the ^ operator typically represent in programming?", {"Multiplication", "Division", "Exponentiation", "Bitwise XOR"}, 3},
        
        {"Which planet is known as the Red Planet?", {"Venus", "Mars", "Jupiter", "Saturn"}, 1},
        {"What is the chemical symbol for gold?", {"Go", "Gl", "Au", "Ag"}, 2},
        {"What is the hardest natural substance on Earth?", {"Diamond", "Titanium", "Platinum", "Steel"}, 0},
        {"Which of these is NOT a state of matter?", {"Solid", "Liquid", "Gas", "Energy"}, 3},
        {"What is the largest organ in the human body?", {"Heart", "Brain", "Liver", "Skin"}, 3},
        {"Which of these animals is NOT a mammal?", {"Bat", "Whale", "Penguin", "Elephant"}, 2},
        {"What is the smallest unit of life?", {"Atom", "Cell", "Molecule", "Tissue"}, 1},
        {"What is the process by which plants make their own food?", {"Respiration", "Photosynthesis", "Digestion", "Fermentation"}, 1},
        {"What is the speed of light approximately?", {"300,000 km/s", "150,000 km/s", "500,000 km/s", "1,000,000 km/s"}, 0},
        {"Which of these is NOT one of Newton's Laws of Motion?", {"Law of Inertia", "Law of Acceleration", "Law of Conservation of Energy", "Law of Action and Reaction"}, 2},
        
        {"In which year did World War II end?", {"1943", "1945", "1947", "1950"}, 1},
        {"Who painted the Mona Lisa?", {"Vincent van Gogh", "Pablo Picasso", "Leonardo da Vinci", "Michelangelo"}, 2},
        {"Which ancient civilization built the pyramids of Giza?", {"Greeks", "Romans", "Egyptians", "Mayans"}, 2},
        {"Who was the first person to step on the moon?", {"Buzz Aldrin", "Yuri Gagarin", "Neil Armstrong", "John Glenn"}, 2},
        {"Which country was NOT part of the Allied Powers in World War II?", {"United States", "Soviet Union", "Italy", "United Kingdom"}, 2},
        {"In which century did the Renaissance begin?", {"13th century", "14th century", "15th century", "16th century"}, 1},
        {"Who wrote 'Romeo and Juliet'?", {"Charles Dickens", "Jane Austen", "William Shakespeare", "Mark Twain"}, 2},
        {"Which ancient empire was ruled by Julius Caesar?", {"Greek", "Persian", "Roman", "Ottoman"}, 2},
        {"Which invention allowed books to be mass-produced for the first time?", {"Typewriter", "Printing Press", "Mechanical Loom", "Telegraph"}, 1},
        {"Which civilization built Machu Picchu?", {"Aztec", "Maya", "Olmec", "Inca"}, 3}
    };
}

Game::~Game()
{
    delete assets;
}

void Game::initEntities() {
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
    
    tiles = &manager.getGroup(Game::groupMap);
    players = &manager.getGroup(Game::groupPlayers);
    enemies = &manager.getGroup(Game::groupEnemies);
    colliders = &manager.getGroup(Game::groupColliders);
    projectiles = &manager.getGroup(Game::groupProjectiles);
    objects = &manager.getGroup(Game::groupObjects);
    ui = &manager.getGroup(Game::groupUI);
    
    for (int i = 0; i < totalClues; i++) {
        Vector2D cluePos = positionManager.findRandomCluePosition(currentLevel);
        assets->CreateObject(cluePos.x, cluePos.y, "clue");
    }
    
    for (int i = 0; i < totalMagazines; i++) {
        Vector2D magazinePos = positionManager.findRandomMagazinePosition();
        assets->CreateObject(magazinePos.x, magazinePos.y, "magazine");
    }
    
    for (int i = 0; i < totalHealthPotions; i++) {
        Vector2D potionPos = positionManager.findRandomHealthPotionPosition();
        assets->CreateObject(potionPos.x, potionPos.y, "healthpotion");
    }
    
    Vector2D playerSpawnPos = positionManager.findRandomSpawnPosition(currentLevel);
    
    player->addComponent<TransformComponent>(playerSpawnPos.x, playerSpawnPos.y, 32, 32, 3);
    player->addComponent<SpriteComponent>("player", true);
    player->addComponent<ColliderComponent>("player", 21 * 3, 29 * 3, 6 * 3, 4 * 3);
    player->addComponent<HealthComponent>(100);
    player->addComponent<AmmoComponent>(30, 10);
    player->addComponent<KeyboardController>();
    player->addGroup(Game::groupPlayers);

    int numEnemies = 3;
    if (currentLevel == 2) {
        numEnemies = 8;
    } else if (currentLevel == 3) {
        numEnemies = 13;
    } else if (currentLevel == 4) {
        numEnemies = 8;
    }
    
    if (currentLevel == 4) {
        finalBoss = &manager.addEntity();
        
        Vector2D bossPos = {34*64, 15*64};
        
        finalBoss->addComponent<TransformComponent>(bossPos.x, bossPos.y, 32, 32, 4);
        finalBoss->addComponent<SpriteComponent>("boss", true);
        finalBoss->addComponent<ColliderComponent>("boss");
        finalBoss->addComponent<HealthComponent>(500);
        finalBoss->addComponent<EnemyAIComponent>(manager);
        finalBoss->getComponent<EnemyAIComponent>().setSpeed(0.5f);
        finalBoss->getComponent<EnemyAIComponent>().setChaseRange(500.0f);
        finalBoss->addGroup(Game::groupEnemies);
        
        scientist = &manager.addEntity();
        Vector2D scientistPos = {34*64, 3*64};
        scientist->addComponent<TransformComponent>(scientistPos.x, scientistPos.y, 32, 32, 3);
        scientist->addComponent<SpriteComponent>("scientist", true);
        scientist->addComponent<ColliderComponent>("scientist");
        scientist->getComponent<SpriteComponent>().Play("Locked");
        scientist->addGroup(Game::groupObjects);
        
    } else {
        finalBoss = nullptr;
        scientist = nullptr;
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

    int initResult = Mix_Init(0);
    printf("SDL_mixer initialization result: %d\n", initResult);
    
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        std::cout << "Error initializing SDL_mixer: " << Mix_GetError() << std::endl;
        std::cout << "Continuing without music support" << std::endl;
    } else {
        std::cout << "SDL_mixer initialized successfully" << std::endl;
    }
    
    Mix_AllocateChannels(8);

    if(assets != nullptr) {
        delete assets;
    }

    assets = new AssetManager(&manager);

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
    
    assets->AddFont("font1", "./assets/MINECRAFT.TTF", 32);
    assets->AddFont("font2", "./assets/MINECRAFT.TTF", 72);

    assets->AddSound("click", "./assets/sounds/click.mp3");
    assets->AddSound("shoot", "./assets/sounds/shoot.wav");
    assets->AddSound("hurt", "./assets/sounds/hurt.wav");
    assets->AddSound("levelTransition", "./assets/sounds/leveltransition.wav");
    assets->AddSound("gameOver", "./assets/sounds/gameover.mp3");
    assets->AddSound("victory", "./assets/sounds/victory.mp3");

    assets->AddSound("magazine", "./assets/sounds/objects/magazine.wav");
    assets->AddSound("healthpotion", "./assets/sounds/objects/healthpotion.wav");

    assets->AddSound("correctanswer", "./assets/sounds/question/correctanswer.mp3");
    assets->AddSound("wronganswer", "./assets/sounds/question/wronganswer.mp3");
    
    assets->AddMusic("mainmenu", "./assets/sounds/levels/mainmenu.ogg");
    assets->AddMusic("level1", "./assets/sounds/levels/level1.ogg");
    assets->AddMusic("level2", "./assets/sounds/levels/level2.ogg");
    assets->AddMusic("level3-4", "./assets/sounds/levels/level3-4.ogg");
    assets->AddMusic("boss", "./assets/sounds/levels/bossmusic.ogg");
    
    assets->SetMasterVolume(volumeLevel);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    transitionManager.init(this, &manager);
    
    transitionLabel = &manager.addEntity();
    transitionLabel->addComponent<UILabel>(0, 0, "", "font2", white);
    transitionManager.mTransitionLabel = transitionLabel;
    
    gameState = STATE_MAIN_MENU;
    
    if (gameState == STATE_MAIN_MENU) {
        initMainMenu();
    } else {
        initEntities();
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
            if (gameState == STATE_REPLAY && event.key.keysym.sym == SDLK_ESCAPE) {
                isReplaying = false;
                returnToMainMenu = true;
                break;
            }
            
            if (gameState == STATE_MAIN_MENU) {
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                        selectedMenuItem = (selectedMenuItem - 1 + MENU_ITEMS_COUNT) % MENU_ITEMS_COUNT;
                        menuHighlightActive = true;
                        updateMainMenu();
                        break;
                    case SDLK_DOWN:
                        selectedMenuItem = (selectedMenuItem + 1) % MENU_ITEMS_COUNT;
                        menuHighlightActive = true;
                        updateMainMenu();
                        break;
                    case SDLK_RETURN:
                    case SDLK_SPACE:
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
                                initLeaderboard();
                                gameState = STATE_LEADERBOARD;
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
                    restart();
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE) {
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
                    togglePause();
                }
                else if (event.key.keysym.sym == SDLK_e && currentLevel == 4 && canRescueScientist && !scientistRescued && scientist != nullptr) {
                    Vector2D playerPos = player->getComponent<TransformComponent>().position;
                    Vector2D scientistPos = scientist->getComponent<TransformComponent>().position;
                    float distance = sqrt(pow(playerPos.x - scientistPos.x, 2) + pow(playerPos.y - scientistPos.y, 2));
                    
                    if (distance <= 100) {
                        scientist->getComponent<SpriteComponent>().Play("Idle");
                        scientistRescued = true;
                        initEndScreen(true);
                    }
                }
            }
            else if (gameState == STATE_PAUSE) {
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
                        switch(selectedPauseItem) {
                            case PAUSE_RESUME:
                                togglePause();
                                break;
                            case PAUSE_SAVE:
                                saveGame();
                                break;
                            case PAUSE_RESTART:
                                restart();
                                break;
                            case PAUSE_SETTINGS:
                                gameState = STATE_SETTINGS;
                                initSettingsMenu();
                                break;
                            case PAUSE_MAIN_MENU:
                                returnToMainMenu = true;
                                break;
                        }
                        break;
                    case SDLK_ESCAPE:
                        togglePause();
                        break;
                }
            }
            else if (gameState == STATE_SETTINGS) {
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_DOWN:
                        selectedSettingsItem = (selectedSettingsItem == SETTINGS_VOLUME) ? 
                                              SETTINGS_BACK : SETTINGS_VOLUME;
                        settingsHighlightActive = true;
                        updateSettingsMenu();
                        break;
                    case SDLK_LEFT:
                        if (selectedSettingsItem == SETTINGS_VOLUME) {
                            volumeLevel = std::max(0, volumeLevel - 5);
                            updateSettingsMenu();
                        }
                        break;
                    case SDLK_RIGHT:
                        if (selectedSettingsItem == SETTINGS_VOLUME) {
                            volumeLevel = std::min(100, volumeLevel + 5);
                            updateSettingsMenu();
                        }
                        break;
                    case SDLK_RETURN:
                    case SDLK_SPACE:
                        if (selectedSettingsItem == SETTINGS_BACK) {
                            gameState = previousState;
                            applySettings();
                        }
                        break;
                    case SDLK_ESCAPE:
                        gameState = previousState;
                        applySettings();
                        break;
                }
            }
            else if (gameState == STATE_END_SCREEN) {
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_DOWN:
                        selectedEndOption = (selectedEndOption == END_RESTART) ? END_MAIN_MENU : END_RESTART;
                        endHighlightActive = true;
                        updateEndScreen();
                        break;
                    case SDLK_RETURN:
                    case SDLK_SPACE:
                        if (selectedEndOption == END_RESTART) {
                            restart();
                        } else if (selectedEndOption == END_REPLAY) {
                            replay();
                        } else if (selectedEndOption == END_MAIN_MENU) {
                            returnToMainMenu = true;
                        }
                        break;
                    case SDLK_ESCAPE:
                        returnToMainMenu = true;
                        break;
                }
            }
            else if (gameState == STATE_LEADERBOARD) {
                if (event.key.keysym.sym == SDLK_ESCAPE || 
                    event.key.keysym.sym == SDLK_RETURN || 
                    event.key.keysym.sym == SDLK_SPACE) {
                    gameState = STATE_MAIN_MENU;
                    
                    if (leaderboardTitle) leaderboardTitle->destroy();
                    for (auto& label : leaderboardEntryLabels) {
                        if (label) label->destroy();
                        label = nullptr;
                    }
                    if (leaderboardBackButton) leaderboardBackButton->destroy();
                    
                    initMainMenu();
                }
            }
            break;
            
        case SDL_MOUSEMOTION:
            if (gameState == STATE_MAIN_MENU) {
                bool anyHovered = false;
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
                
                if (menuNewGameButton && menuNewGameButton->hasComponent<UILabel>()) {
                    bool isOver = menuNewGameButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedMenuItem = MENU_NEW_GAME;
                        menuHighlightActive = true;
                    }
                }
                
                if (menuLoadGameButton && menuLoadGameButton->hasComponent<UILabel>()) {
                    bool isOver = menuLoadGameButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedMenuItem = MENU_LOAD_GAME;
                        menuHighlightActive = true;
                    }
                }
                
                if (menuSettingsButton && menuSettingsButton->hasComponent<UILabel>()) {
                    bool isOver = menuSettingsButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedMenuItem = MENU_SETTINGS;
                        menuHighlightActive = true;
                    }
                }
                
                if (menuLeaderboardButton && menuLeaderboardButton->hasComponent<UILabel>()) {
                    bool isOver = menuLeaderboardButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedMenuItem = MENU_LEADERBOARD;
                        menuHighlightActive = true;
                    }
                }
                
                if (menuExitButton && menuExitButton->hasComponent<UILabel>()) {
                    bool isOver = menuExitButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedMenuItem = MENU_EXIT;
                        menuHighlightActive = true;
                    }
                }
                
                if (!anyHovered && menuHighlightActive) {
                    menuHighlightActive = false;
                    updateMainMenu();
                } else if (anyHovered) {
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
                bool anyHovered = false;
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
                
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
                
                if (!anyHovered && pauseHighlightActive) {
                    pauseHighlightActive = false;
                    updatePauseMenu();
                } else if (anyHovered) {
                    updatePauseMenu();
                }
            }
            else if (gameState == STATE_SETTINGS) {
                bool anyHovered = false;
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
                
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
                    
                    settingsBackButton->getComponent<UILabel>().HandleEvent(event);
                }
                
                if (draggingVolumeSlider) {
                    if (mouseX < sliderX) {
                        volumeLevel = 0;
                    } else if (mouseX > sliderX + sliderWidth) {
                        volumeLevel = 100;
                    } else {
                        volumeLevel = ((mouseX - sliderX) * 100) / sliderWidth;
                    }
                    
                    if (volumeLabel && volumeLabel->hasComponent<UILabel>()) {
                        std::stringstream volSS;
                        volSS << "Volume: " << volumeLevel << "%";
                        volumeLabel->getComponent<UILabel>().SetLabelText(volSS.str(), "font1");
                        
                        int volLabelWidth = volumeLabel->getComponent<UILabel>().GetWidth();
                        int volLabelX = (1920 - volLabelWidth) / 2;
                        volumeLabel->getComponent<UILabel>().SetPosition(volLabelX, 300);
                    }
                }
                
                if (!anyHovered && settingsHighlightActive && !draggingVolumeSlider) {
                    settingsHighlightActive = false;
                }
            }
            else if (gameState == STATE_END_SCREEN) {
                bool anyHovered = false;
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
                
                if (endRestartButton && endRestartButton->hasComponent<UILabel>()) {
                    bool isOver = endRestartButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedEndOption = END_RESTART;
                        endHighlightActive = true;
                    }
                }
                
                if (endReplayButton && endReplayButton->hasComponent<UILabel>()) {
                    bool isOver = endReplayButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedEndOption = END_REPLAY;
                        endHighlightActive = true;
                    }
                }
                
                if (endMenuButton && endMenuButton->hasComponent<UILabel>()) {
                    bool isOver = endMenuButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
                    if (isOver) {
                        anyHovered = true;
                        selectedEndOption = END_MAIN_MENU;
                        endHighlightActive = true;
                    }
                }
                
                if (!anyHovered && endHighlightActive) {
                    endHighlightActive = false;
                    updateEndScreen();
                } else if (anyHovered) {
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
            else if (gameState == STATE_LEADERBOARD) {
                if (leaderboardBackButton && leaderboardBackButton->hasComponent<UILabel>()) {
                    leaderboardBackButton->getComponent<UILabel>().HandleEvent(event);
                }
            }
            break;
            
        case SDL_MOUSEBUTTONDOWN:
            if (gameState == STATE_MAIN_MENU) {
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
                int mouseX = event.button.x;
                int mouseY = event.button.y;
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
                    
                    if (mouseX < sliderX) {
                        volumeLevel = 0;
                    } else if (mouseX > sliderX + sliderWidth) {
                        volumeLevel = 100;
                    } else {
                        volumeLevel = ((mouseX - sliderX) * 100) / sliderWidth;
                    }
                    
                    if (volumeLabel && volumeLabel->hasComponent<UILabel>()) {
                        std::stringstream volSS;
                        volSS << "Volume: " << volumeLevel << "%";
                        volumeLabel->getComponent<UILabel>().SetLabelText(volSS.str(), "font1");
                        
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
            else if (gameState == STATE_LEADERBOARD) {
                if (leaderboardBackButton && leaderboardBackButton->hasComponent<UILabel>()) {
                    leaderboardBackButton->getComponent<UILabel>().HandleEvent(event);
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
    
    if (returnToMainMenu) {
        returnToMainMenu = false;
        
        gameOver = false;
        playerWon = false;
        collectedClues = 0;
        damageTimer = 1.0f;
        hurtSoundTimer = 0.0f;
        objectCollisionDelay = 1.0f;
        objectCollisionsEnabled = false;
        questionActive = false;
        pendingClueEntity = nullptr;
        showFeedback = false;
        showingExitInstructions = false;
        level4MapChanged = false;
        finalBossDefeated = false;
        bossMusicPlaying = false;
        scientistRescued = false;
        canRescueScientist = false;
        currentLevel = 1;
        
        resetUsedQuestions();
        
        totalClues = 3;
        
        positionManager.resetPositions();
        
        gameStartTime = 0;
        gameplayTime = 0;
        
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
        
        if (map != nullptr) {
            delete map;
            map = nullptr;
        }
        
        manager.clear();
        
        gameState = STATE_MAIN_MENU;
        
        initMainMenu();
        
        return;
    }
    
    switch (gameState) {
        case STATE_MAIN_MENU:
            break;
            
        case STATE_END_SCREEN:
            updateEndScreen();
            break;
            
        case STATE_PAUSE:
            updatePauseMenu();
            break;
            
        case STATE_SETTINGS:
            updateSettingsMenu();
            break;
            
        case STATE_REPLAY:
            updateReplay();
            break; 
            
        case STATE_LEADERBOARD:
            updateLeaderboard();
            break;
            
        case STATE_GAME:
            manager.refresh();
            
            if (player && player->isActive() && !isReplaying) {
                recordPlayerPosition();
            }
            
            if (gameStartTime > 0 && !gameOver) {
                gameplayTime = currentTime - gameStartTime;
                
                Uint32 totalSeconds = gameplayTime / 1000;
                Uint32 minutes = totalSeconds / 60;
                Uint32 seconds = totalSeconds % 60;
                
                std::stringstream timeSS;
                timeSS << "Time: " << std::setfill('0') << std::setw(2) << minutes 
                       << ":" << std::setfill('0') << std::setw(2) << seconds;
                
                if (timerLabel != nullptr && timerLabel->hasComponent<UILabel>()) {
                    timerLabel->getComponent<UILabel>().SetLabelText(timeSS.str(), "font1");
                    
                    int timerWidth = timerLabel->getComponent<UILabel>().GetWidth();
                    int xPos = (1920 - timerWidth) / 2;
                    timerLabel->getComponent<UILabel>().SetPosition(xPos, 20);
                }
            }
            
            if (transitionManager.isTransitioning()) {
                if (transitionManager.updateTransition()) {
                    collectedClues = 0;
                    damageTimer = 1.0f;
                    hurtSoundTimer = 0.0f;
                    objectCollisionDelay = 1.0f;
                    objectCollisionsEnabled = false;
                    questionActive = false;
                    pendingClueEntity = nullptr;
                    showFeedback = false;
                    showingExitInstructions = false;
                    
                    positionManager.resetPositions();
                    
                    int nextLevel = currentLevel + 1;
                    currentLevel = nextLevel;
                    
                    manager.clear();
                    
                    if (map != nullptr) {
                        delete map;
                        map = nullptr;
                    }
                    
                    loadLevel(currentLevel);
                    
                    initEntities();
                    
                    transitionLabel = &manager.addEntity();
                    transitionLabel->addComponent<UILabel>(0, 0, "", "font2", white);
                    
                    transitionManager.mTransitionLabel = transitionLabel;
                    transitionManager.init(this, &manager);
                }
                return;
            }
    
            for(auto& p : *players) {
                if (p->hasComponent<SpriteComponent>()) {
                    p->getComponent<SpriteComponent>().update();
                }
            }
            
            for(auto& e : *enemies) {
                if (!e || !e->isActive()) {
                    continue; 
                }

                if (e->hasComponent<SpriteComponent>()) {
                    try {
                         e->getComponent<SpriteComponent>().update();
                    } catch (const std::exception& ex) {
                         std::cerr << "Exception during SpriteComponent::update(): " << ex.what() << std::endl;
                    } catch (...) {
                         std::cerr << "Unknown exception during SpriteComponent::update()." << std::endl;
                    }
                }
                if (!questionActive && !gameOver) {
                    if (e->hasComponent<EnemyAIComponent>()) {
                         if (!player || !player->isActive()) {
                         } else {
                            try {
                                e->getComponent<EnemyAIComponent>().update();
                            } catch (const std::exception& ex) {
                                std::cerr << "Exception during EnemyAIComponent::update(): " << ex.what() << std::endl;
                            } catch (...) {
                                std::cerr << "Unknown exception during EnemyAIComponent::update()." << std::endl;
                            }
                         }
                    } else {
                    }
                } else if (e->hasComponent<SpriteComponent>()) {
                    try {
                        e->getComponent<SpriteComponent>().Play("Idle");
                    } catch (const std::exception& ex) {
                         std::cerr << "Exception during SpriteComponent::Play('Idle'): " << ex.what() << std::endl;
                    } catch (...) {
                         std::cerr << "Unknown exception during SpriteComponent::Play('Idle')." << std::endl;
                    }
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
            
            if (player != nullptr && player->isActive() && !gameOver) {
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

            if (showFeedback) {
                if (!showingExitInstructions && (currentTime - feedbackStartTime > 1200)) {
                    closeQuestion();
                } 
            }

            if (questionActive || gameOver) {
                if (questionActive && player != nullptr && player->isActive() && 
                    player->hasComponent<TransformComponent>()) {
                    player->getComponent<TransformComponent>().velocity.x = 0;
                    player->getComponent<TransformComponent>().velocity.y = 0;
                }
                return;
            }
            
            manager.update();
            
            if (player != nullptr && player->isActive() && gameState == STATE_GAME) {
                Vector2D playerPos = player->getComponent<TransformComponent>().position;
                
                TransformComponent& playerTransform = player->getComponent<TransformComponent>();
                
                int worldWidth = 60 * 32 * 2;
                int worldHeight = 34 * 32 * 2;
                
                int playerWidth = playerTransform.width * playerTransform.scale;
                int playerHeight = playerTransform.height * playerTransform.scale;
                
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

                player->getComponent<ColliderComponent>().update();
                SDL_Rect playerCol = player->getComponent<ColliderComponent>().collider;
                
                damageTimer -= 1.0f/60.0f;
                
                if (hurtSoundTimer > 0.0f) {
                    hurtSoundTimer -= 1.0f/60.0f;
                }
                
                if (!objectCollisionsEnabled) {
                    objectCollisionDelay -= 1.0f/60.0f;
                    if (objectCollisionDelay <= 0.0f) {
                        objectCollisionsEnabled = true;
                    }
                }

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
                                assets->PlaySound("magazine", volumeLevel);
                            }
                            else if (o->getComponent<ColliderComponent>().tag == "healthpotion") {
                                player->getComponent<HealthComponent>().heal(20);
                                o->destroy();
                                assets->PlaySound("healthpotion", volumeLevel);
                            }
                        }
                    }
                }

                for(auto& c : *colliders) {
                    SDL_Rect cCol = c->getComponent<ColliderComponent>().collider;
                    SDL_Rect updatedPlayerCol = player->getComponent<ColliderComponent>().collider;
                    
                    if(Collision::AABB(cCol, updatedPlayerCol)) {
                        float overlapX = 0.0f;
                        float overlapY = 0.0f;

                        float dx = (updatedPlayerCol.x + updatedPlayerCol.w / 2.0f) - (cCol.x + cCol.w / 2.0f);
                        float dy = (updatedPlayerCol.y + updatedPlayerCol.h / 2.0f) - (cCol.y + cCol.h / 2.0f);

                        float combinedHalfWidths = (updatedPlayerCol.w / 2.0f) + (cCol.w / 2.0f);
                        float combinedHalfHeights = (updatedPlayerCol.h / 2.0f) + (cCol.h / 2.0f);

                        overlapX = combinedHalfWidths - std::abs(dx);
                        overlapY = combinedHalfHeights - std::abs(dy);

                        if (overlapX < overlapY) {
                            if (dx > 0) {
                                playerTransform.position.x += overlapX;
                            } else {
                                playerTransform.position.x -= overlapX;
                            }
                        } else {
                            if (dy > 0) {
                                playerTransform.position.y += overlapY;
                            } else {
                                playerTransform.position.y -= overlapY;
                            }
                        }

                        player->getComponent<ColliderComponent>().update(); 
                    }
                }

                for(auto& e : *enemies) {
                    TransformComponent& enemyTransform = e->getComponent<TransformComponent>();

                    for (auto& c : *colliders) {
                        SDL_Rect cCol = c->getComponent<ColliderComponent>().collider;
                        SDL_Rect enemyCol = e->getComponent<ColliderComponent>().collider;

                        if (Collision::AABB(cCol, enemyCol)) {
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

                            if (overlapX < overlapY) {
                                collidedHorizontally = true;
                                if (dx > 0) enemyTransform.position.x += overlapX;
                                else enemyTransform.position.x -= overlapX;
                                enemyTransform.velocity.x = 0;
                            } else {
                                collidedVertically = true;
                                if (dy > 0) enemyTransform.position.y += overlapY;
                                else enemyTransform.position.y -= overlapY;
                                enemyTransform.velocity.y = 0;
                            }
                            
                            if (e->hasComponent<EnemyAIComponent>()) {
                                e->getComponent<EnemyAIComponent>().notifyTerrainCollision(collidedHorizontally, collidedVertically);
                            }

                            e->getComponent<ColliderComponent>().update();
                        }
                    }

                    for(auto& p : *projectiles) {
                        if(Collision::AABB(e->getComponent<ColliderComponent>().collider, 
                                        p->getComponent<ColliderComponent>().collider)) {
                            e->getComponent<HealthComponent>().takeDamage(25);
                            p->destroy();
                        }
                    }

                    SDL_Rect updatedPlayerCol = player->getComponent<ColliderComponent>().collider;
                    if(Collision::AABB(updatedPlayerCol, e->getComponent<ColliderComponent>().collider) && damageTimer <= 0) {
                        if (currentLevel == 4 && e == finalBoss) {
                            player->getComponent<HealthComponent>().takeDamage(10);
                        } else {
                            player->getComponent<HealthComponent>().takeDamage(5);
                        }
                        if (hurtSoundTimer <= 0.0f) {
                            assets->PlaySound("hurt", volumeLevel);
                            hurtSoundTimer = hurtSoundCooldown;
                        }
                        damageTimer = damageCooldown;
                    }
                    
                    if(e->getComponent<HealthComponent>().health <= 0) {
                        if (currentLevel == 4 && e == finalBoss) {
                            finalBossDefeated = true;
                            
                            if (assets) {
                                assets->StopMusic();
                                assets->PlayMusic("level3-4", volumeLevel);
                                currentMusic = "level3-4";
                                bossMusicPlaying = false;
                            }
                            
                            feedbackLabel->getComponent<UILabel>().SetLabelText("BOSS DEFEATED! The path is revealed! Find and rescue the SUPERUM!", "font1", {255, 215, 0, 255});
                            
                            int feedbackWidth = feedbackLabel->getComponent<UILabel>().GetWidth();
                            int xPos = (1920 - feedbackWidth) / 2;
                            feedbackLabel->getComponent<UILabel>().SetPosition(xPos, 950);
                            
                            showFeedback = true;
                            feedbackStartTime = SDL_GetTicks();
                            canRescueScientist = true;   
                            level4MapChanged = true;
                            
                            for (auto c : *colliders) {
                                c->destroy();
                            }
                            
                            if (map != nullptr) {
                                delete map;
                                map = nullptr;
                            }
                            
                            std::string terrainTexture = "terrainlvl4";
                            std::string mapPath = "./assets/lvl4/Level4MapAfter.map";
                            
                            map = new Map(terrainTexture, 2, 32, manager);
                            map->LoadMap(mapPath, 60, 34);
                            
                            manager.refresh();
                        }
                        
                        e->destroy();
                    }
                }

                if(player->getComponent<HealthComponent>().health <= 0) {
                    player->destroy();
                    
                    for(auto& e : *enemies) {
                        if(e->hasComponent<SpriteComponent>()) {
                            e->getComponent<SpriteComponent>().Play("Idle");
                        }
                    }
                    
                    gameOver = true;
                    playerWon = false;
                    
                    initEndScreen(false);
                    return;
                }

                camera.x = player->getComponent<TransformComponent>().position.x - (camera.w / 2);
                camera.y = player->getComponent<TransformComponent>().position.y - (camera.h / 2);

                if(camera.x < 0) camera.x = 0;
                if(camera.y < 0) camera.y = 0;
                if(camera.x > worldWidth - camera.w) camera.x = worldWidth - camera.w;
                if(camera.y > worldHeight - camera.h) camera.y = worldHeight - camera.h;
                
                if (currentLevel == 4 && !finalBossDefeated && finalBoss != nullptr && player != nullptr) {
                    Vector2D playerPos = player->getComponent<TransformComponent>().position;
                    Vector2D bossPos = finalBoss->getComponent<TransformComponent>().position;
                    float distance = sqrt(pow(playerPos.x - bossPos.x, 2) + pow(playerPos.y - bossPos.y, 2));
                    
                    if (distance <= 500 && assets && !bossMusicPlaying) {
                        finalBoss->getComponent<EnemyAIComponent>().setChaseRange(800.0f);
                        assets->StopMusic();
                        assets->PlayMusic("boss", volumeLevel);
                        currentMusic = "boss";
                        bossMusicPlaying = true;    
                    }
                }
                
                if ((currentLevel != 4 && collectedClues >= totalClues) || 
                    (currentLevel == 4 && finalBossDefeated)) {
                    if (!showingExitInstructions) {
                        if (currentLevel == 3) {
                            feedbackLabel->getComponent<UILabel>().SetLabelText("All clues collected! Enter the pyramid to see what lies ahead.", "font1", {255, 215, 0, 255});
                        } else if (currentLevel == 4 && level4MapChanged) {
                            showingExitInstructions = true;
                        } else {
                            feedbackLabel->getComponent<UILabel>().SetLabelText("All clues collected! Head NORTH to exit the level.", "font1", {255, 215, 0, 255});
                        }
                        
                        int feedbackWidth = feedbackLabel->getComponent<UILabel>().GetWidth();
                        int xPos = (1920 - feedbackWidth) / 2;
                        feedbackLabel->getComponent<UILabel>().SetPosition(xPos, 950);
                        
                        showFeedback = true;
                        feedbackStartTime = SDL_GetTicks();
                        showingExitInstructions = true;
                    }
                    
                    if (currentLevel == 3) {
                        float playerX = player->getComponent<TransformComponent>().position.x / 64;
                        float playerY = player->getComponent<TransformComponent>().position.y / 64;
                        
                        SDL_Rect playerCollider = player->getComponent<ColliderComponent>().collider;
                        SDL_Rect pyramidEntrance = {
                            24 * 64, // x
                            19 * 64, // y
                            3 * 64,  // width
                            1 * 64   // height
                        };
                        
                        pyramidEntrance.x -= camera.x;
                        pyramidEntrance.y -= camera.y;
                        
                        if (Collision::AABB(playerCollider, pyramidEntrance) || 
                            ((playerX >= 23 && playerX <= 27) && (playerY >= 18 && playerY <= 20))) {
                            advanceToNextLevel();
                        }
                    } 
                    else {
                        if (player->getComponent<TransformComponent>().position.y < 100) {
                                advanceToNextLevel();
                        }
                    }
                }
            }
            break;
    }

    if (showFeedback && feedbackLabel && feedbackLabel->hasComponent<UILabel>()) {
        bool isExitInstructions = 
            showingExitInstructions && 
            ((currentLevel != 4 && collectedClues >= totalClues) || 
            (currentLevel == 4 && finalBossDefeated));
            
        if (hasSavedDuringExitInstructions && currentTime - feedbackStartTime >= feedbackDuration) {
            feedbackLabel->getComponent<UILabel>().SetLabelText(savedExitInstructionsText, "font1", {255, 215, 0, 255});
            int feedbackWidth = feedbackLabel->getComponent<UILabel>().GetWidth();
            int xPos = (1920 - feedbackWidth) / 2;
            feedbackLabel->getComponent<UILabel>().SetPosition(xPos, 950); // Bottom of screen
            
            hasSavedDuringExitInstructions = false;
            
            feedbackStartTime = currentTime;
        }
        else if (!isExitInstructions && !hasSavedDuringExitInstructions) {
            if (currentTime - feedbackStartTime >= feedbackDuration) {
                showFeedback = false;
                feedbackLabel->getComponent<UILabel>().SetLabelText("", "font1", white);
            }
        }
    }
}

void Game::render() {
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
            
        case STATE_LEADERBOARD:
            renderLeaderboard();
            break;
            
        case STATE_GAME:
            SDL_RenderClear(renderer);

            if (transitionManager.isTransitioning()) {
                transitionManager.renderTransition();
                SDL_RenderPresent(renderer);
                return;
            }
            
            for (auto& m : manager.getGroup(groupMap)) {
                m->draw();
            }
            
            for (auto& c : manager.getGroup(groupColliders)) {
                c->draw();
            }
            for (auto& o : manager.getGroup(groupObjects)) {
                o->draw();
            }
            for (auto& n : manager.getGroup(groupNPCs)) {
                n->draw();
            }
            for (auto& p : manager.getGroup(groupPlayers)) {
                p->draw();
            }
            for (auto& p : manager.getGroup(groupProjectiles)) {
                p->draw();
            }
            for (auto& e : manager.getGroup(groupEnemies)) {
                e->draw();
            }
            
            for (auto& ui : manager.getGroup(groupUI)) {
                ui->draw();
            }
            
            if (questionActive) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
                SDL_Rect questionBg = {1920/6, 250, 1920*2/3, 350};
                SDL_RenderFillRect(renderer, &questionBg);
                
                questionLabel->draw();
                answer1Label->draw();
                answer2Label->draw();
                answer3Label->draw();
                answer4Label->draw();
                
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }
            
            if (showFeedback && feedbackLabel != nullptr) {
                bool isSaveNotification = 
                    feedbackLabel->hasComponent<UILabel>() && 
                    feedbackLabel->getComponent<UILabel>().GetWidth() > 0 &&
                    feedbackLabel->getComponent<UILabel>().GetPosition().y == 950 &&
                    !showingExitInstructions;
                    
                if (!showingExitInstructions && !isSaveNotification) {
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 230);
                    SDL_Rect feedbackBg = {1920/4, 630, 1920/2, 100};
                    SDL_RenderFillRect(renderer, &feedbackBg);
                }
                
                feedbackLabel->draw();
                
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }
            
            if (currentLevel == 4 && canRescueScientist && !scientistRescued && 
                player != nullptr && scientist != nullptr) {
                Vector2D playerPos = player->getComponent<TransformComponent>().position;
                Vector2D scientistPos = scientist->getComponent<TransformComponent>().position;
                float distance = sqrt(pow(playerPos.x - scientistPos.x, 2) + pow(playerPos.y - scientistPos.y, 2));
                
                if (distance <= 100) {
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);
                    
                    int promptX = static_cast<int>(scientistPos.x) - Game::camera.x + (32 * 3) / 2;
                    int promptY = static_cast<int>(scientistPos.y) - Game::camera.y - 50;
                    
                    SDL_Rect promptRect = {promptX - 75, promptY - 15, 150, 35};
                    SDL_RenderFillRect(renderer, &promptRect);
                    
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 80);
                    SDL_RenderDrawRect(renderer, &promptRect);
                    
                    static Entity* promptLabel = nullptr;
                    if (promptLabel == nullptr) {
                        promptLabel = &manager.addEntity();
                        SDL_Color dimWhite = {220, 220, 220, 255};
                        promptLabel->addComponent<UILabel>(0, 0, "Press E", "font1", dimWhite);
                    }
                    
                    int labelWidth = promptLabel->getComponent<UILabel>().GetWidth();
                    promptLabel->getComponent<UILabel>().SetPosition(promptX - labelWidth/2, promptY - 10);
                    promptLabel->draw();
                    
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                }
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
    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
    std::cout << "Game cleaned" << std::endl;
}

void Game::restart() {
    gameOver = false;
    playerWon = false;
    collectedClues = 0;
    damageTimer = 1.0f;
    hurtSoundTimer = 0.0f;
    objectCollisionDelay = 1.0f;
    objectCollisionsEnabled = false;
    questionActive = false;
    pendingClueEntity = nullptr;
    showFeedback = false;
    showingExitInstructions = false;
    level4MapChanged = false;
    finalBossDefeated = false;
    bossMusicPlaying = false;
    scientistRescued = false;
    canRescueScientist = false;
    currentLevel = 1;
    gameState = STATE_GAME;
    
    resetUsedQuestions();
    
    totalClues = 3;
    
    positionManager.resetPositions();
    
    manager.clear();
    
    if (map != nullptr) {
        delete map;
        map = nullptr;
    }
    
    gameStartTime = SDL_GetTicks();
    gameplayTime = 0;
    
    loadLevel(currentLevel);
    
    initEntities();
}

void Game::showQuestion(Entity* clueEntity) {
    if (questionActive) return;
    
    questionActive = true;
    answerSubmitted = false;
    pendingClueEntity = clueEntity;
    
    if (player != nullptr && player->isActive()) {
        player->getComponent<SpriteComponent>().Play("Idle");
        
        if (player->hasComponent<KeyboardController>()) {
            player->getComponent<KeyboardController>().enabled = false;
        }
    }
    
    for(auto& e : *enemies) {
        e->getComponent<SpriteComponent>().Play("Idle");
    }
    
    if (usedQuestions.size() >= questions.size()) {
        usedQuestions.clear();
    }
    
    static std::mt19937 rng(std::time(nullptr));
    
    std::vector<int> availableQuestions;
    for (size_t i = 0; i < questions.size(); i++) {
        if (usedQuestions.find(i) == usedQuestions.end()) {
            availableQuestions.push_back(i);
        }
    }
    
    int randomIndex = rng() % availableQuestions.size();
    currentQuestion = availableQuestions[randomIndex];
    
    usedQuestions.insert(currentQuestion);
    
    Question q = questions[currentQuestion];
    
    SDL_Color questionColor = {255, 255, 0, 255};
    SDL_Color answerColor = {255, 255, 255, 255};
    
    questionLabel->getComponent<UILabel>().SetLabelText(q.question, "font1", questionColor);
    answer1Label->getComponent<UILabel>().SetLabelText("1: " + q.answers[0], "font1", answerColor);
    answer2Label->getComponent<UILabel>().SetLabelText("2: " + q.answers[1], "font1", answerColor);
    answer3Label->getComponent<UILabel>().SetLabelText("3: " + q.answers[2], "font1", answerColor);
    answer4Label->getComponent<UILabel>().SetLabelText("4: " + q.answers[3], "font1", answerColor);
    
    int questionWidth = questionLabel->getComponent<UILabel>().GetWidth();
    int xPos = (1920 - questionWidth) / 2;
    
    questionLabel->getComponent<UILabel>().SetPosition(xPos, 300);
    
    int xPosAnswers = 1920 / 3;
    answer1Label->getComponent<UILabel>().SetPosition(xPosAnswers, 360);
    answer2Label->getComponent<UILabel>().SetPosition(xPosAnswers, 410);
    answer3Label->getComponent<UILabel>().SetPosition(xPosAnswers, 460);
    answer4Label->getComponent<UILabel>().SetPosition(xPosAnswers, 510);
}

void Game::checkAnswer(int selectedAnswer) {
    if (!questionActive || answerSubmitted) return;
    
    answerSubmitted = true;
    
    isAnswerCorrect = (selectedAnswer == questions[currentQuestion].correctAnswer);
    
    if (isAnswerCorrect) {
        collectedClues++;
        pendingClueEntity->destroy();
        feedbackLabel->getComponent<UILabel>().SetLabelText("CORRECT!", "font2", green);
        assets->PlaySound("correctanswer", volumeLevel);
    } else {
        feedbackLabel->getComponent<UILabel>().SetLabelText("INCORRECT!", "font2", red);
        assets->PlaySound("wronganswer", volumeLevel);
    }
    
    int feedbackWidth = feedbackLabel->getComponent<UILabel>().GetWidth();
    int xPos = (1920 - feedbackWidth) / 2;
    feedbackLabel->getComponent<UILabel>().SetPosition(xPos, 650);
    
    showFeedback = true;
    feedbackStartTime = SDL_GetTicks();
}

void Game::closeQuestion() {
    questionActive = false;
    answerSubmitted = false;
    pendingClueEntity = nullptr;
    showFeedback = false;
    
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

// Add this helper function before loadLevel
void Game::setLevelParameters(int level) {
    if (level == 1) {
        totalClues = 3;
        totalMagazines = 3;
        totalHealthPotions = 2;
    } else if (level == 2) {
        totalClues = 5;
        totalMagazines = 9;  
        totalHealthPotions = 9;
    } else if (level == 3) {
        totalClues = 7;
        totalMagazines = 15;  
        totalHealthPotions = 15;
    } else if (level == 4) {
        totalClues = 0;
        totalMagazines = 7;
        totalHealthPotions = 9;
    } else {
        // Default to level 1 parameters if something is wrong
        std::cerr << "Warning: Unknown level " << level << ". Using level 1 parameters." << std::endl;
        totalClues = 3;
        totalMagazines = 3;
        totalHealthPotions = 2;
    }
    
    // Level 4 specific initializations
    if (level == 4) {
        if (!level4MapChanged) {
            level4MapChanged = false;
            finalBossDefeated = false;
            bossMusicPlaying = false;
        }
    }
}

void Game::loadLevel(int levelNum) {
    if (map != nullptr) {
        delete map;
        map = nullptr;
    }
    
    currentLevel = levelNum;
    collectedClues = 0;
    showingExitInstructions = false;
    
    if (assets) {
        assets->StopMusic();
        
        switch (currentLevel) {
            case 1:
                assets->PlayMusic("level1", volumeLevel);
                currentMusic = "level1";
                break;
            case 2:
                assets->PlayMusic("level2", volumeLevel);
                currentMusic = "level2";
                break;
            case 3:
            case 4:
                if (currentLevel == 4 && finalBossDefeated) {
                    assets->PlayMusic("level3-4", volumeLevel);
                    currentMusic = "level3-4";
                } else {
                    assets->PlayMusic("level3-4", volumeLevel);
                    currentMusic = "level3-4";
                }
                break;
            default:
                assets->PlayMusic("level1", volumeLevel);
                currentMusic = "level1";
                break;
        }
    }
    
    setLevelParameters(currentLevel);
    
    std::string terrainTexture = "terrainlvl" + std::to_string(levelNum);
    std::string mapPath;
    
    if (currentLevel == 4 && level4MapChanged) {
        mapPath = "./assets/lvl4/Level4MapAfter.map";
    } else {
        mapPath = "./assets/lvl" + std::to_string(levelNum) + "/Level" + std::to_string(levelNum) + "Map.map";
    }
    
    if (Game::assets->GetTexture(terrainTexture) == nullptr) {
        return;
    }
    
    map = new Map(terrainTexture, 2, 32, manager);
    
    map->LoadMap(mapPath, 60, 34);
}

void Game::advanceToNextLevel() {
    gameState = STATE_GAME;
    
    assets->PlaySound("levelTransition", volumeLevel);
    
    transitionManager.startTransition(currentLevel, currentLevel + 1);
    
    if (transitionLabel == nullptr) {
        transitionLabel = &manager.addEntity();
        transitionLabel->addComponent<UILabel>(0, 0, "", "font2", white);
    }
    
    lastRecordedPosition = Vector2D(0, 0);
}

void Game::initMainMenu() {
    selectedMenuItem = MENU_NEW_GAME;
    menuItemSelected = false;
    menuHighlightActive = false;
    
    if (assets) {
        if (currentMusic != "mainmenu") {
            assets->StopMusic();
            assets->PlayMusic("mainmenu", volumeLevel);
            currentMusic = "mainmenu";
        }
    }
    
    menuTitle = &manager.addEntity();
    menuNewGameButton = &manager.addEntity();
    menuLoadGameButton = &manager.addEntity();
    menuSettingsButton = &manager.addEntity();
    menuLeaderboardButton = &manager.addEntity();
    menuExitButton = &manager.addEntity();
    
    menuTitle->addComponent<UILabel>(0, 200, "Dejte mi RPA 3 prosm", "font2", white);
    
    menuNewGameButton->addComponent<UILabel>(0, 400, "NEW GAME", "font1", white);
    menuLoadGameButton->addComponent<UILabel>(0, 470, "LOAD GAME", "font1", white);
    menuSettingsButton->addComponent<UILabel>(0, 540, "SETTINGS", "font1", white);
    menuLeaderboardButton->addComponent<UILabel>(0, 610, "LEADERBOARD", "font1", white);
    menuExitButton->addComponent<UILabel>(0, 680, "EXIT", "font1", white);
    
    int titleWidth = menuTitle->getComponent<UILabel>().GetWidth();
    int newGameWidth = menuNewGameButton->getComponent<UILabel>().GetWidth();
    int loadGameWidth = menuLoadGameButton->getComponent<UILabel>().GetWidth();
    int settingsWidth = menuSettingsButton->getComponent<UILabel>().GetWidth();
    int leaderboardWidth = menuLeaderboardButton->getComponent<UILabel>().GetWidth();
    int exitWidth = menuExitButton->getComponent<UILabel>().GetWidth();
    
    int titleX = (1920 - titleWidth) / 2;
    int newGameX = (1920 - newGameWidth) / 2;
    int loadGameX = (1920 - loadGameWidth) / 2;
    int settingsX = (1920 - settingsWidth) / 2;
    int leaderboardX = (1920 - leaderboardWidth) / 2;
    int exitX = (1920 - exitWidth) / 2;
    
    menuTitle->getComponent<UILabel>().SetPosition(titleX, 200);
    menuNewGameButton->getComponent<UILabel>().SetPosition(newGameX, 400);
    menuLoadGameButton->getComponent<UILabel>().SetPosition(loadGameX, 470);
    menuSettingsButton->getComponent<UILabel>().SetPosition(settingsX, 540);
    menuLeaderboardButton->getComponent<UILabel>().SetPosition(leaderboardX, 610);
    menuExitButton->getComponent<UILabel>().SetPosition(exitX, 680);
    
    menuNewGameButton->getComponent<UILabel>().SetClickable(true);
    menuNewGameButton->getComponent<UILabel>().SetOnClick([this]() { startGame(); });
    menuNewGameButton->getComponent<UILabel>().SetHoverColor(yellow);
    menuNewGameButton->getComponent<UILabel>().ResetHoverState();
    
    menuLoadGameButton->getComponent<UILabel>().SetClickable(true);
    menuLoadGameButton->getComponent<UILabel>().SetOnClick([this]() { loadGame(); });
    menuLoadGameButton->getComponent<UILabel>().SetHoverColor(yellow);
    menuLoadGameButton->getComponent<UILabel>().ResetHoverState();
    
    menuSettingsButton->getComponent<UILabel>().SetClickable(true);
    menuSettingsButton->getComponent<UILabel>().SetOnClick([this]() { 
        previousState = STATE_MAIN_MENU;
        gameState = STATE_SETTINGS;
        initSettingsMenu();
    });
    menuSettingsButton->getComponent<UILabel>().SetHoverColor(yellow);
    menuSettingsButton->getComponent<UILabel>().ResetHoverState();
    
    menuLeaderboardButton->getComponent<UILabel>().SetClickable(true);
    menuLeaderboardButton->getComponent<UILabel>().SetOnClick([this]() { 
        gameState = STATE_LEADERBOARD;
        initLeaderboard();
    });
    menuLeaderboardButton->getComponent<UILabel>().SetHoverColor(yellow);
    menuLeaderboardButton->getComponent<UILabel>().ResetHoverState();
    
    menuExitButton->getComponent<UILabel>().SetClickable(true);
    menuExitButton->getComponent<UILabel>().SetOnClick([this]() { isRunning = false; });
    menuExitButton->getComponent<UILabel>().SetHoverColor(yellow);
    menuExitButton->getComponent<UILabel>().ResetHoverState();
}

void Game::updateMainMenu() {
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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    menuTitle->draw();
    menuNewGameButton->draw();
    menuLoadGameButton->draw();
    menuSettingsButton->draw();
    menuLeaderboardButton->draw();
    menuExitButton->draw();
    
    SDL_RenderPresent(renderer);
}

void Game::startGame() {
    promptPlayerName();
}

void Game::loadGame() {
    std::ifstream saveFile("assets/savegame.bin", std::ios::binary);
    if (!saveFile.is_open()) {
        std::cout << "No save file found or could not open. Starting new game." << std::endl;
        gameOver = false;
        playerWon = false;
        questionActive = false;
        collectedClues = 0;
        currentLevel = 1;
        gameplayTime = 0;
        level4MapChanged = false;
        finalBossDefeated = false;
        scientistRescued = false;
        canRescueScientist = false;
        usedQuestions.clear();
        
        if (playerName.empty()) {
            playerName = "Player";
        }
        
        gameState = STATE_GAME;
        loadLevel(currentLevel);
        initEntities();
        gameStartTime = SDL_GetTicks();
        
        if (assets) {
            assets->StopMusic();
            assets->PlayMusic("level1", volumeLevel);
            currentMusic = "level1";
        }
        
        return;
    }
    
    if (!assets) {
        std::cerr << "Assets manager is null. Cannot load game." << std::endl;
        gameState = STATE_MAIN_MENU;
        initMainMenu();
        return;
    }

    try {
        float playerX, playerY;
        int playerHealth, playerCurrentAmmo, playerMaxAmmo;
        int loadedClues, loadedLevel;
        Uint32 loadedGameplayTime;
        bool loadedLvl4MapChanged, loadedFinalBossDefeated, loadedScientistRescued, loadedCanRescueScientist;
        std::vector<int> loadedUsedQuestions;
        std::vector<EnemySaveData> loadedEnemyData;
        std::vector<ObjectSaveData> loadedObjectData;

        bool loadSuccess = true;

        saveFile.read(reinterpret_cast<char*>(&playerX), sizeof(playerX));
        saveFile.read(reinterpret_cast<char*>(&playerY), sizeof(playerY));
        saveFile.read(reinterpret_cast<char*>(&playerHealth), sizeof(playerHealth));
        saveFile.read(reinterpret_cast<char*>(&playerCurrentAmmo), sizeof(playerCurrentAmmo));
        saveFile.read(reinterpret_cast<char*>(&playerMaxAmmo), sizeof(playerMaxAmmo));
        loadSuccess &= saveFile.good();

        std::string loadedPlayerName;
        if (readString(saveFile, loadedPlayerName)) {
            playerName = loadedPlayerName;
        } else {
            loadSuccess = false;
        }

        saveFile.read(reinterpret_cast<char*>(&loadedClues), sizeof(loadedClues));
        saveFile.read(reinterpret_cast<char*>(&loadedLevel), sizeof(loadedLevel));
        saveFile.read(reinterpret_cast<char*>(&loadedGameplayTime), sizeof(loadedGameplayTime));
        saveFile.read(reinterpret_cast<char*>(&loadedLvl4MapChanged), sizeof(loadedLvl4MapChanged));
        saveFile.read(reinterpret_cast<char*>(&loadedFinalBossDefeated), sizeof(loadedFinalBossDefeated));
        saveFile.read(reinterpret_cast<char*>(&loadedScientistRescued), sizeof(loadedScientistRescued));
        saveFile.read(reinterpret_cast<char*>(&loadedCanRescueScientist), sizeof(loadedCanRescueScientist));
        loadSuccess &= saveFile.good();

        if (loadedLevel < 1 || loadedLevel > maxLevels) {
            std::cerr << "Invalid level in save file: " << loadedLevel << std::endl;
            loadSuccess = false;
        }

        size_t numUsedQuestions;
        saveFile.read(reinterpret_cast<char*>(&numUsedQuestions), sizeof(numUsedQuestions));
        loadSuccess &= saveFile.good();
        
        if (numUsedQuestions > 100) {
            std::cerr << "Too many used questions in save: " << numUsedQuestions << std::endl;
            loadSuccess = false;
        }
        
        if (loadSuccess) {
            loadedUsedQuestions.resize(numUsedQuestions);
            for (size_t i = 0; i < numUsedQuestions; ++i) {
                saveFile.read(reinterpret_cast<char*>(&loadedUsedQuestions[i]), sizeof(int));
                if (!saveFile.good()) { loadSuccess = false; break; }
            }
        }

        size_t numEnemies;
        saveFile.read(reinterpret_cast<char*>(&numEnemies), sizeof(numEnemies));
        loadSuccess &= saveFile.good();
        
        if (numEnemies > 100) {
            std::cerr << "Too many enemies in save: " << numEnemies << std::endl;
            loadSuccess = false;
        }
        
        if (loadSuccess) {
            loadedEnemyData.resize(numEnemies);
            for (size_t i = 0; i < numEnemies; ++i) {
                saveFile.read(reinterpret_cast<char*>(&loadedEnemyData[i].x), sizeof(float));
                saveFile.read(reinterpret_cast<char*>(&loadedEnemyData[i].y), sizeof(float));
                saveFile.read(reinterpret_cast<char*>(&loadedEnemyData[i].health), sizeof(int));
                saveFile.read(reinterpret_cast<char*>(&loadedEnemyData[i].isBoss), sizeof(bool));
                if (!saveFile.good()) { loadSuccess = false; break; }
            }
        }

        size_t numObjects;
        saveFile.read(reinterpret_cast<char*>(&numObjects), sizeof(numObjects));
        loadSuccess &= saveFile.good();
        
        if (numObjects > 100) {
            std::cerr << "Too many objects in save: " << numObjects << std::endl;
            loadSuccess = false;
        }
        
        if (loadSuccess && numObjects > 0) {
            loadedObjectData.resize(numObjects);
            for (size_t i = 0; i < numObjects; ++i) {
                saveFile.read(reinterpret_cast<char*>(&loadedObjectData[i].x), sizeof(float));
                saveFile.read(reinterpret_cast<char*>(&loadedObjectData[i].y), sizeof(float));
                if (!readString(saveFile, loadedObjectData[i].type)) {
                    loadSuccess = false; break;
                }
                if (!saveFile.good()) { loadSuccess = false; break; }
            }
        }

        saveFile.close();

        if (!loadSuccess) {
            std::cerr << "Save file corrupted or incomplete. Starting new game." << std::endl;
            gameState = STATE_MAIN_MENU;
            initMainMenu();
            return;
        }

        std::cout << "Load game successful. Reconstructing state..." << std::endl;

        manager.clear();
        
        if (map != nullptr) {
            delete map;
            map = nullptr;
        }

        gameOver = false;
        playerWon = false;
        questionActive = false;
        pendingClueEntity = nullptr;
        showFeedback = false;
        showingExitInstructions = false;
        damageTimer = 1.0f;
        objectCollisionDelay = 1.0f;
        objectCollisionsEnabled = false;

        menuTitle = nullptr; menuNewGameButton = nullptr; menuLoadGameButton = nullptr;
        menuSettingsButton = nullptr; menuLeaderboardButton = nullptr; menuExitButton = nullptr;
        endTitle = nullptr; endMessage = nullptr; endRestartButton = nullptr;
        endReplayButton = nullptr; endMenuButton = nullptr;
        pauseTitle = nullptr; pauseResumeButton = nullptr; pauseSaveButton = nullptr;
        pauseRestartButton = nullptr; pauseSettingsButton = nullptr; pauseMainMenuButton = nullptr;
        settingsTitle = nullptr; volumeLabel = nullptr; keybindsLabel = nullptr; settingsBackButton = nullptr;

        player = nullptr; finalBoss = nullptr; healthbar = nullptr; ammobar = nullptr;
        gameover = nullptr; clueCounter = nullptr; feedbackLabel = nullptr; scientist = nullptr;
        questionLabel = nullptr; answer1Label = nullptr; answer2Label = nullptr;
        answer3Label = nullptr; answer4Label = nullptr; questionBackground = nullptr;
        timerLabel = nullptr; transitionLabel = nullptr;

        currentLevel = loadedLevel;
        collectedClues = loadedClues;
        gameplayTime = loadedGameplayTime;
        level4MapChanged = loadedLvl4MapChanged;
        finalBossDefeated = loadedFinalBossDefeated;
        scientistRescued = loadedScientistRescued;
        canRescueScientist = loadedCanRescueScientist;

        usedQuestions.clear();
        for (int index : loadedUsedQuestions) {
            usedQuestions.insert(index);
        }

        gameStartTime = SDL_GetTicks() - gameplayTime;

        if (currentLevel == 1) { totalClues = 3; totalMagazines = 3; totalHealthPotions = 2; }
        else if (currentLevel == 2) { totalClues = 5; totalMagazines = 9; totalHealthPotions = 9; }
        else if (currentLevel == 3) { totalClues = 7; totalMagazines = 15; totalHealthPotions = 15; }
        else if (currentLevel == 4) { totalClues = 0; totalMagazines = 7; totalHealthPotions = 9; }
        else {
            std::cerr << "Unexpected level: " << currentLevel << ". Setting to level 1." << std::endl;
            currentLevel = 1;
            totalClues = 3; totalMagazines = 3; totalHealthPotions = 2;
        }

        setLevelParameters(currentLevel);

        std::string terrainTexture = "terrainlvl" + std::to_string(currentLevel);
        std::string mapPath;
        if (currentLevel == 4 && level4MapChanged) {
            mapPath = "./assets/lvl4/Level4MapAfter.map";
        } else {
            mapPath = "./assets/lvl" + std::to_string(currentLevel) + "/Level" + std::to_string(currentLevel) + "Map.map";
        }

        if (assets->GetTexture(terrainTexture) == nullptr) {
            std::cerr << "Error: Failed to find texture " << terrainTexture << " for loaded level. Starting new game." << std::endl;
            gameState = STATE_MAIN_MENU;
            initMainMenu();
            return;
        }

        try {
            map = new Map(terrainTexture, 2, 32, manager);
            map->LoadMap(mapPath, 60, 34);
        }
        catch (const std::exception& e) {
            std::cerr << "Map loading error: " << e.what() << std::endl;
            gameState = STATE_MAIN_MENU;
            initMainMenu();
            return;
        }

        transitionLabel = &manager.addEntity();
        transitionLabel->addComponent<UILabel>(0, 0, "", "font2", white);

        transitionManager.init(this, &manager);
        transitionManager.mTransitionLabel = transitionLabel;

        try {
            healthbar = &manager.addEntity();
            ammobar = &manager.addEntity();
            gameover = &manager.addEntity();
            clueCounter = &manager.addEntity();
            feedbackLabel = &manager.addEntity();
            timerLabel = &manager.addEntity();
            questionLabel = &manager.addEntity();
            answer1Label = &manager.addEntity();
            answer2Label = &manager.addEntity();
            answer3Label = &manager.addEntity();
            answer4Label = &manager.addEntity();
            questionBackground = &manager.addEntity();

            healthbar->addComponent<UILabel>(20, 20, "Health: " + std::to_string(playerHealth), "font1", white);
            ammobar->addComponent<UILabel>(20, 60, "Ammo: " + std::to_string(playerCurrentAmmo), "font1", white);
            clueCounter->addComponent<UILabel>(20, 100, "Clues: " + std::to_string(collectedClues) + "/" + std::to_string(totalClues), "font1", white);
            gameover->addComponent<UILabel>(0, 0, "", "font2", white);
            feedbackLabel->addComponent<UILabel>(0, 650, "", "font2", white);

            Uint32 totalSeconds = gameplayTime / 1000;
            Uint32 minutes = totalSeconds / 60;
            Uint32 seconds = totalSeconds % 60;
            std::stringstream timeSS;
            timeSS << "Time: " << std::setfill('0') << std::setw(2) << minutes
                << ":" << std::setfill('0') << std::setw(2) << seconds;
            timerLabel->addComponent<UILabel>(0, 20, timeSS.str(), "font1", white);
            int timerWidth = timerLabel->getComponent<UILabel>().GetWidth();
            int xPosTimer = (1920 - timerWidth) / 2;
            timerLabel->getComponent<UILabel>().SetPosition(xPosTimer, 20);

            questionLabel->addComponent<UILabel>(0, 300, "", "font1", white);
            answer1Label->addComponent<UILabel>(0, 340, "", "font1", white);
            answer2Label->addComponent<UILabel>(0, 380, "", "font1", white);
            answer3Label->addComponent<UILabel>(0, 420, "", "font1", white);
            answer4Label->addComponent<UILabel>(0, 460, "", "font1", white);
        }
        catch (const std::exception& e) {
            std::cerr << "UI initialization error: " << e.what() << std::endl;
            gameState = STATE_MAIN_MENU;
            initMainMenu();
            return;
        }

        try {
            player = &manager.addEntity();
            player->addComponent<TransformComponent>(playerX, playerY, 32, 32, 3);
            player->addComponent<SpriteComponent>("player", true);
            player->addComponent<ColliderComponent>("player", 21 * 3, 29 * 3, 6 * 3, 4 * 3);
            player->addComponent<HealthComponent>(playerHealth);
            player->addComponent<AmmoComponent>(playerCurrentAmmo, playerMaxAmmo);
            player->addComponent<KeyboardController>();
            player->addGroup(Game::groupPlayers);
        }
        catch (const std::exception& e) {
            std::cerr << "Player creation error: " << e.what() << std::endl;
            gameState = STATE_MAIN_MENU;
            initMainMenu();
            return;
        }

        try {
            for (const auto& data : loadedEnemyData) {
                Entity& enemy = manager.addEntity();
                enemy.addComponent<TransformComponent>(data.x, data.y, 32, 32, 3);
                enemy.addComponent<ColliderComponent>("enemy");
                enemy.addComponent<HealthComponent>(data.health);
                enemy.addComponent<EnemyAIComponent>(manager);
                enemy.addGroup(Game::groupEnemies);

                if (data.isBoss) {
                    enemy.addComponent<SpriteComponent>("boss", true);
                    enemy.getComponent<ColliderComponent>().tag = "boss";
                    enemy.getComponent<TransformComponent>().scale = 4;
                    enemy.getComponent<EnemyAIComponent>().setSpeed(0.5f);
                    finalBoss = &enemy;
                } else {
                    enemy.addComponent<SpriteComponent>("enemy", true);
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Enemy creation error: " << e.what() << std::endl;
            gameState = STATE_MAIN_MENU;
            initMainMenu();
            return;
        }

        try {
            for (const auto& data : loadedObjectData) {
                if (data.type == "scientist") {
                    scientist = &manager.addEntity();
                    scientist->addComponent<TransformComponent>(data.x, data.y, 32, 32, 3);
                    scientist->addComponent<SpriteComponent>("scientist", true);
                    scientist->addComponent<ColliderComponent>("scientist");
                    scientist->getComponent<SpriteComponent>().Play(scientistRescued ? "Idle" : "Locked");
                    scientist->addGroup(Game::groupObjects);
                } else {
                    assets->CreateObject(data.x, data.y, data.type);
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Object creation error: " << e.what() << std::endl;
            gameState = STATE_MAIN_MENU;
            initMainMenu();
            return;
        }

        tiles = &manager.getGroup(Game::groupMap);
        players = &manager.getGroup(Game::groupPlayers);
        enemies = &manager.getGroup(Game::groupEnemies);
        colliders = &manager.getGroup(Game::groupColliders);
        projectiles = &manager.getGroup(Game::groupProjectiles);
        objects = &manager.getGroup(Game::groupObjects);
        ui = &manager.getGroup(Game::groupUI);

        isRecordingPositions = true;
        lastRecordedPosition = player->getComponent<TransformComponent>().position;

        gameState = STATE_GAME;

        if (assets) {
            assets->StopMusic();
            
            switch (currentLevel) {
                case 1:
                    assets->PlayMusic("level1", volumeLevel);
                    currentMusic = "level1";
                    break;
                case 2:
                    assets->PlayMusic("level2", volumeLevel);
                    currentMusic = "level2";
                    break;
                case 3:
                case 4:
                    if (currentLevel == 4 && finalBossDefeated) {
                        assets->PlayMusic("level3-4", volumeLevel);
                        currentMusic = "level3-4";
                    } else {
                        assets->PlayMusic("level3-4", volumeLevel);
                        currentMusic = "level3-4";
                    }
                    break;
                default:
                    assets->PlayMusic("level1", volumeLevel);
                    currentMusic = "level1";
                    break;
            }
        }

        std::cout << "Game loaded successfully!" << std::endl;
    } 
    catch (const std::exception& e) {
        std::cerr << "Exception during game load: " << e.what() << std::endl;
        gameState = STATE_MAIN_MENU;
        initMainMenu();
    } 
    catch (...) {
        std::cerr << "Unknown exception during game load. Starting new game." << std::endl;
        gameState = STATE_MAIN_MENU;
        initMainMenu();
    }
}

void Game::initEndScreen(bool victory) {
    selectedEndOption = END_RESTART;
    endOptionSelected = false;
    endHighlightActive = false;
    
    if (victory) {
        assets->PlaySound("victory", volumeLevel);
    } else {
        assets->PlaySound("gameOver", volumeLevel);
    }
    
    assets->StopMusic();
    
    endTitle = &manager.addEntity();
    endMessage = &manager.addEntity();
    endRestartButton = &manager.addEntity();
    endReplayButton = &manager.addEntity();
    endMenuButton = &manager.addEntity();
    
    endTitle->addComponent<UILabel>(0, 150, victory ? "VICTORY!" : "GAME OVER", "font2", victory ? green : red);
    
    std::string message = victory ? 
        "You have rescued the scientist and completed your mission!" : 
        "You failed to complete your mission. Better luck next time!";
    endMessage->addComponent<UILabel>(0, 300, message, "font1", white);
    
    if (victory && !playerName.empty()) {
        saveToLeaderboard(playerName, gameplayTime);
    }
    
    endRestartButton->addComponent<UILabel>(0, 450, "RESTART GAME", "font1", white);
    endReplayButton->addComponent<UILabel>(0, 500, "REPLAY (NAJBOLJ NEPOTREBNA FUNKCIJA, KI NE DELA)", "font1", white);
    endMenuButton->addComponent<UILabel>(0, 550, "MAIN MENU", "font1", white);
    
    int titleWidth = endTitle->getComponent<UILabel>().GetWidth();
    int messageWidth = endMessage->getComponent<UILabel>().GetWidth();
    int restartWidth = endRestartButton->getComponent<UILabel>().GetWidth();
    int replayWidth = endReplayButton->getComponent<UILabel>().GetWidth();
    int menuWidth = endMenuButton->getComponent<UILabel>().GetWidth();
    
    int titleX = (1920 - titleWidth) / 2;
    int messageX = (1920 - messageWidth) / 2;
    int restartX = (1920 - restartWidth) / 2;
    int replayX = (1920 - replayWidth) / 2;
    int menuX = (1920 - menuWidth) / 2;
    
    endTitle->getComponent<UILabel>().SetPosition(titleX, 150);
    endMessage->getComponent<UILabel>().SetPosition(messageX, 300);
    endRestartButton->getComponent<UILabel>().SetPosition(restartX, 450);
    endReplayButton->getComponent<UILabel>().SetPosition(replayX, 500);
    endMenuButton->getComponent<UILabel>().SetPosition(menuX, 550);
    
    endRestartButton->getComponent<UILabel>().SetClickable(true);
    endRestartButton->getComponent<UILabel>().SetOnClick([this]() { 
        restart();
    });
    endRestartButton->getComponent<UILabel>().SetHoverColor(yellow);
    endRestartButton->getComponent<UILabel>().ResetHoverState();
    
    endReplayButton->getComponent<UILabel>().SetClickable(true);
    endReplayButton->getComponent<UILabel>().SetOnClick([this]() { 
        replay();
    });
    endReplayButton->getComponent<UILabel>().SetHoverColor(yellow);
    endReplayButton->getComponent<UILabel>().ResetHoverState();
    
    endMenuButton->getComponent<UILabel>().SetClickable(true);
    endMenuButton->getComponent<UILabel>().SetOnClick([this]() { 
        returnToMainMenu = true;
    });
    endMenuButton->getComponent<UILabel>().SetHoverColor(yellow);
    endMenuButton->getComponent<UILabel>().ResetHoverState();
    
    gameState = STATE_END_SCREEN;
}

void Game::updateEndScreen() {
    if (endRestartButton && endRestartButton->hasComponent<UILabel>()) {
        endRestartButton->getComponent<UILabel>().ResetHoverState();
    }
    
    if (endReplayButton && endReplayButton->hasComponent<UILabel>()) {
        endReplayButton->getComponent<UILabel>().ResetHoverState();
    }
    
    if (endMenuButton && endMenuButton->hasComponent<UILabel>()) {
        endMenuButton->getComponent<UILabel>().ResetHoverState();
    }
    
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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    endTitle->draw();
    endMessage->draw();
    endRestartButton->draw();
    endReplayButton->draw();
    endMenuButton->draw();
    
    SDL_RenderPresent(renderer);
}

void Game::togglePause() {
    if (gameState == STATE_GAME) {
        gameplayTime = SDL_GetTicks() - gameStartTime;
        
        if (assets) {
            assets->StopMusic();
            assets->PlayMusic("mainmenu", volumeLevel);
            currentMusic = "mainmenu";
        }
        
        gameState = STATE_PAUSE;
        initPauseMenu();
    } 
    else if (gameState == STATE_PAUSE) {
        gameStartTime = SDL_GetTicks() - gameplayTime;
        
        if (assets) {
            assets->StopMusic();
            
            switch (currentLevel) {
                case 1:
                    assets->PlayMusic("level1", volumeLevel);
                    currentMusic = "level1";
                    break;
                case 2:
                    assets->PlayMusic("level2", volumeLevel);
                    currentMusic = "level2";
                    break;
                case 3:
                case 4:
                    if (currentLevel == 4 && finalBossDefeated) {
                        assets->PlayMusic("level3-4", volumeLevel);
                        currentMusic = "level3-4";
                    } else {
                        assets->PlayMusic("level3-4", volumeLevel);
                        if (currentLevel == 4) {
                            bossMusicPlaying = false;
                        }
                    }
                    break;
                default:
                    assets->PlayMusic("level1", volumeLevel);
                    currentMusic = "level1";
                    break;
            }
        }
        
        gameState = STATE_GAME;
        
        if (pauseTitle) pauseTitle->destroy();
        if (pauseResumeButton) pauseResumeButton->destroy();
        if (pauseSaveButton) pauseSaveButton->destroy();
        if (pauseRestartButton) pauseRestartButton->destroy();
        if (pauseSettingsButton) pauseSettingsButton->destroy();
        if (pauseMainMenuButton) pauseMainMenuButton->destroy();
        if (pauseBackground) pauseBackground->destroy();
        
        manager.refresh();
        
        pauseTitle = nullptr;
        pauseResumeButton = nullptr;
        pauseSaveButton = nullptr;
        pauseRestartButton = nullptr;
        pauseSettingsButton = nullptr;
        pauseMainMenuButton = nullptr;
        pauseBackground = nullptr;
    }
}

void Game::initPauseMenu() {
    selectedPauseItem = PAUSE_RESUME;
    pauseItemSelected = false;
    pauseHighlightActive = true;
    
    pauseTitle = &manager.addEntity();
    pauseResumeButton = &manager.addEntity();
    pauseSaveButton = &manager.addEntity();
    pauseRestartButton = &manager.addEntity();
    pauseSettingsButton = &manager.addEntity();
    pauseMainMenuButton = &manager.addEntity();
    
    pauseTitle->addComponent<UILabel>(0, 150, "PAUSED", "font2", white);
    pauseResumeButton->addComponent<UILabel>(0, 300, "RESUME", "font1", white);
    pauseSaveButton->addComponent<UILabel>(0, 360, "SAVE GAME", "font1", white);
    pauseRestartButton->addComponent<UILabel>(0, 420, "RESTART", "font1", white);
    pauseSettingsButton->addComponent<UILabel>(0, 480, "SETTINGS", "font1", white);
    pauseMainMenuButton->addComponent<UILabel>(0, 540, "MAIN MENU", "font1", white);
    
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
    
    pauseResumeButton->getComponent<UILabel>().SetClickable(true);
    pauseResumeButton->getComponent<UILabel>().SetOnClick([this]() { togglePause(); });
    pauseResumeButton->getComponent<UILabel>().SetHoverColor(yellow);
    
    pauseSaveButton->getComponent<UILabel>().SetClickable(true);
    pauseSaveButton->getComponent<UILabel>().SetOnClick([this]() { saveGame(); });
    pauseSaveButton->getComponent<UILabel>().SetHoverColor(yellow);
    
    pauseRestartButton->getComponent<UILabel>().SetClickable(true);
    pauseRestartButton->getComponent<UILabel>().SetOnClick([this]() { 
        restart();
    });
    pauseRestartButton->getComponent<UILabel>().SetHoverColor(yellow);
    
    pauseSettingsButton->getComponent<UILabel>().SetClickable(true);
    pauseSettingsButton->getComponent<UILabel>().SetOnClick([this]() { 
        previousState = STATE_PAUSE;
        gameState = STATE_SETTINGS;
        initSettingsMenu();
    });
    pauseSettingsButton->getComponent<UILabel>().SetHoverColor(yellow);
    
    pauseMainMenuButton->getComponent<UILabel>().SetClickable(true);
    pauseMainMenuButton->getComponent<UILabel>().SetOnClick([this]() { returnToMainMenu = true; });
    pauseMainMenuButton->getComponent<UILabel>().SetHoverColor(yellow);
    
    updatePauseMenu();
}

void Game::updatePauseMenu() {
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
    for(auto& t : *tiles) t->draw();
    for(auto& p : *players) p->draw();
    for(auto& e : *enemies) e->draw();
    for(auto& o : *objects) o->draw();
    for(auto& p : *projectiles) p->draw();
    
    if (scientist != nullptr && scientist->isActive()) {
        scientist->draw();
    }
    
    healthbar->draw();
    ammobar->draw();
    clueCounter->draw();
    timerLabel->draw();
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect fullscreen = {0, 0, 1920, 1080};
    SDL_RenderFillRect(renderer, &fullscreen);
    
    if (pauseTitle) pauseTitle->draw();
    if (pauseResumeButton) pauseResumeButton->draw();
    if (pauseSaveButton) pauseSaveButton->draw();
    if (pauseRestartButton) pauseRestartButton->draw();
    if (pauseSettingsButton) pauseSettingsButton->draw();
    if (pauseMainMenuButton) pauseMainMenuButton->draw();
    
    SDL_RenderPresent(renderer);
}

void Game::saveGame() {
    if (!player || !player->isActive()) {
        std::cout << "Cannot save game: Player does not exist." << std::endl;
        return;
    }

    // Validate the current level is within range
    if (currentLevel < 1 || currentLevel > maxLevels) {
        std::cerr << "Error: Invalid current level value: " << currentLevel << ". Cannot save game." << std::endl;
        return;
    }

    std::ofstream saveFile("assets/savegame.bin", std::ios::binary | std::ios::trunc);
    if (!saveFile.is_open()) {
        std::cerr << "Error: Could not open savegame.bin for writing!" << std::endl;
        return;
    }

    TransformComponent& playerTransform = player->getComponent<TransformComponent>();
    HealthComponent& playerHealth = player->getComponent<HealthComponent>();
    AmmoComponent& playerAmmo = player->getComponent<AmmoComponent>();
    float pX = playerTransform.position.x;
    float pY = playerTransform.position.y;
    int pHealth = playerHealth.health;
    int pCurrentAmmo = playerAmmo.currentAmmo;
    int pMaxAmmo = playerAmmo.maxAmmo;

    std::vector<int> questionsToSave(usedQuestions.begin(), usedQuestions.end());

    std::vector<EnemySaveData> enemiesToSave;
    manager.refresh();
    enemies = &manager.getGroup(Game::groupEnemies);
    for (const auto& enemy : *enemies) {
        if (enemy->isActive()) {
            TransformComponent& et = enemy->getComponent<TransformComponent>();
            HealthComponent& eh = enemy->getComponent<HealthComponent>();
            bool isBoss = (enemy == finalBoss);
            enemiesToSave.push_back({et.position.x, et.position.y, eh.health, isBoss});
        }
    }

    std::vector<ObjectSaveData> objectsToSave;
    objects = &manager.getGroup(Game::groupObjects);
    for (const auto& object : *objects) {
        if (object->isActive() && object->hasComponent<ColliderComponent>()) {
            TransformComponent& ot = object->getComponent<TransformComponent>();
            std::string type = object->getComponent<ColliderComponent>().tag;

             if (object == scientist) {
                 type = "scientist";
             }

            if (type == "clue" || type == "magazine" || type == "healthpotion" || type == "scientist" || type == "cactus") {
                 objectsToSave.push_back({ot.position.x, ot.position.y, type});
            }
        }
    }

    saveFile.write(reinterpret_cast<const char*>(&pX), sizeof(pX));
    saveFile.write(reinterpret_cast<const char*>(&pY), sizeof(pY));
    saveFile.write(reinterpret_cast<const char*>(&pHealth), sizeof(pHealth));
    saveFile.write(reinterpret_cast<const char*>(&pCurrentAmmo), sizeof(pCurrentAmmo));
    saveFile.write(reinterpret_cast<const char*>(&pMaxAmmo), sizeof(pMaxAmmo));
    
    writeString(saveFile, playerName);

    saveFile.write(reinterpret_cast<const char*>(&collectedClues), sizeof(collectedClues));
    saveFile.write(reinterpret_cast<const char*>(&currentLevel), sizeof(currentLevel));
    saveFile.write(reinterpret_cast<const char*>(&gameplayTime), sizeof(gameplayTime));
    saveFile.write(reinterpret_cast<const char*>(&level4MapChanged), sizeof(level4MapChanged));
    saveFile.write(reinterpret_cast<const char*>(&finalBossDefeated), sizeof(finalBossDefeated));
    saveFile.write(reinterpret_cast<const char*>(&scientistRescued), sizeof(scientistRescued));
    saveFile.write(reinterpret_cast<const char*>(&canRescueScientist), sizeof(canRescueScientist));

    size_t numUsedQuestions = questionsToSave.size();
    saveFile.write(reinterpret_cast<const char*>(&numUsedQuestions), sizeof(numUsedQuestions));
    for (int questionIndex : questionsToSave) {
        saveFile.write(reinterpret_cast<const char*>(&questionIndex), sizeof(questionIndex));
    }

    size_t numEnemies = enemiesToSave.size();
    saveFile.write(reinterpret_cast<const char*>(&numEnemies), sizeof(numEnemies));
    for (const auto& data : enemiesToSave) {
        saveFile.write(reinterpret_cast<const char*>(&data.x), sizeof(float));
        saveFile.write(reinterpret_cast<const char*>(&data.y), sizeof(float));
        saveFile.write(reinterpret_cast<const char*>(&data.health), sizeof(int));
        saveFile.write(reinterpret_cast<const char*>(&data.isBoss), sizeof(bool));
    }

    size_t numObjects = objectsToSave.size();
    saveFile.write(reinterpret_cast<const char*>(&numObjects), sizeof(numObjects));
    for (const auto& data : objectsToSave) {
        saveFile.write(reinterpret_cast<const char*>(&data.x), sizeof(float));
        saveFile.write(reinterpret_cast<const char*>(&data.y), sizeof(float));
        writeString(saveFile, data.type);
    }

    saveFile.close();

    std::cout << "Game saved successfully." << std::endl;

    if (feedbackLabel && feedbackLabel->hasComponent<UILabel>()) {
        bool isExitInstructions = 
            showingExitInstructions && 
            ((currentLevel != 4 && collectedClues >= totalClues) || 
            (currentLevel == 4 && finalBossDefeated));
            
        if (isExitInstructions) {
            if (currentLevel == 3) {
                savedExitInstructionsText = "All clues collected! Enter the pyramid to see what lies ahead.";
            } else if (currentLevel == 4 && level4MapChanged) {
                savedExitInstructionsText = "BOSS DEFEATED! The path is revealed! Find and rescue the SUPERUM!";
            } else {
                savedExitInstructionsText = "All clues collected! Head NORTH to exit the level.";
            }
            hasSavedDuringExitInstructions = true;
        }
    
        feedbackLabel->getComponent<UILabel>().SetLabelText("Game Saved!", "font1", green);
        int feedbackWidth = feedbackLabel->getComponent<UILabel>().GetWidth();
        int xPos = (1920 - feedbackWidth) / 2;
        feedbackLabel->getComponent<UILabel>().SetPosition(xPos, 950);
        showFeedback = true;
        feedbackStartTime = SDL_GetTicks();
    } else {
        std::cout << "Feedback label not available for save notification." << std::endl;
    }
}

void Game::initSettingsMenu() {
    selectedSettingsItem = SETTINGS_VOLUME;
    settingsItemSelected = false;
    settingsHighlightActive = true;
    draggingVolumeSlider = false;
    
    settingsTitle = &manager.addEntity();
    volumeLabel = &manager.addEntity();
    keybindsLabel = &manager.addEntity();
    settingsBackButton = &manager.addEntity();
    
    settingsTitle->addComponent<UILabel>(0, 150, "SETTINGS", "font2", white);
    
    std::stringstream volSS;
    volSS << "Volume: " << volumeLevel << "%";
    volumeLabel->addComponent<UILabel>(0, 300, volSS.str(), "font1", white);
    
    keybindsLabel->addComponent<UILabel>(0, 400, "CONTROLS", "font1", white);
    
    settingsBackButton->addComponent<UILabel>(0, 800, "BACK", "font1", white);
    
    int titleWidth = settingsTitle->getComponent<UILabel>().GetWidth();
    int volLabelWidth = volumeLabel->getComponent<UILabel>().GetWidth();
    int keybindsWidth = keybindsLabel->getComponent<UILabel>().GetWidth();
    int backWidth = settingsBackButton->getComponent<UILabel>().GetWidth();
    
    int titleX = (1920 - titleWidth) / 2;
    int volLabelX = (1920 - volLabelWidth) / 2;
    int keybindsX = (1920 - keybindsWidth) / 2;
    int backX = (1920 - backWidth) / 2;
    
    settingsTitle->getComponent<UILabel>().SetPosition(titleX, 150);
    volumeLabel->getComponent<UILabel>().SetPosition(volLabelX, 300);
    keybindsLabel->getComponent<UILabel>().SetPosition(keybindsX, 400);
    settingsBackButton->getComponent<UILabel>().SetPosition(backX, 800);
    
    settingsBackButton->getComponent<UILabel>().SetClickable(true);
    settingsBackButton->getComponent<UILabel>().SetOnClick([this]() { 
        gameState = previousState; 
        applySettings();
    });
    settingsBackButton->getComponent<UILabel>().SetHoverColor(yellow);
    
    updateSettingsMenu();
}

void Game::updateSettingsMenu() {
    if (volumeLabel && volumeLabel->hasComponent<UILabel>()) {
        std::stringstream volSS;
        volSS << "Volume: " << volumeLevel << "%";
        volumeLabel->getComponent<UILabel>().SetLabelText(volSS.str(), "font1");
        
        int volLabelWidth = volumeLabel->getComponent<UILabel>().GetWidth();
        int volLabelX = (1920 - volLabelWidth) / 2;
        volumeLabel->getComponent<UILabel>().SetPosition(volLabelX, 300);
    }
    
    if (settingsBackButton && settingsBackButton->hasComponent<UILabel>()) {
        settingsBackButton->getComponent<UILabel>().ResetHoverState();
    }
    
    if (settingsHighlightActive) {
        switch (selectedSettingsItem) {
            case SETTINGS_VOLUME:
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
    if (previousState == STATE_PAUSE) {
        for(auto& t : *tiles) t->draw();
        for(auto& p : *players) p->draw();
        for(auto& e : *enemies) e->draw();
        for(auto& o : *objects) o->draw();
        for(auto& p : *projectiles) p->draw();
        
        if (scientist != nullptr && scientist->isActive()) {
            scientist->draw();
        }
        
        healthbar->draw();
        ammobar->draw();
        clueCounter->draw();
        timerLabel->draw();
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
    }
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect fullscreen = {0, 0, 1920, 1080};
    SDL_RenderFillRect(renderer, &fullscreen);
    
    if (settingsTitle) settingsTitle->draw();
    if (volumeLabel) volumeLabel->draw();
    
    int screenCenter = 1920 / 2;
    int sliderY = 340;
    int sliderWidth = 400;
    int sliderHeight = 20;
    int sliderX = screenCenter - (sliderWidth / 2);
    
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    SDL_Rect sliderBg = {sliderX, sliderY, sliderWidth, sliderHeight};
    SDL_RenderFillRect(renderer, &sliderBg);
    
    int filledWidth = (volumeLevel * sliderWidth) / 100;
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_Rect filledRect = {sliderX, sliderY, filledWidth, sliderHeight};
    SDL_RenderFillRect(renderer, &filledRect);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &sliderBg);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int knobX = sliderX + filledWidth - 4;
    SDL_Rect knob = {knobX, sliderY - 5, 8, sliderHeight + 10};
    SDL_RenderFillRect(renderer, &knob);
    
    if (settingsHighlightActive && selectedSettingsItem == SETTINGS_VOLUME) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_Rect highlightRect = {sliderX - 2, sliderY - 2, sliderWidth + 4, sliderHeight + 4};
        SDL_RenderDrawRect(renderer, &highlightRect);
    }
    
    if (keybindsLabel) {
        keybindsLabel->draw();
        
        TTF_Font* font = assets->GetFont("font1");
        if (font) {
            int yOffset = 450;
            int screenCenter = 1920 / 2;
            
            for (const auto& keybind : keybinds) {
                std::string bindText = keybind.action + ": " + keybind.key;
                
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, bindText.c_str(), white);
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    if (textTexture) {
                        int textWidth = textSurface->w;
                        int textHeight = textSurface->h;
                        int xPos = screenCenter - (textWidth / 2);
                        
                        SDL_Rect destRect = {xPos, yOffset, textWidth, textHeight};
                        SDL_RenderCopy(renderer, textTexture, NULL, &destRect);
                        
                        SDL_DestroyTexture(textTexture);
                    }
                    SDL_FreeSurface(textSurface);
                }
                
                yOffset += 45;
            }
        }
    }
    
    if (settingsBackButton) settingsBackButton->draw();
    
    SDL_RenderPresent(renderer);
}

void Game::applySettings() {
    if (assets) {
        assets->SetMasterVolume(volumeLevel);
    }
    
    if (settingsTitle) settingsTitle->destroy();
    if (volumeLabel) volumeLabel->destroy();
    if (keybindsLabel) keybindsLabel->destroy();
    if (settingsBackButton) settingsBackButton->destroy();
    
    settingsTitle = nullptr;
    volumeLabel = nullptr;
    keybindsLabel = nullptr;
    settingsBackButton = nullptr;
    
    selectedSettingsItem = SETTINGS_VOLUME;
    settingsHighlightActive = false;
    settingsItemSelected = false;
    draggingVolumeSlider = false;
}

void Game::replay() {
    gameState = STATE_REPLAY;
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

    if (replayEntity) { replayEntity->destroy(); replayEntity = nullptr; }
    replayEntity = &manager.addEntity();
    replayEntity->addComponent<TransformComponent>(startPos.x, startPos.y, 32, 32, 3);
    replayEntity->addComponent<SpriteComponent>("player", true);
    replayEntity->addGroup(Game::groupPlayers);

    if (timerLabel) { timerLabel->destroy(); timerLabel = nullptr; }
    timerLabel = &manager.addEntity();
    timerLabel->addComponent<UILabel>(20, 20, " ", "font1", white); 
    timerLabel->addGroup(Game::groupUI);
    replayEntity->getComponent<SpriteComponent>().Play("Idle");
}

void Game::updateReplay() {
    if (!isReplaying || !replayEntity || !replayEntity->hasComponent<TransformComponent>()) return;

    if (replayEntity->hasComponent<SpriteComponent>()) { replayEntity->getComponent<SpriteComponent>().update(); }
    Uint32 currentTime = SDL_GetTicks();

    if (allReplayPositionsByLevel.find(currentReplayLevel) == allReplayPositionsByLevel.end() || allReplayPositionsByLevel[currentReplayLevel].empty()) { /* End replay logic ... */ return; }
    const auto& currentLevelPositions = allReplayPositionsByLevel[currentReplayLevel];

    if (currentTime - lastReplayFrameTime >= replayFrameTime) {
        auto& transform = replayEntity->getComponent<TransformComponent>();
        float currentX = transform.position.x; float currentY = transform.position.y;
        Vector2D nextPos = currentLevelPositions[replayPositionIndex];
        float deltaX = nextPos.x - currentX; float deltaY = nextPos.y - currentY;

        transform.position.x = nextPos.x; transform.position.y = nextPos.y;
        transform.velocity.x = 0; transform.velocity.y = 0;
        replayPositionIndex++;
        lastReplayFrameTime = currentTime;
    }
}

void Game::renderReplay() {
    SDL_RenderClear(renderer);
    for(auto& t : *tiles) t->draw();

    if (replayEntity && replayEntity->hasComponent<TransformComponent>()) {
        replayEntity->draw();
    }

    if (timerLabel) {
        timerLabel->draw();
    }

    SDL_RenderPresent(renderer);
}

void Game::recordPlayerPosition() {
    if (player && isRecordingPositions && gameState == STATE_GAME) {
        Vector2D currentPosition = player->getComponent<TransformComponent>().position;
        
        const float recordThreshold = 0.1f;
        if (std::abs(currentPosition.x - lastRecordedPosition.x) >= recordThreshold || 
            std::abs(currentPosition.y - lastRecordedPosition.y) >= recordThreshold) {
            
            std::ofstream positionFile("assets/position.txt", std::ios::app);
            
            if (positionFile.is_open()) {
                positionFile << static_cast<int>(currentPosition.x) << "," 
                             << static_cast<int>(currentPosition.y) << "," 
                             << currentLevel << std::endl;
                
                lastRecordedPosition = currentPosition;
            }
            positionFile.close();
        }
    }
}

void Game::promptPlayerName() {
    SDL_StopTextInput();
    SDL_StartTextInput();
    
    waitingForPlayerName = true;
    playerName = "";
    
    Entity* namePrompt = &manager.addEntity();
    namePrompt->addComponent<UILabel>(0, 400, "Enter your name:", "font1", white);
    
    int promptWidth = namePrompt->getComponent<UILabel>().GetWidth();
    int promptX = (1920 - promptWidth) / 2;
    namePrompt->getComponent<UILabel>().SetPosition(promptX, 400);
    
    Entity* inputBox = &manager.addEntity();
    inputBox->addComponent<UILabel>(0, 450, "_", "font1", white);
    
    int inputWidth = inputBox->getComponent<UILabel>().GetWidth();
    int inputX = (1920 - inputWidth) / 2;
    inputBox->getComponent<UILabel>().SetPosition(inputX, 450);
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    namePrompt->draw();
    inputBox->draw();
    SDL_RenderPresent(renderer);
    
    bool enteringName = true;
    while (enteringName && isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
                enteringName = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN) {
                    enteringName = false;
                    if (playerName.empty()) {
                        playerName = "Player";
                    }
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE && !playerName.empty()) {
                    playerName.pop_back();
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    enteringName = false;
                    playerName = "Player";
                }
            }
            else if (event.type == SDL_TEXTINPUT) {
                if (playerName.length() < 15) {
                    playerName += event.text.text;
                }
            }
            
            std::string displayText = playerName + "_";
            inputBox->getComponent<UILabel>().SetLabelText(displayText, "font1");
            
            inputWidth = inputBox->getComponent<UILabel>().GetWidth();
            inputX = (1920 - inputWidth) / 2;
            inputBox->getComponent<UILabel>().SetPosition(inputX, 450);
            
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            namePrompt->draw();
            inputBox->draw();
            SDL_RenderPresent(renderer);
        }
        
        SDL_Delay(16);
    }
    
    SDL_StopTextInput();
    waitingForPlayerName = false;
    
    namePrompt->destroy();
    inputBox->destroy();
    manager.refresh();
    
    gameOver = false;
    playerWon = false;
    collectedClues = 0;
    damageTimer = 1.0f;
    hurtSoundTimer = 0.0f;
    objectCollisionDelay = 1.0f;
    objectCollisionsEnabled = false;
    questionActive = false;
    pendingClueEntity = nullptr;
    showFeedback = false;
    showingExitInstructions = false;
    level4MapChanged = false;
    finalBossDefeated = false;
    bossMusicPlaying = false;
    scientistRescued = false;
    canRescueScientist = false;
    currentLevel = 1;
    
    resetUsedQuestions();
    
    std::ofstream positionFile("assets/position.txt", std::ios::trunc);
    positionFile.close();
    isRecordingPositions = true;
    lastRecordedPosition = Vector2D(0, 0);
    
    gameState = STATE_GAME;
    gameStartTime = SDL_GetTicks();
    gameplayTime = 0;
    
    menuTitle = nullptr;
    menuNewGameButton = nullptr;
    menuLoadGameButton = nullptr;
    menuSettingsButton = nullptr;
    menuLeaderboardButton = nullptr;
    menuExitButton = nullptr;
    
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
    
    manager.clear();
    
    transitionLabel = &manager.addEntity();
    transitionLabel->addComponent<UILabel>(0, 0, "", "font2", white);
    transitionManager.mTransitionLabel = transitionLabel;
    
    transitionManager.init(this, &manager);
    
    currentLevel = 1;
    
    positionManager.resetPositions();
    
    loadLevel(currentLevel);
    initEntities();
}

void Game::initLeaderboard() {
    leaderboardEntries.clear();
    
    std::ifstream file("assets/leaderboard.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line) && leaderboardEntries.size() < 5) {
            size_t commaPos = line.find(',');
            if (commaPos != std::string::npos) {
                std::string name = line.substr(0, commaPos);
                std::string time = line.substr(commaPos + 1);
                leaderboardEntries.push_back(std::make_pair(name, time));
            }
        }
        file.close();
    }
    
    leaderboardTitle = &manager.addEntity();
    leaderboardTitle->addComponent<UILabel>(0, 150, "LEADERBOARD", "font2", white);
    
    int titleWidth = leaderboardTitle->getComponent<UILabel>().GetWidth();
    int titleX = (1920 - titleWidth) / 2;
    leaderboardTitle->getComponent<UILabel>().SetPosition(titleX, 150);
    
    if (leaderboardEntries.empty()) {
        leaderboardEntryLabels[0] = &manager.addEntity();
        leaderboardEntryLabels[0]->addComponent<UILabel>(0, 300, "No entries yet", "font1", white);
        
        int msgWidth = leaderboardEntryLabels[0]->getComponent<UILabel>().GetWidth();
        int msgX = (1920 - msgWidth) / 2;
        leaderboardEntryLabels[0]->getComponent<UILabel>().SetPosition(msgX, 300);
    } 
    else {
        for (int i = 0; i < leaderboardEntries.size(); i++) {
            std::string entryText = "RUN " + std::to_string(i + 1) + ": " + leaderboardEntries[i].first + " " + leaderboardEntries[i].second;
            leaderboardEntryLabels[i] = &manager.addEntity();
            leaderboardEntryLabels[i]->addComponent<UILabel>(0, 250 + i * 80, entryText, "font1", white);
            
            int entryWidth = leaderboardEntryLabels[i]->getComponent<UILabel>().GetWidth();
            int entryX = (1920 - entryWidth) / 2;
            leaderboardEntryLabels[i]->getComponent<UILabel>().SetPosition(entryX, 250 + i * 80);
        }
    }
    
    leaderboardBackButton = &manager.addEntity();
    leaderboardBackButton->addComponent<UILabel>(0, 700, "PRESS ESC TO RETURN", "font1", white);
    
    int backWidth = leaderboardBackButton->getComponent<UILabel>().GetWidth();
    int backX = (1920 - backWidth) / 2;
    leaderboardBackButton->getComponent<UILabel>().SetPosition(backX, 700);
    
    leaderboardBackButton->getComponent<UILabel>().SetClickable(true);
    leaderboardBackButton->getComponent<UILabel>().SetOnClick([this]() {
        gameState = STATE_MAIN_MENU;
        
        if (leaderboardTitle) leaderboardTitle->destroy();
        for (auto& label : leaderboardEntryLabels) {
            if (label) label->destroy();
            label = nullptr;
        }
        leaderboardBackButton->destroy();
        
        initMainMenu();
    });
    leaderboardBackButton->getComponent<UILabel>().SetHoverColor(yellow);
}

void Game::renderLeaderboard() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    leaderboardTitle->draw();
    for (auto& label : leaderboardEntryLabels) {
        if (label) label->draw();
    }
    leaderboardBackButton->draw();
    
    SDL_RenderPresent(renderer);
}

void Game::updateLeaderboard() {
    if (leaderboardBackButton && leaderboardBackButton->hasComponent<UILabel>()) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        bool isOver = leaderboardBackButton->getComponent<UILabel>().IsMouseOver(mouseX, mouseY);
        
        if (isOver) {
            leaderboardBackButton->getComponent<UILabel>().SetTextColor(yellow);
        } else {
            leaderboardBackButton->getComponent<UILabel>().SetTextColor(white);
        }
    }
}

void Game::saveToLeaderboard(const std::string& playerName, Uint32 gameTime) {
    Uint32 totalSeconds = gameTime / 1000;
    Uint32 minutes = totalSeconds / 60;
    Uint32 seconds = totalSeconds % 60;
    
    std::string formattedTime = 
        (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" + 
        (seconds < 10 ? "0" : "") + std::to_string(seconds);
    
    std::vector<std::pair<std::string, std::string>> entries;
    
    std::ifstream inFile("assets/leaderboard.txt");
    if (inFile.is_open()) {
        std::string line;
        while (std::getline(inFile, line) && entries.size() < 5) {
            size_t commaPos = line.find(',');
            if (commaPos != std::string::npos) {
                std::string name = line.substr(0, commaPos);
                std::string time = line.substr(commaPos + 1);
                entries.push_back(std::make_pair(name, time));
            }
        }
        inFile.close();
    }
    
    entries.push_back(std::make_pair(playerName, formattedTime));
    
    if (entries.size() > 5) {
        entries.erase(entries.begin());
    }
    
    std::ofstream outFile("assets/leaderboard.txt");
    if (outFile.is_open()) {
        for (const auto& entry : entries) {
            outFile << entry.first << "," << entry.second << std::endl;
        }
        outFile.close();
    }
    
    if (gameState == STATE_LEADERBOARD) {
        if (leaderboardTitle) leaderboardTitle->destroy();
        for (auto& label : leaderboardEntryLabels) {
            if (label) label->destroy();
            label = nullptr;
        }
        if (leaderboardBackButton) leaderboardBackButton->destroy();
        
        initLeaderboard();
    }
}