#include "RandomPositionManager.hpp"

RandomPositionManager::RandomPositionManager() {
    // Initialize random number generator with time-based seed
    rng.seed(static_cast<unsigned int>(std::time(nullptr)));
}

RandomPositionManager::~RandomPositionManager() {
    // No dynamic memory to clean up
}

void RandomPositionManager::resetPositions() {
    usedCluePositions.clear();
    usedMagazinePositions.clear();
    usedHealthPotionPositions.clear();
    usedEnemyPositions.clear();
}

Vector2D RandomPositionManager::findRandomSpawnPosition(int currentLevel) {
    // On level 2, always spawn at bottom center of map
    if (currentLevel == 2) {
        return {30*64, 30*64}; // Bottom center of map for level 2
    }
    
    // Random spawns for level 1
    const std::vector<Vector2D> spawnPoints = {
        {8*64, 6*64},
        {12*64, 18*64},
        {18*64, 5*64},
        {26*64, 17*64},
        {26*64, 28*64},
        {28*64, 3*64},
        {34*64, 11*64},
        {39*64, 4*64},
        {36*64, 19*64},
        {53*64, 11*64},
        {50*64, 20*64}
    };
    
    int idx = rng() % spawnPoints.size();
    return spawnPoints[idx];
}

Vector2D RandomPositionManager::findRandomCluePosition(int currentLevel) {
    std::vector<Vector2D> cluePoints;
    
    // Use different spawn positions based on the current level
    if (currentLevel == 1) {
        // Level 1 clue positions
        cluePoints = {
            {10*64, 3*64},
            {8*64, 13*64},
            {18*64, 7*64},
            {17*64, 24*64},
            {22*64, 17*64},
            {32*64, 25*64}, // Fixed position
            {46*64, 2*64},
            {30*64, 6*64}
        };
    } else {
        // Level 2 clue positions
        cluePoints = {
            {10*64, 3*64},
            {8*64, 13*64},
            {18*64, 7*64},
            {17*64, 24*64},
            {22*64, 17*64},
            {32*64, 35*64},
            {46*64, 2*64},
            {30*64, 6*64}
        };
    }
    
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

Vector2D RandomPositionManager::findRandomMagazinePosition() {
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

Vector2D RandomPositionManager::findRandomHealthPotionPosition() {
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

Vector2D RandomPositionManager::findRandomEnemyPosition() {
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