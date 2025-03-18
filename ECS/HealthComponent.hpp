#pragma once

#include "ECS.hpp"
#include "Components.hpp"

class HealthComponent : public Component {
public:
    int health;
    int maxHealth;
    float lastDamageTime;
    float damageCooldown;

    HealthComponent(int maxHealth) {
        this->maxHealth = maxHealth;
        this->health = maxHealth;
        this->lastDamageTime = 0.0f;
        this->damageCooldown = 200.0f; 
    }

    void takeDamage(int amount, float deltaTime) {
        float currentTime = SDL_GetTicks();
        
        if (currentTime - lastDamageTime >= damageCooldown) {
            health -= amount;
            if (health < 0) health = 0;
            lastDamageTime = currentTime;
        }
    }

    void takeDamage(int amount) {
            health -= amount;
            if (health < 0) health = 0;
    }

    void heal(int amount) {
        health += amount;
        if (health > maxHealth) health = maxHealth;
    }
};