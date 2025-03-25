#include "Game.hpp"
#include "TextureManager.hpp"
#include "Map.hpp"
#include "ECS/Components.hpp"
#include "ECS/EnemyAIComponent.hpp"
#include "Vector2D.hpp"
#include "Collision.hpp"
#include "AssetManager.hpp"
#include <sstream>

// Global manager and entities
Map* map;
Manager manager;

// Global entity references
auto& player(manager.addEntity());
auto& enemy(manager.addEntity());
auto& healthbar(manager.addEntity());
auto& ammobar(manager.addEntity());
auto& gameover(manager.addEntity());

// Global entity groups
auto& tiles(manager.getGroup(Game::groupMap));
auto& players(manager.getGroup(Game::groupPlayers));
auto& enemies(manager.getGroup(Game::groupEnemies));
auto& colliders(manager.getGroup(Game::groupColliders));
auto& projectiles(manager.getGroup(Game::groupProjectiles));
auto& objects(manager.getGroup(Game::groupObjects));

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

Game::Game()
{
    assets = new AssetManager(&manager);
}

Game::~Game()
{
    delete assets;
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

    assets->AddFont("font1", "./assets/MINECRAFT.TTF", 32);
    assets->AddFont("font2", "./assets/MINECRAFT.TTF", 72);

    map = new Map("terrain", 2, 32, manager);
    map->LoadMap("./assets/Level1Map.map", 60, 34);

    // Setup player entity
    player.addComponent<TransformComponent>(400, 320, 32, 32, 3);
    player.addComponent<SpriteComponent>("player", true);
    player.addComponent<ColliderComponent>("player");
    player.addComponent<HealthComponent>(100);
    player.addComponent<AmmoComponent>(30, 10);
    player.addComponent<KeyboardController>();
    player.addGroup(Game::groupPlayers);

    // Setup enemy entity
    enemy.addComponent<TransformComponent>(800, 800, 32, 32, 3);
    enemy.addComponent<SpriteComponent>("enemy", true);
    enemy.addComponent<ColliderComponent>("player");
    enemy.addComponent<HealthComponent>(100);
    enemy.addComponent<EnemyAIComponent>(manager);
    enemy.addGroup(Game::groupEnemies);

    // Setup UI entities with improved positioning for larger fonts
    healthbar.addComponent<UILabel>(20, 20, "Test", "font1", white);
    ammobar.addComponent<UILabel>(20, 60, "Test", "font1", white);
    gameover.addComponent<UILabel>(0, 0, "", "font2", white);  // Position will be set dynamically

    // Create an object
    assets->CreateObject(600, 800, "clue");
    
    // Create magazine pickups
    assets->CreateMagazine(500, 400);
    assets->CreateMagazine(700, 600);
}

