#include "Game.hpp"
#include "TextureManager.hpp"
#include "Map.hpp"
#include "ECS/Components.hpp"
#include "Vector2D.hpp"
#include "Collision.hpp"
#include "AssetManager.hpp"
#include <sstream>
#include <random>
#include <ctime>

// Global manager and entities
Map* map;
Manager manager;

// Global entity pointers instead of references
Entity* player = nullptr;
// enemy is now multiple entities, so no global pointer
Entity* healthbar = nullptr;
Entity* ammobar = nullptr;
Entity* gameover = nullptr;
Entity* clueCounter = nullptr;
Entity* feedbackLabel = nullptr;

// Global entity groups
std::vector<Entity*>* tiles;
std::vector<Entity*>* players;
std::vector<Entity*>* enemies;
std::vector<Entity*>* colliders;
std::vector<Entity*>* projectiles;
std::vector<Entity*>* objects;
std::vector<Entity*>* ui;

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
        {"Which of these is NOT a programming paradigm?", {"Object-Oriented", "Functional", "Procedural", "Dialectical"}, 3}
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
        Vector2D cluePos = findRandomCluePosition();
        assets->CreateObject(cluePos.x, cluePos.y, "clue");
    }
    
    // Create magazine pickups at random locations using magazine-specific function
    for (int i = 0; i < totalMagazines; i++) {
        Vector2D magazinePos = findRandomMagazinePosition();
        assets->CreateObject(magazinePos.x, magazinePos.y, "magazine");
    }
    
    // Create health potion pickups at random locations using potion-specific function
    for (int i = 0; i < totalHealthPotions; i++) {
        Vector2D potionPos = findRandomHealthPotionPosition();
        assets->CreateObject(potionPos.x, potionPos.y, "healthpotion");
    }
    
    // Find a random valid spawn position for the player
    Vector2D playerSpawnPos = findRandomSpawnPosition();
    
    // Setup player entity with random position
    player->addComponent<TransformComponent>(playerSpawnPos.x, playerSpawnPos.y, 32, 32, 3);
    player->addComponent<SpriteComponent>("player", true);
    player->addComponent<ColliderComponent>("player");
    player->addComponent<HealthComponent>(100);
    player->addComponent<AmmoComponent>(30, 10);
    player->addComponent<KeyboardController>();
    player->addGroup(Game::groupPlayers);

    // Create multiple enemies at random positions
    const int numEnemies = 3; // Number of enemies to spawn
    for (int i = 0; i < numEnemies; i++) {
        Entity& enemy = manager.addEntity();
        
        Vector2D enemyPos = findRandomEnemyPosition();
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
    
    questionLabel->addComponent<UILabel>(0, 300, "", "font1", white);
    answer1Label->addComponent<UILabel>(0, 340, "", "font1", white);
    answer2Label->addComponent<UILabel>(0, 380, "", "font1", white);
    answer3Label->addComponent<UILabel>(0, 420, "", "font1", white);
    answer4Label->addComponent<UILabel>(0, 460, "", "font1", white);
    feedbackLabel->addComponent<UILabel>(0, 650, "", "font2", white);
    
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

    assets->AddTexture("terrain", "./assets/TerrainTexturesLevel1.png");
    assets->AddTexture("player", "./assets/playeranimations.png");
    assets->AddTexture("enemy", "./assets/enemyanimations.png");
    assets->AddTexture("clue", "./assets/clue.png");
    assets->AddTexture("magazine", "./assets/magazine.png");
    assets->AddTexture("bulletHorizontal", "./assets/bulletHorizontal.png");
    assets->AddTexture("bulletVertical", "./assets/bulletVertical.png");
    assets->AddTexture("healthpotion", "./assets/healthpotion.png");
    assets->AddFont("font1", "./assets/MINECRAFT.TTF", 32);
    assets->AddFont("font2", "./assets/MINECRAFT.TTF", 72);

    map = new Map("terrain", 2, 32, manager);
    map->LoadMap("./assets/Level1Map.map", 60, 34);

    // Initialize all game entities and objects
    initEntities();
}

