#include "Game.hpp"
#include "TextureManager.hpp"
#include "Map.hpp"
#include "ECS/Components.hpp"
#include "ECS/EnemyAIComponent.hpp"
#include "ECS/CollisionSystem.hpp"
#include "Vector2D.hpp"
#include "Collision.hpp"
#include "AssetManager.hpp"
#include <sstream>

SDL_Renderer *Game::renderer = nullptr;
SDL_Event Game::event;
SDL_Rect Game::camera = {0, 0, 800, 640};
bool Game::isRunning = false;
AssetManager* Game::assets = nullptr;

Game::Game() : 
    player(manager.addEntity()),
    enemy(manager.addEntity()),
    healthbar(manager.addEntity()),
    gameover(manager.addEntity())
{
    assets = new AssetManager(&manager);
}

Game::~Game() {
    delete assets;
}

void Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen) {
    
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

    assets->AddTexture("terrain", "./assets/terain_textures.png");
    assets->AddTexture("player", "./assets/playeranimations.png");
    assets->AddTexture("enemy", "./assets/enemyanimations.png");
    assets->AddTexture("clue", "./assets/clue.png");
    assets->AddTexture("bulletHorizontal", "./assets/bulletHorizontal.png");
    assets->AddTexture("bulletVertical", "./assets/bulletVertical.png");

    assets->AddFont("font1", "./assets/MINECRAFT.TTF", 24);
    assets->AddFont("font2", "./assets/MINECRAFT.TTF", 36);

    map = new Map("terrain", 2, 32, manager);

    map->LoadMap("./assets/map0.map", 25, 20);

    player.addComponent<TransformComponent>(3);
    player.addComponent<SpriteComponent>("player", true);
    player.addComponent<KeyboardController>();
    player.addComponent<ColliderComponent>("player");
    player.addComponent<HealthComponent>(100);
    player.addGroup(groupPlayers);

    enemy.addComponent<TransformComponent>(200,200, 3);
    enemy.addComponent<SpriteComponent>("enemy", true);
    enemy.addComponent<ColliderComponent>("player");
    enemy.addComponent<HealthComponent>(100);
    enemy.addComponent<EnemyAIComponent>(manager);
    enemy.addGroup(groupEnemies);

    healthbar.addComponent<UILabel>(10, 10, "Test", "font1", white);
    gameover.addComponent<UILabel>(300, 320, "", "font2", white);

    assets->CreateObject(300, 300, "clue");
    
    manager.addSystem<CollisionSystem>(manager);
}

void Game::handleEvents() {
    
    SDL_PollEvent(&event);
    switch(event.type) {
        case SDL_QUIT:
            isRunning = false;
            break;
        default:
            break;
    }
}

void Game::update() {

    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    
    int health = player.getComponent<HealthComponent>().health;

    std::stringstream ss;
    ss << "Player Health: " << health;
    healthbar.getComponent<UILabel>().SetLabelText(ss.str(), "font1");

    manager.refresh();
    manager.update();

    camera.x = player.getComponent<TransformComponent>().position.x - 400;
    camera.y = player.getComponent<TransformComponent>().position.y - 320;

    // Camera bounds checking
    if(camera.x < 0) camera.x = 0;
    if(camera.y < 0) camera.y = 0;
    if(camera.x > camera.w) camera.x = camera.w;
    if(camera.y > camera.h) camera.y = camera.h;
    
    if(player.getComponent<HealthComponent>().health <= 0) {
        gameover.getComponent<UILabel>().SetLabelText("Game Over", "font2");
        healthbar.getComponent<UILabel>().SetLabelText("", "font1");
        player.destroy();
        //isRunning = false;
    }
}

void Game::render() {
    SDL_RenderClear(renderer);
    
    // Render all game objects
    for(auto& t : manager.getGroup(groupMap)) t->draw();
    for(auto& o : manager.getGroup(groupObjects)) o->draw();
    for(auto& e : manager.getGroup(groupEnemies)) e->draw();
    for(auto& p : manager.getGroup(groupPlayers)) p->draw();
    for(auto& p : manager.getGroup(groupProjectiles)) p->draw();
    
    healthbar.draw();
    gameover.draw();

    SDL_RenderPresent(renderer);
}

void Game::clean() {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    std::cout << "Game cleaned" << std::endl;
}

