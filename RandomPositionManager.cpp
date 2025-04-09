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
        return {30*64, 30*64}; // Position player at the bottom center of the map
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
        // Level 2 clue positions (number 3 from map)
        cluePoints = {
            {39*64, 1*64},
            {14*64, 3*64},
            {4*64, 4*64},
            {19*64, 4*64},
            {46*64, 4*64},
            {28*64, 5*64},
            {58*64, 5*64},
            {38*64, 8*64},
            {18*64, 10*64},
            {12*64, 15*64},
            {44*64, 15*64},
            {43*64, 19*64}
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
        return currentLevel == 1 ? Vector2D{10*64, 3*64} : Vector2D{24*64, 3*64}; // Level-specific fallback
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
        // Level 2 magazine positions (number 1 from map)
        magazinePoints = {
            {15*64, 1*64},
            {7*64, 2*64},
            {57*64, 3*64},
            {16*64, 5*64},
            {24*64, 5*64},
            {33*64, 4*64},
            {44*64, 6*64},
            {11*64, 8*64},
            {28*64, 9*64},
            {58*64, 9*64},
            {60*64, 9*64},
            {32*64, 11*64},
            {21*64, 12*64},
            {40*64, 12*64},
            {49*64, 12*64},
            {36*64, 14*64},
            {30*64, 17*64},
            {47*64, 19*64},
            {11*64, 19*64},
            {24*64, 22*64},
            {31*64, 25*64},
            {23*64, 29*64},
            {39*64, 31*64},
            {49*64, 31*64}
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
        return currentLevel == 1 ? Vector2D{8*64, 3*64} : Vector2D{0*64, 3*64}; // Level-specific fallback
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
        // Level 2 health potion positions (number 4 from map)
        potionPoints = {
            {0*64, 0*64},
            {11*64, 1*64},
            {50*64, 1*64},
            {23*64, 2*64},
            {13*64, 5*64},
            {40*64, 5*64},
            {10*64, 6*64},
            {5*64, 7*64},
            {22*64, 7*64},
            {50*64, 9*64},
            {13*64, 12*64},
            {42*64, 12*64},
            {33*64, 17*64},
            {45*64, 17*64},
            {27*64, 19*64},
            {36*64, 19*64},
            {21*64, 20*64},
            {40*64, 20*64},
            {17*64, 21*64},
            {22*64, 26*64},
            {13*64, 31*64},
            {44*64, 31*64}
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
        return currentLevel == 1 ? Vector2D{9*64, 9*64} : Vector2D{1*64, 0*64}; // Level-specific fallback
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
        // Level 2 enemy positions (number 2 from map)
        enemyPoints = {
            {24*64, 0*64},
            {33*64, 1*64},
            {45*64, 2*64},
            {2*64, 4*64},
            {8*64, 4*64},
            {15*64, 8*64},
            {25*64, 9*64},
            {17*64, 13*64},
            {29*64, 14*64},
            {20*64, 16*64},
            {24*64, 18*64},
            {39*64, 18*64},
            {30*64, 20*64},
            {34*64, 22*64},
            {12*64, 26*64},
            {27*64, 26*64},
            {42*64, 26*64},
            {50*64, 26*64},
            {36*64, 28*64},
            {18*64, 30*64}
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
        return currentLevel == 1 ? Vector2D{12*64, 4*64} : Vector2D{9*64, 3*64}; // Level-specific fallback
    }
    
    // Select a random available position
    int idx = rng() % availablePositions.size();
    Vector2D selectedPos = availablePositions[idx];
    
    // Mark position as used
    usedEnemyPositions.insert(selectedPos);
    
    return selectedPos;
} 