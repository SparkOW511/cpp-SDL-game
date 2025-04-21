#pragma once

#include <vector>
#include <set>
#include <random>
#include <ctime>
#include "Vector2D.hpp"

class RandomPositionManager {
public:
    RandomPositionManager();
    ~RandomPositionManager();
    
    void resetPositions();
    
    Vector2D findRandomSpawnPosition(int currentLevel);
    Vector2D findRandomCluePosition(int currentLevel);
    Vector2D findRandomMagazinePosition();
    Vector2D findRandomHealthPotionPosition();
    Vector2D findRandomEnemyPosition();
    
private:
    std::mt19937 rng;
    
    std::set<Vector2D> usedCluePositions;
    std::set<Vector2D> usedMagazinePositions;
    std::set<Vector2D> usedHealthPotionPositions;
    std::set<Vector2D> usedEnemyPositions;
}; 