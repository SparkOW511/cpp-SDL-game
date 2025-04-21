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
}

void TransitionManager::init(Game* game, Manager* manager) {
    mGame = game;
    mManager = manager;
    
    if (mTransitionLabel == nullptr && mManager != nullptr) {
        mTransitionLabel = &mManager->addEntity();
        mTransitionLabel->addComponent<UILabel>(0, 0, "", "font2", mWhite);
    }
}

void TransitionManager::startTransition(int currentLevel, int nextLevel) {
    mIsTransitioning = true;
    mTransitionStartTime = SDL_GetTicks();
    mTransitionState = 0;
    
    std::stringstream levelMessage;
    levelMessage << "Level " << nextLevel;
    mLevelTransitionText = levelMessage.str();
    
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
    
    if (mTransitionState == 0) {
        if (elapsedTime >= mFadeInOutDuration) {
            mTransitionState = 1;
            mTransitionStartTime = currentTime;
        }
    }
    else if (mTransitionState == 1) {
        if (elapsedTime >= mTransitionDuration - (2 * mFadeInOutDuration)) {
            mTransitionState = 2;
            mTransitionStartTime = currentTime;
        }
    }
    else if (mTransitionState == 2) {
        if (elapsedTime >= mFadeInOutDuration) {
            mIsTransitioning = false;
            
            if (mTransitionLabel != nullptr) {
                mTransitionLabel->getComponent<UILabel>().SetLabelText("", "font2");
            }
            
            return true;
        }
    }
    
    return false;
}

void TransitionManager::renderTransition() {
    if (!mIsTransitioning || mGame == nullptr) {
        return;
    }
    
    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsedTime = currentTime - mTransitionStartTime;
    
    SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_BLEND);
    
    Uint8 alpha = 0;
    
    if (mTransitionState == 0) {
        float progress = static_cast<float>(elapsedTime) / mFadeInOutDuration;
        alpha = static_cast<Uint8>(255 * progress);
    }
    else if (mTransitionState == 1) {
        alpha = 255;
    }
    else if (mTransitionState == 2) {
        float progress = 1.0f - (static_cast<float>(elapsedTime) / mFadeInOutDuration);
        alpha = static_cast<Uint8>(255 * progress);
    }
    
    SDL_SetRenderDrawColor(Game::renderer, 0, 0, 0, alpha);
    SDL_Rect fullScreen = {0, 0, 1920, 1080};
    SDL_RenderFillRect(Game::renderer, &fullScreen);
    
    if (mTransitionState >= 1 && mTransitionLabel != nullptr) {
        mTransitionLabel->draw();
    }
    
    SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
    SDL_SetRenderDrawBlendMode(Game::renderer, SDL_BLENDMODE_NONE);
} 