#include "RandomPositionManager.hpp"
#include "Game.hpp"

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
    // Different spawn points based on level
    if (currentLevel == 1) {
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
    else if (currentLevel == 2) {
        // Level 2: Spawn player at the bottom center of the map
        return {30*32, 30*32}; // Position player with enough space around them
    }
    
    // Default fallback if level is not recognized
    return {15*32, 15*32};
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
    } 
    else if (currentLevel == 2) {
        // Level 2 clue positions - strategically placed throughout the map with more on right side
        cluePoints = {
            {10*32, 8*32},    // Top left area
            {25*32, 5*32},    // Top middle area
            {45*32, 7*32},    // Top right area
            {15*32, 15*32},   // Left middle area
            {30*32, 17*32},   // Center area
            {50*32, 16*32},   // Right middle area
            {12*32, 25*32},   // Bottom left area
            {35*32, 28*32},   // Bottom middle area
            {55*32, 27*32},   // Bottom right area
            {58*32, 12*32},   // Far right middle
            {53*32, 5*32},    // Far right top
            {48*32, 29*32}    // Far right bottom
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
        return currentLevel == 1 ? Vector2D{10*64, 3*64} : Vector2D{30*32, 15*32}; // Level-specific fallback
    }
    
    // Select a random available position
    int idx = rng() % availablePositions.size();
    Vector2D selectedPos = availablePositions[idx];
    
    // Mark position as used
    usedCluePositions.insert(selectedPos);
    
    return selectedPos;
}

Vector2D RandomPositionManager::findRandomMagazinePosition() {
    std::vector<Vector2D> magazinePoints;
    
    // Check current level
    int currentLevel = Game::currentLevel;
    
    if (currentLevel == 1) {
        // Level 1 magazine positions
        magazinePoints = {
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
    } 
    else if (currentLevel == 2) {
        // Level 2 magazine pickup positions - expanded with more on right side
        magazinePoints = {
            {8*32, 10*32},    // Left side
            {20*32, 8*32},    // Top middle
            {42*32, 12*32},   // Right side
            {15*32, 20*32},   // Middle left
            {28*32, 22*32},   // Center
            {45*32, 18*32},   // Middle right
            {12*32, 29*32},   // Bottom left
            {32*32, 26*32},   // Bottom middle
            {52*32, 28*32},   // Bottom right
            {55*32, 15*32},   // Far right middle
            {58*32, 8*32},    // Far right top
            {59*32, 30*32}    // Far right bottom
        };
    }
    
    // Create a list of available positions (not used yet)
    std::vector<Vector2D> availablePositions;
    for (const auto& pos : magazinePoints) {
        if (usedMagazinePositions.find(pos) == usedMagazinePositions.end()) {
            availablePositions.push_back(pos);
        }
    }
    
    // If all positions are used, return a fallback position
    if (availablePositions.empty()) {
        return currentLevel == 1 ? Vector2D{8*64, 3*64} : Vector2D{32*32, 15*32}; // Level-specific fallback
    }
    
    // Select a random available position
    int idx = rng() % availablePositions.size();
    Vector2D selectedPos = availablePositions[idx];
    
    // Mark position as used
    usedMagazinePositions.insert(selectedPos);
    
    return selectedPos;
}

Vector2D RandomPositionManager::findRandomHealthPotionPosition() {
    std::vector<Vector2D> potionPoints;
    
    // Use different positions based on level
    int currentLevel = Game::currentLevel;
    
    if (currentLevel == 1) {
        // Level 1 potion positions
        potionPoints = {
            {9*64, 9*64},
            {16*64, 16*64},
            {23*64, 5*64},
            {28*64, 8*64},
            {39*64, 9*64},
            {28*64, 21*64},
            {21*64, 26*64},
            {52*64, 15*64}
        };
    } 
    else if (currentLevel == 2) {
        // Level 2 health potion positions - expanded with more on right side
        potionPoints = {
            {5*32, 7*32},     // Top left corner
            {22*32, 10*32},   // Top middle
            {48*32, 6*32},    // Top right
            {13*32, 15*32},   // Left middle
            {35*32, 16*32},   // Right middle
            {7*32, 25*32},    // Bottom left
            {25*32, 28*32},   // Bottom middle
            {52*32, 22*32},   // Bottom right
            {56*32, 10*32},   // Far right top
            {59*32, 20*32},   // Far right middle
            {57*32, 33*32}    // Far right bottom
        };
    }
    
    // Create a list of available positions (not used yet)
    std::vector<Vector2D> availablePositions;
    for (const auto& pos : potionPoints) {
        if (usedHealthPotionPositions.find(pos) == usedHealthPotionPositions.end()) {
            availablePositions.push_back(pos);
        }
    }
    
    // If all positions are used, return a fallback position
    if (availablePositions.empty()) {
        return currentLevel == 1 ? Vector2D{9*64, 9*64} : Vector2D{25*32, 20*32}; // Level-specific fallback
    }
    
    // Select a random available position
    int idx = rng() % availablePositions.size();
    Vector2D selectedPos = availablePositions[idx];
    
    // Mark position as used
    usedHealthPotionPositions.insert(selectedPos);
    
    return selectedPos;
}

Vector2D RandomPositionManager::findRandomEnemyPosition() {
    std::vector<Vector2D> enemyPoints;
    
    // Different enemy positions based on level
    int currentLevel = Game::currentLevel;
    
    if (currentLevel == 1) {
        // Level 1 enemy positions
        enemyPoints = {
            {12*64, 4*64},
            {10*64, 13*64},
            {17*64, 22*64},
            {22*64, 19*64},
            {34*64, 23*64},
            {45*64, 3*64},
            {49*64, 13*64}
        };
    } 
    else if (currentLevel == 2) {
        // Level 2 enemy positions - expanded with more on right side
        enemyPoints = {
            {12*32, 6*32},    // Top left patrol area
            {38*32, 7*32},    // Top right patrol area
            {8*32, 20*32},    // Middle left patrol area
            {25*32, 15*32},   // Middle area patrol
            {42*32, 16*32},   // Middle right patrol area
            {15*32, 28*32},   // Bottom left patrol area 
            {45*32, 25*32},   // Bottom right patrol area
            {56*32, 8*32},    // Far right top patrol area
            {58*32, 18*32},   // Far right middle patrol area
            {55*32, 30*32}    // Far right bottom patrol area
        };
    }
    
    // Create a list of available positions (not used yet)
    std::vector<Vector2D> availablePositions;
    for (const auto& pos : enemyPoints) {
        if (usedEnemyPositions.find(pos) == usedEnemyPositions.end()) {
            availablePositions.push_back(pos);
        }
    }
    
    // If all positions are used, return a fallback position
    if (availablePositions.empty()) {
        return currentLevel == 1 ? Vector2D{12*64, 4*64} : Vector2D{30*32, 10*32}; // Level-specific fallback
    }
    
    // Select a random available position
    int idx = rng() % availablePositions.size();
    Vector2D selectedPos = availablePositions[idx];
    
    // Mark position as used
    usedEnemyPositions.insert(selectedPos);
    
    return selectedPos;
} 