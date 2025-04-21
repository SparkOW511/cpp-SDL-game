#include "RandomPositionManager.hpp"
#include "Game.hpp"

RandomPositionManager::RandomPositionManager() {
    rng.seed(static_cast<unsigned int>(std::time(nullptr)));
}

RandomPositionManager::~RandomPositionManager() {
}

void RandomPositionManager::resetPositions() {
    usedCluePositions.clear();
    usedMagazinePositions.clear();
    usedHealthPotionPositions.clear();
    usedEnemyPositions.clear();
}

Vector2D RandomPositionManager::findRandomSpawnPosition(int currentLevel) {
    if (currentLevel == 1) {
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
        return {30*64, 30*64};
    }
    else if (currentLevel == 3) {
        return {30*64, 31*64};
    }
    else if (currentLevel == 4) {
        return {34*64, 32*64};
    }
    
    return {15*64, 15*64};
}

Vector2D RandomPositionManager::findRandomCluePosition(int currentLevel) {
    std::vector<Vector2D> cluePoints;
    
    if (currentLevel == 1) {
        cluePoints = {
            {10*64, 3*64},
            {8*64, 13*64},
            {18*64, 7*64},
            {17*64, 24*64},
            {22*64, 17*64},
            {32*64, 25*64},
            {46*64, 2*64},
            {30*64, 6*64}
        };
    } 
    else if (currentLevel == 2) {
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
    else if (currentLevel == 3) {
        cluePoints = {
            {9*64, 2*64},
            {22*64, 2*64},
            {39*64, 2*64},
            {3*64, 4*64},
            {30*64, 4*64},
            {58*64, 4*64},
            {15*64, 8*64},
            {26*64, 9*64},
            {8*64, 15*64},
            {41*64, 18*64},
            {52*64, 17*64},
            {32*64, 21*64},
            {41*64, 24*64},
            {8*64, 25*64},
            {15*64, 31*64},
            {52*64, 30*64}
        };
    }
    
    std::vector<Vector2D> availablePositions;
    for (const auto& pos : cluePoints) {
        if (usedCluePositions.find(pos) == usedCluePositions.end()) {
            availablePositions.push_back(pos);
        }
    }
    
    if (availablePositions.empty()) {
        if (currentLevel == 1) return Vector2D{10*64, 3*64};
        else if (currentLevel == 2) return Vector2D{24*64, 3*64};
        else if (currentLevel == 3) return Vector2D{10*64, 5*64};
        else return Vector2D{10*64, 3*64};
    }
    
    int idx = rng() % availablePositions.size();
    Vector2D selectedPos = availablePositions[idx];
    
    usedCluePositions.insert(selectedPos);
    
    return selectedPos;
}

Vector2D RandomPositionManager::findRandomMagazinePosition() {
    std::vector<Vector2D> magazinePoints;
    
    int currentLevel = Game::currentLevel;
    
    if (currentLevel == 1) {
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
    else if (currentLevel == 3) {
        magazinePoints = {
            {33*64, 2*64},
            {7*64, 4*64},
            {17*64, 4*64},
            {26*64, 6*64},
            {35*64, 6*64},
            {2*64, 7*64},
            {6*64, 10*64},
            {19*64, 12*64},
            {31*64, 12*64},
            {39*64, 12*64},
            {48*64, 15*64},
            {12*64, 16*64},
            {38*64, 16*64},
            {2*64, 17*64},
            {7*64, 17*64},
            {16*64, 20*64},
            {59*64, 19*64},
            {14*64, 24*64},
            {37*64, 25*64},
            {2*64, 28*64},
            {9*64, 28*64},
            {17*64, 28*64},
            {5*64, 31*64},
            {58*64, 31*64}
        };
    }
    else if (currentLevel == 4) {
        magazinePoints = {
            {34*64, 9*64},
            {43*64, 13*64},
            {24*64, 15*64},
            {23*64, 21*64},
            {39*64, 20*64},
            {24*64, 26*64},
            {42*64, 21*64}
        };
    }
    
    std::vector<Vector2D> availablePositions;
    for (const auto& pos : magazinePoints) {
        if (usedMagazinePositions.find(pos) == usedMagazinePositions.end()) {
            availablePositions.push_back(pos);
        }
    }
    
    if (availablePositions.empty()) {
        if (currentLevel == 1) return Vector2D{8*64, 3*64};
        else if (currentLevel == 2) return Vector2D{0*64, 3*64};
        else if (currentLevel == 3) return Vector2D{8*64, 3*64};
        else if (currentLevel == 4) return Vector2D{34*64, 9*64};
        else return Vector2D{8*64, 3*64};
    }
    
    int idx = rng() % availablePositions.size();
    Vector2D selectedPos = availablePositions[idx];
    
    usedMagazinePositions.insert(selectedPos);
    
    return selectedPos;
}

Vector2D RandomPositionManager::findRandomHealthPotionPosition() {
    std::vector<Vector2D> potionPoints;
    
    int currentLevel = Game::currentLevel;
    
    if (currentLevel == 1) {
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
        potionPoints = {
            {2*64, 3*64},
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
    else if (currentLevel == 3) {
        potionPoints = {
            {17*64, 1*64},
            {59*64, 2*64},
            {10*64, 5*64},
            {42*64, 5*64},
            {47*64, 5*64},
            {4*64, 6*64},
            {18*64, 7*64},
            {32*64, 7*64},
            {59*64, 7*64},
            {16*64, 13*64},
            {36*64, 12*64},
            {49*64, 13*64},
            {47*64, 17*64},
            {38*64, 19*64},
            {22*64, 22*64},
            {12*64, 22*64},
            {27*64, 26*64},
            {10*64, 28*64},
            {24*64, 28*64},
            {31*64, 28*64},
            {43*64, 28*64},
            {59*64, 22*64},
            {32*64, 31*64},
            {44*64, 31*64},
            {9*64, 31*64}
        };
    }
    else if (currentLevel == 4) {
        potionPoints = {
            {24*64, 11*64},
            {43*64, 10*64},
            {29*64, 16*64},
            {42*64, 16*64},
            {24*64, 21*64},
            {34*64, 21*64},
            {39*64, 25*64},
            {43*64, 25*64},
            {29*64, 23*64}
        };
    }
    
    std::vector<Vector2D> availablePositions;
    for (const auto& pos : potionPoints) {
        if (usedHealthPotionPositions.find(pos) == usedHealthPotionPositions.end()) {
            availablePositions.push_back(pos);
        }
    }
    
    if (availablePositions.empty()) {
        if (currentLevel == 1) return Vector2D{9*64, 9*64};
        else if (currentLevel == 2) return Vector2D{1*64, 0*64};
        else if (currentLevel == 3) return Vector2D{5*64, 6*64};
        else if (currentLevel == 4) return Vector2D{24*64, 11*64};
        else return Vector2D{9*64, 9*64};
    }
    
    int idx = rng() % availablePositions.size();
    Vector2D selectedPos = availablePositions[idx];
    
    usedHealthPotionPositions.insert(selectedPos);
    
    return selectedPos;
}

Vector2D RandomPositionManager::findRandomEnemyPosition() {
    std::vector<Vector2D> enemyPoints;
    
    int currentLevel = Game::currentLevel;
    
    if (currentLevel == 1) {
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
    else if (currentLevel == 3) {
        enemyPoints = {
            {45*64, 3*64},
            {52*64, 4*64},
            {20*64, 5*64},
            {33*64, 5*64},
            {39*64, 6*64},
            {4*64, 8*64},
            {16*64, 9*64},
            {33*64, 9*64},
            {9*64, 10*64},
            {13*64, 13*64},
            {38*64, 13*64},
            {7*64, 14*64},
            {35*64, 19*64},
            {2*64, 22*64},
            {7*64, 22*64},
            {18*64, 22*64},
            {22*64, 22*64},
            {27*64, 22*64},
            {32*64, 22*64},
            {17*64, 25*64},
            {58*64, 27*64},
            {6*64, 30*64},
            {51*64, 30*64}
        };
    }
    else if (currentLevel == 4) {
        enemyPoints = {
            {30*64, 12*64},
            {38*64, 12*64},
            {30*64, 17*64},
            {38*64, 17*64}
        };
    }
    
    std::vector<Vector2D> availablePositions;
    for (const auto& pos : enemyPoints) {
        if (usedEnemyPositions.find(pos) == usedEnemyPositions.end()) {
            availablePositions.push_back(pos);
        }
    }
    
    if (availablePositions.empty()) {
        if (currentLevel == 1) return Vector2D{12*64, 4*64};
        else if (currentLevel == 2) return Vector2D{9*64, 3*64};
        else if (currentLevel == 3) return Vector2D{12*64, 4*64};
        else if (currentLevel == 4) return Vector2D{30*64, 12*64};
        else return Vector2D{12*64, 4*64};
    }
    
    int idx = rng() % availablePositions.size();
    Vector2D selectedPos = availablePositions[idx];
    
    usedEnemyPositions.insert(selectedPos);
    
    return selectedPos;
} 