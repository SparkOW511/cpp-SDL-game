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
    
    // Initialize with reference to game and entity manager
    void init(Game* game, Manager* manager);
    
    // Start a transition to the next level
    void startTransition(int currentLevel, int nextLevel);
    
    // Update transition state based on elapsed time
    bool updateTransition();
    
    // Render transition effects
    void renderTransition();
    
    // Get the current transition state
    bool isTransitioning() const { return mIsTransitioning; }
    
private:
    // Transition states and timing
    bool mIsTransitioning;
    Uint32 mTransitionStartTime;
    int mTransitionState; // 0: fade in, 1: show level text, 2: fade out
    std::string mLevelTransitionText;
    
    // Transition durations
    const Uint32 mTransitionDuration = 3000;  // Total transition duration in milliseconds
    const Uint32 mFadeInOutDuration = 1000;   // Duration of fade in/out in milliseconds
    
    // Colors
    SDL_Color mWhite = {255, 255, 255, 255};
    
    // Reference to the game
    Game* mGame;
    
    // Reference to the entity manager
    Manager* mManager;
    
    // UI Entity for transition text
    Entity* mTransitionLabel;
}; 