void Game::handleEvents()
{
    SDL_PollEvent(&event);
    switch(event.type) {
        case SDL_QUIT:
            isRunning = false;
            break;
        case SDL_KEYDOWN:
            if (gameOver) {
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
    
    // Always refresh entity manager for proper lifecycle management
    manager.refresh();
    
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
    if (showFeedback && (currentTime - feedbackStartTime > 1500)) {
        closeQuestion();
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
    if (player != nullptr && player->isActive()) {
        Vector2D playerPos = player->getComponent<TransformComponent>().position;
        
        manager.update();
        
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
                player->getComponent<HealthComponent>().takeDamage(5);
                damageTimer = damageCooldown;
            }

            // Destroy dead enemies
            if(e->getComponent<HealthComponent>().health <= 0) {
                e->destroy();
            }
        }

        // Center camera on player
        camera.x = player->getComponent<TransformComponent>().position.x - (camera.w / 2);
        camera.y = player->getComponent<TransformComponent>().position.y - (camera.h / 2);

        // Camera bounds
        int worldWidth = 60 * 32 * 2;
        int worldHeight = 34 * 32 * 2;
        if(camera.x < 0) camera.x = 0;
        if(camera.y < 0) camera.y = 0;
        if(camera.x > worldWidth - camera.w) camera.x = worldWidth - camera.w;
        if(camera.y > worldHeight - camera.h) camera.y = worldHeight - camera.h;
        
        // Check win condition
        if (collectedClues >= totalClues) {
            // Show game over message
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
        
        // Check player death
        if(player->getComponent<HealthComponent>().health <= 0) {
            // Show game over message
            gameover->getComponent<UILabel>().SetLabelText("GAME OVER! Press R to restart or ESC to exit", "font2");
            
            // Center the text horizontally and vertically on 1920x1080 resolution
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
            
            // Set game over state before destroying player 
            gameOver = true;
            playerWon = false;
            player->destroy();
        }
    }
}

void Game::render()
{
    SDL_RenderClear(renderer);
    
    // Always render game elements
    for(auto& t : *tiles) t->draw();
    for(auto& p : *players) p->draw();
    for(auto& e : *enemies) e->draw();
    for(auto& o : *objects) o->draw();
    for(auto& p : *projectiles) p->draw();
    
    // Always render UI elements
    healthbar->draw();
    ammobar->draw();
    clueCounter->draw();
    gameover->draw();
    
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
        
        // Draw feedback if active with its own background
        if (showFeedback && feedbackLabel != nullptr) {
            // Create special background for feedback
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 230); // Darker background for feedback
            SDL_Rect feedbackBg = {1920/4, 630, 1920/2, 100};
            SDL_RenderFillRect(renderer, &feedbackBg);
            
            // Draw the feedback text
            feedbackLabel->draw();
        }
        
        // Reset draw color
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }

    SDL_RenderPresent(renderer);
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
    
    // Clear tracking of used positions
    usedCluePositions.clear();
    usedMagazinePositions.clear();
    usedHealthPotionPositions.clear();
    usedEnemyPositions.clear();
    
    // Clear all entities except the map entities
    manager.clearAllExcept(Game::groupMap);
    
    // Reinitialize all game entities
    initEntities();
}

