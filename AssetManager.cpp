#include "AssetManager.hpp"
#include "ECS/Components.hpp"

AssetManager::AssetManager(Manager* man) : manager(man) {}

AssetManager::~AssetManager() {}

void AssetManager::CreateProjectile(Vector2D pos, Vector2D vel, int range, int speed, std::string id, SDL_RendererFlip bulletFlip) {
    auto& projectile(manager->addEntity());
    projectile.addComponent<TransformComponent>(pos.x, pos.y, 32 , 32, 1);
    projectile.addComponent<SpriteComponent>(id, false);
    projectile.addComponent<ProjectileComponent>(range, speed, vel);
    projectile.addComponent<ColliderComponent>("projectile");
    projectile.addGroup(Game::groupProjectiles);
    projectile.getComponent<SpriteComponent>().SetFlip(bulletFlip);
}

void AssetManager::CreateObject(int posX, int posY, std::string id) {
    auto& obj(manager->addEntity());
    obj.addComponent<TransformComponent>(posX, posY, 32, 32, 1);
    obj.addComponent<SpriteComponent>(id, true);
    obj.addComponent<ColliderComponent>(id);
    obj.addGroup(Game::groupObjects);
}

void AssetManager::AddTexture(std::string id, const char* path) {
    textures.emplace(id, TextureManager::LoadTexture(path));
}

SDL_Texture* AssetManager::GetTexture(std::string id) {
    return textures[id];
}

void AssetManager::AddFont(std::string id, std::string path, int fontSize) {
    fonts.emplace(id, TTF_OpenFont(path.c_str(), fontSize));
}
TTF_Font* AssetManager::GetFont(std::string id) {
    return fonts[id];
}