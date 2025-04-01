#include "TransitionManager.hpp"
#include "Game.hpp"
#include "ECS/Components.hpp"
#include <sstream>

TransitionManager::TransitionManager() : 
    mIsTransitioning(false),
    mTransitionStartTime(0),
    mTransitionState(0),
    mLevelTransitionText(""),
    mGame(nullptr),
    mManager(nullptr),
    mTransitionLabel(nullptr)
{
}

TransitionManager::~TransitionManager() {
    // No dynamic memory to clean up
}

void TransitionManager::init(Game* game, Manager* manager) {
    mGame = game;
    mManager = manager;
    
    // Create transition label entity
    if (mTransitionLabel == nullptr && mManager != nullptr) {
        mTransitionLabel = &mManager->addEntity();
        mTransitionLabel->addComponent<UILabel>(0, 0, "", "font2", mWhite);
    }
}

void TransitionManager::startTransition(int currentLevel, int nextLevel) {
    // Start transition sequence
    mIsTransitioning = true;
    mTransitionStartTime = SDL_GetTicks();
    mTransitionState = 0; // Start with fade in
    
    // Set transition text to show between levels
    std::stringstream levelMessage;
    levelMessage << "Level " << nextLevel;
    mLevelTransitionText = levelMessage.str();
    
    // Center the transition text
    if (mTransitionLabel != nullptr) {
        mTransitionLabel->getComponent<UILabel>().SetLabelText(mLevelTransitionText, "font2", mWhite);
        int textWidth = mTransitionLabel->getComponent<UILabel>().GetWidth();
        int textHeight = mTransitionLabel->getComponent<UILabel>().GetHeight();
        int xPos = (1920 - textWidth) / 2;
        int yPos = (1080 - textHeight) / 2;
        mTransitionLabel->getComponent<UILabel>().SetPosition(xPos, yPos);
    }
}

bool TransitionManager::updateTransition() {
    if (!mIsTransitioning) {
        return false;
    }
    
    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsedTime = currentTime - mTransitionStartTime;
    
    // Update transition state based on elapsed time
    if (mTransitionState == 0) { // Fade in
        if (elapsedTime >= mFadeInOutDuration) {
            mTransitionState = 1; // Move to showing level text
            mTransitionStartTime = currentTime;
        }
    }
    else if (mTransitionState == 1) { // Show level text
        if (elapsedTime >= mTransitionDuration - (2 * mFadeInOutDuration)) {
            mTransitionState = 2; // Move to fade out
            mTransitionStartTime = currentTime;
        }
    }
    else if (mTransitionState == 2) { // Fade out
        if (elapsedTime >= mFadeInOutDuration) {
            // Transition complete, reset transition state
            mIsTransitioning = false;
            
            // Clear transition text
            if (mTransitionLabel != nullptr) {
                mTransitionLabel->getComponent<UILabel>().SetLabelText("", "font2");
            }
            
            // Return true to signal the transition is complete
            return true;
        }
    }
    
    // Transition still in progress
    return false;
}

void TransitionManager::renderTransition() {
    if (!mIsTransitioning || mGame == nullptr) {
        return;
    }
    
    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsedTime = currentTime - mTransitionStartTime;
    
    SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_BLEND);
    
    // Calculate alpha based on transition state
    Uint8 alpha = 0;
    
    if (mTransitionState == 0) { // Fade in
        float progress = static_cast<float>(elapsedTime) / mFadeInOutDuration;
        alpha = static_cast<Uint8>(255 * progress);
    }
    else if (mTransitionState == 1) { // Show level text
        alpha = 255; // Fully opaque
    }
    else if (mTransitionState == 2) { // Fade out
        float progress = 1.0f - (static_cast<float>(elapsedTime) / mFadeInOutDuration);
        alpha = static_cast<Uint8>(255 * progress);
    }
    
    // Fill screen with black at calculated alpha
    SDL_SetRenderDrawColor(Game::renderer, 0, 0, 0, alpha);
    SDL_Rect fullScreen = {0, 0, 1920, 1080};
    SDL_RenderFillRect(Game::renderer, &fullScreen);
    
    // Show level text if in state 1 or 2
    if (mTransitionState >= 1 && mTransitionLabel != nullptr) {
        mTransitionLabel->draw();
    }
    
    // Reset renderer settings
    SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
    SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_NONE);
} 