void Game::showQuestion(Entity* clueEntity) {
    if (questionActive) return;
    
    questionActive = true;
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
    
    // Use proper seeded random for question selection
    static std::mt19937 rng(std::time(nullptr));
    currentQuestion = rng() % questions.size();
    
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
    if (!questionActive) return;
    
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

Vector2D Game::findRandomSpawnPosition() {
    static std::mt19937 rng(std::time(nullptr));
    
    const std::vector<Vector2D> spawnPoints = {
        {8*64, 6*64},
        {12*64, 18*64},
        {18*64, 5*64},
        {26*64, 17*64},
        {26*64, 28*64},
        {28*64, 3*64},
        {34*64, 11*64},
        {39*64, 4*64},
        {40*64, 16*64},
        {53*64, 11*64},
        {50*64, 20*64}
    };
    
    int idx = rng() % spawnPoints.size();
    return spawnPoints[idx];
}

Vector2D Game::findRandomCluePosition() {
    static std::mt19937 rng(std::time(nullptr));
    
    const std::vector<Vector2D> cluePoints = {
        {10*64, 3*64},
        {8*64, 13*64},
        {18*64, 7*64},
        {17*64, 24*64},
        {22*64, 17*64},
        {32*64, 35*64},
        {47*64, 2*64},
        {30*64, 6*64}
    };
    
    // Create a list of available positions (not used yet)
    std::vector<Vector2D> availablePositions;
    for (const auto& pos : cluePoints) {
        if (usedCluePositions.find(pos) == usedCluePositions.end()) {
            availablePositions.push_back(pos);
        }
    }
    
    // If all positions are used, return a fallback position
    if (availablePositions.empty()) {
        return {10*64, 3*64}; // Default fallback
    }
    
    // Select a random available position
    int idx = rng() % availablePositions.size();
    Vector2D selectedPos = availablePositions[idx];
    
    // Mark position as used
    usedCluePositions.insert(selectedPos);
    
    return selectedPos;
}

Vector2D Game::findRandomMagazinePosition() {
    static std::mt19937 rng(std::time(nullptr));
    
    const std::vector<Vector2D> magazinePoints = {
        {8*64, 3*64},
        {19*64, 2*64},
        {34*64, 5*64},
        {43*64, 10*64},
        {14*64, 12*64},
        {27*64, 12*64},
        {35*64, 17*64},
        {15*64, 21*64},
        {30*64, 28*64}
    };
    
    // Create a list of available positions (not used yet)
    std::vector<Vector2D> availablePositions;
    for (const auto& pos : magazinePoints) {
        if (usedMagazinePositions.find(pos) == usedMagazinePositions.end()) {
            availablePositions.push_back(pos);
        }
    }
    
    // If all positions are used, return a fallback position
    if (availablePositions.empty()) {
        return {8*64, 3*64}; // Default fallback
    }
    
    // Select a random available position
    int idx = rng() % availablePositions.size();
    Vector2D selectedPos = availablePositions[idx];
    
    // Mark position as used
    usedMagazinePositions.insert(selectedPos);
    
    return selectedPos;
}

Vector2D Game::findRandomHealthPotionPosition() {
    static std::mt19937 rng(std::time(nullptr));
    
    const std::vector<Vector2D> potionPoints = {
        {9*64, 9*64},
        {16*64, 16*64},
        {23*64, 5*64},
        {28*64, 8*64},
        {39*64, 9*64},
        {28*64, 21*64},
        {21*64, 26*64},
        {52*64, 15*64}
    };
    
    // Create a list of available positions (not used yet)
    std::vector<Vector2D> availablePositions;
    for (const auto& pos : potionPoints) {
        if (usedHealthPotionPositions.find(pos) == usedHealthPotionPositions.end()) {
            availablePositions.push_back(pos);
        }
    }
    
    // If all positions are used, return a fallback position
    if (availablePositions.empty()) {
        return {9*64, 9*64}; // Default fallback
    }
    
    // Select a random available position
    int idx = rng() % availablePositions.size();
    Vector2D selectedPos = availablePositions[idx];
    
    // Mark position as used
    usedHealthPotionPositions.insert(selectedPos);
    
    return selectedPos;
}

Vector2D Game::findRandomEnemyPosition() {
    static std::mt19937 rng(std::time(nullptr));
    
    const std::vector<Vector2D> enemyPoints = {
        {12*64, 4*64},
        {10*64, 13*64},
        {17*64, 22*64},
        {22*64, 19*64},
        {34*64, 23*64},
        {45*64, 3*64},
        {49*64, 13*64}
    };
    
    // Create a list of available positions (not used yet)
    std::vector<Vector2D> availablePositions;
    for (const auto& pos : enemyPoints) {
        if (usedEnemyPositions.find(pos) == usedEnemyPositions.end()) {
            availablePositions.push_back(pos);
        }
    }
    
    // If all positions are used, return a fallback position
    if (availablePositions.empty()) {
        return {12*64, 4*64}; // Default fallback
    }
    
    // Select a random available position
    int idx = rng() % availablePositions.size();
    Vector2D selectedPos = availablePositions[idx];
    
    // Mark position as used
    usedEnemyPositions.insert(selectedPos);
    
    return selectedPos;
}

