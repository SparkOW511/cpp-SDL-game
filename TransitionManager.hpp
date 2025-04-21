#pragma once

#include <string>
#include "SDL2/SDL.h"
#include "Vector2D.hpp"
#include "ECS/ECS.hpp"

class Game;

class TransitionManager {
public:
    TransitionManager();
    ~TransitionManager();
    
    void init(Game* game, Manager* manager);
    
    void startTransition(int currentLevel, int nextLevel);
    
    bool updateTransition();
    
    void renderTransition();
    
    bool isTransitioning() const { return mIsTransitioning; }
    
    Entity* mTransitionLabel;
    
private:
    bool mIsTransitioning;
    Uint32 mTransitionStartTime;
    int mTransitionState;
    std::string mLevelTransitionText;
    
    const Uint32 mTransitionDuration = 3000;
    const Uint32 mFadeInOutDuration = 1000;
    
    SDL_Color mWhite = {255, 255, 255, 255};
    
    Game* mGame;
    
    Manager* mManager;
}; 