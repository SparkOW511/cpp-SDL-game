#include "Game.hpp"
#include "TextureManager.hpp"
#include "Map.hpp"
#include "ECS/Components.hpp"
#include "Vector2D.hpp"
#include "Collision.hpp"
#include "AssetManager.hpp"
#include <sstream>

Map *map;
Manager manager;

SDL_Renderer *Game::renderer = nullptr;
SDL_Event Game::event;

SDL_Rect Game::camera = {0, 0, 800, 640};

AssetManager* Game::assets= new AssetManager(&manager);

bool Game::isRunning = false;

auto& player(manager.addEntity());
auto& enemy(manager.addEntity());
auto& healthbar(manager.addEntity());
auto& gameover(manager.addEntity());

SDL_Color white = {255, 255, 255, 255};

int velX;
int velY;
int offsetY;
int offsetX;

SDL_RendererFlip bulletFlip = SDL_FLIP_NONE;
SDL_RendererFlip playerFlip = SDL_FLIP_NONE;

const char* direction;
const char* playerDirection;

Uint32 lastShotTime = 0;
const Uint32 shotCooldown = 350;

Game::Game() {
    
}
Game::~Game() {

}

void Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen) {

        int flags = 0;
        if(fullscreen){
            flags = SDL_WINDOW_FULLSCREEN;
        }

        if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
        
        window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
        renderer = SDL_CreateRenderer(window, -1, 0);

        if(renderer){
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

    map = new Map("terrain", 2, 32);

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
    enemy.addGroup(groupEnemies);

    healthbar.addComponent<UILabel>(10, 10, "Test", "font1", white);
    gameover.addComponent<UILabel>(300, 320, "", "font2", white);

    assets->CreateObject(300, 300, "clue");
}

auto& tiles(manager.getGroup(Game::groupMap));
auto& players(manager.getGroup(Game::groupPlayers));
auto& enemies(manager.getGroup(Game::groupEnemies));
auto& colliders(manager.getGroup(Game::groupColliders));
auto& projectiles(manager.getGroup(Game::groupProjectiles));
auto& objects(manager.getGroup(Game::groupObjects));

void Game::handleEvents() {
    
    SDL_PollEvent(&event);
    switch(event.type){
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

    SDL_Rect playerCol = player.getComponent<ColliderComponent>().collider;
    Vector2D playerPos = player.getComponent<TransformComponent>().position;
    int health = player.getComponent<HealthComponent>().health;

    std::stringstream ss;
    ss << "Player Health: " << health;
    healthbar.getComponent<UILabel>().SetLabelText(ss.str(), "font1");

    manager.refresh();
    manager.update();

    for(auto& c : colliders) {
        SDL_Rect cCol = c->getComponent<ColliderComponent>().collider;
        if(Collision::AABB(cCol, playerCol)) {
            player.getComponent<TransformComponent>().position = playerPos;
        }
    }

    for(auto& o : objects) {
        o->getComponent<SpriteComponent>().Play("Clue");
        if(Collision::AABB(player.getComponent<ColliderComponent>().collider, o->getComponent<ColliderComponent>().collider)) {
            std::cout << "Clue picked up" << std::endl;
            player.getComponent<HealthComponent>().heal(20);
            o->destroy();
        }
    }

    for(auto& e : enemies) {
        Vector2D enemyPos = e->getComponent<TransformComponent>().position;
        Vector2D direction = playerPos - enemyPos;
        float distance = direction.magnitude();

        e->getComponent<SpriteComponent>().Play("Idle");
        
        if(distance < 200) {
            direction = direction.normalize();
            e->getComponent<TransformComponent>().position += direction * 1.5f;
            if (std::abs(direction.x) > std::abs(direction.y)) {
                e->getComponent<SpriteComponent>().Play("Walk");
                if (direction.x > 0) {
                    e->getComponent<SpriteComponent>().SetFlip(SDL_FLIP_NONE);
                } 
                else {
                    e->getComponent<SpriteComponent>().SetFlip(SDL_FLIP_HORIZONTAL);
                }
            } 
            else {
                if (direction.y > 0) {
                    e->getComponent<SpriteComponent>().Play("WalkDown");
                } 
                else {
                    e->getComponent<SpriteComponent>().Play("WalkUp");
                }
            }
        }

        for(auto& p : projectiles) {
            if(Collision::AABB(e->getComponent<ColliderComponent>().collider, p->getComponent<ColliderComponent>().collider)) {
                std::cout << "Hit enemy" << std::endl;
                e->getComponent<HealthComponent>().takeDamage(25);
                p->destroy();
            }
        }

        if(Collision::AABB(playerCol, e->getComponent<ColliderComponent>().collider)) {
            player.getComponent<HealthComponent>().takeDamage(5, deltaTime);
        }

        if(e->getComponent<HealthComponent>().health <= 0) {
            e->destroy();
        }

    }

    camera.x = player.getComponent<TransformComponent>().position.x - 400;
    camera.y = player.getComponent<TransformComponent>().position.y - 320;

    if(camera.x < 0) {
        camera.x = 0;
    }
    if(camera.y < 0) {
        camera.y = 0;
    }
    if(camera.x > camera.w) {
        camera.x = camera.w;
    }
    if(camera.y > camera.h) {
        camera.y = camera.h;
    }
    
    if(player.getComponent<HealthComponent>().health <= 0) {
        gameover.getComponent<UILabel>().SetLabelText("Game Over", "font2");
        healthbar.getComponent<UILabel>().SetLabelText("", "font1");
        player.destroy();
        //isRunning = false;
    }

    float playerPosX = player.getComponent<TransformComponent>().position.x;
    float playerPosY = player.getComponent<TransformComponent>().position.y;

    char playerInput = player.getComponent<KeyboardController>().playerInput;

    if(playerInput == 'w') {
        velX = 0;
        velY = -3;
        offsetY = 30;
        offsetX = 60;
        bulletFlip = SDL_FLIP_NONE;
        playerFlip = SDL_FLIP_NONE;
        playerDirection = "ShootUp";
        direction = "bulletVertical";
    }
    else if(playerInput == 'a') {
        velX = -3;
        velY = 0;
        offsetY = 25;
        offsetX = -35;
        bulletFlip = SDL_FLIP_HORIZONTAL;
        playerFlip = SDL_FLIP_HORIZONTAL;
        playerDirection = "Shoot";
        direction = "bulletHorizontal";
    }
    else if(playerInput == 's') {
        velX = 0;
        velY = 3;
        offsetY = 30;
        offsetX = 60;
        bulletFlip = SDL_FLIP_HORIZONTAL;
        playerFlip = SDL_FLIP_NONE;
        playerDirection = "ShootDown";
        direction = "bulletVertical";
    }
    else if(playerInput == 'd') {
        velX = 3;
        velY = 0;
        offsetY = 25;
        offsetX = 65;
        bulletFlip = SDL_FLIP_NONE;
        playerFlip = SDL_FLIP_NONE;
        playerDirection = "Shoot";
        direction = "bulletHorizontal";
    }
    else {
        velX = 0;
        velY = 3;
        offsetY = 30;
        offsetX = 60;
        bulletFlip = SDL_FLIP_HORIZONTAL;
        playerFlip = SDL_FLIP_NONE;
        playerDirection = "ShootDown";
        direction = "bulletVertical";
    }
    if(Game::event.type == SDL_KEYDOWN) {
        switch (Game::event.key.keysym.sym) {
                case SDLK_SPACE:
                    if (currentTime - lastShotTime >= shotCooldown) {
                        player.getComponent<SpriteComponent>().Play(playerDirection);
                        player.getComponent<SpriteComponent>().SetFlip(playerFlip);
                        assets->CreateProjectile(Vector2D(playerPosX + offsetX,playerPosY+offsetY), Vector2D(velX,velY), 250, 1, direction, bulletFlip);
                        lastShotTime = currentTime;
                    }
                    break;
                default:
                    break;
            }
    }
    if(Game::event.type == SDL_KEYUP) {
        switch (Game::event.key.keysym.sym)
                {
                case SDLK_SPACE:
                player.getComponent<SpriteComponent>().Play("Idle");
                    break;
                default:
                    break;
                }
    }
}

void Game::render() {
    SDL_RenderClear(renderer);
    for(auto& t : tiles) {
        t->draw();
    }
 /*  for(auto& c : colliders) {
        c->draw();
    } */
    for(auto& o : objects) {
        o->draw();
    }
    for(auto& e : enemies) {
        e->draw();
    }
    for(auto& p : players) {
        p->draw();
    }
    for(auto& p : projectiles) {
        p->draw();
    }
    
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