void Game::handleEvents()
{
    SDL_PollEvent(&event);
    switch(event.type) {
        case SDL_QUIT:
            isRunning = false;
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
    
    // Update health display
    int health = player.getComponent<HealthComponent>().health;
    std::stringstream healthSS;
    healthSS << "Health: " << health;
    healthbar.getComponent<UILabel>().SetLabelText(healthSS.str(), "font1");
    
    // Update ammo display
    int ammo = player.getComponent<AmmoComponent>().currentAmmo;
    std::stringstream ammoSS;
    ammoSS << "Ammo: " << ammo;
    ammobar.getComponent<UILabel>().SetLabelText(ammoSS.str(), "font1");

    // Save player position before movement
    Vector2D playerPos = player.getComponent<TransformComponent>().position;
    
    // Process other entities and input
    manager.refresh();
    manager.update();
    
    // Get updated player collider after movement
    SDL_Rect playerCol = player.getComponent<ColliderComponent>().collider;
    
    damageTimer -= 1.0f/60.0f;
    
    if (!objectCollisionsEnabled) {
        objectCollisionDelay -= 1.0f/60.0f;
        if (objectCollisionDelay <= 0.0f) {
            objectCollisionsEnabled = true;
        }
    }

    // Player collision with terrain
    for(auto& c : colliders) {
        SDL_Rect cCol = c->getComponent<ColliderComponent>().collider;
        
        // Check if collision happened
        if(Collision::AABB(cCol, playerCol)) {
            // Simply restore player to previous position before movement
            player.getComponent<TransformComponent>().position = playerPos;
            // Update collider position
            player.getComponent<ColliderComponent>().update();
        }
    }

    // Player collision with objects (clues and magazines)
    if (objectCollisionsEnabled) {
        for(auto& o : objects) {
            if(Collision::AABB(player.getComponent<ColliderComponent>().collider, 
                            o->getComponent<ColliderComponent>().collider)) {
                if (o->getComponent<ColliderComponent>().tag == "magazine") {
                    std::cout << "Magazine picked up" << std::endl;
                    player.getComponent<AmmoComponent>().addAmmo();
                    o->destroy();
                }
                else if (o->getComponent<ColliderComponent>().tag == "object") {
                    std::cout << "Clue picked up" << std::endl;
                    player.getComponent<HealthComponent>().heal(20);
                    o->destroy();
                }
            }
        }
    }

    // Enemy collision with projectiles and player
    for(auto& e : enemies) {
        // Projectile collision
        for(auto& p : projectiles) {
            if(Collision::AABB(e->getComponent<ColliderComponent>().collider, 
                            p->getComponent<ColliderComponent>().collider)) {
                std::cout << "Hit enemy" << std::endl;
                e->getComponent<HealthComponent>().takeDamage(25);
                p->destroy();
            }
        }

        // Player collision with damage cooldown
        SDL_Rect updatedPlayerCol = player.getComponent<ColliderComponent>().collider;
        if(Collision::AABB(updatedPlayerCol, e->getComponent<ColliderComponent>().collider) && damageTimer <= 0) {
            player.getComponent<HealthComponent>().takeDamage(5);
            damageTimer = damageCooldown;
        }

        // Destroy dead enemies
        if(e->getComponent<HealthComponent>().health <= 0) {
            e->destroy();
        }
    }

    // Center camera on player
    camera.x = player.getComponent<TransformComponent>().position.x - (camera.w / 2);
    camera.y = player.getComponent<TransformComponent>().position.y - (camera.h / 2);

    // Camera bounds
    int worldWidth = 60 * 32 * 2;
    int worldHeight = 34 * 32 * 2;
    if(camera.x < 0) camera.x = 0;
    if(camera.y < 0) camera.y = 0;
    if(camera.x > worldWidth - camera.w) camera.x = worldWidth - camera.w;
    if(camera.y > worldHeight - camera.h) camera.y = worldHeight - camera.h;
    
    // Check player death
    if(player.getComponent<HealthComponent>().health <= 0) {
        gameover.getComponent<UILabel>().SetLabelText("GAME OVER", "font2");
        
        // Center the text horizontally and vertically on 1920x1080 resolution
        int textWidth = gameover.getComponent<UILabel>().GetWidth();
        int textHeight = gameover.getComponent<UILabel>().GetHeight();
        
        int xPos = (1920 - textWidth) / 2;
        int yPos = (1080 - textHeight) / 2;
        
        gameover.getComponent<UILabel>().SetPosition(xPos, yPos);
        
        healthbar.getComponent<UILabel>().SetLabelText("", "font1");
        ammobar.getComponent<UILabel>().SetLabelText("", "font1");
        player.destroy();
        //isRunning = false;
    }
}

void Game::render()
{
    SDL_RenderClear(renderer);
    
    // Render all game objects by group
    for(auto& t : tiles) t->draw();
    for(auto& c : colliders) c->draw();
    for(auto& o : objects) o->draw();
    for(auto& e : enemies) e->draw();
    for(auto& p : players) p->draw();
    for(auto& p : projectiles) p->draw();
    
    // Render UI elements
    healthbar.draw();
    ammobar.draw();
    gameover.draw();

    SDL_RenderPresent(renderer);
}

void Game::clean()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    std::cout << "Game cleaned" << std::endl;
}

