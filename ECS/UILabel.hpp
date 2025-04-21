#pragma once

#include "ECS.hpp"
#include "../AssetManager.hpp"
#include "../Game.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <functional>

class UILabel : public Component {
    public:
        UILabel(int xpos, int ypos, std::string text, std::string font, SDL_Color& color) : labelText(text),  labelFont(font), textColor(color){
            position.x = xpos; 
            position.y = ypos;
            labelTexture = nullptr;
            clickable = false;
            onClick = nullptr;
            hoverColor = {255, 255, 0, 255};
            isHovered = false;
            originalColor = color;
            SetLabelText(labelText, labelFont);
        }
        
        ~UILabel() {
            if (labelTexture != nullptr) {
                SDL_DestroyTexture(labelTexture);
                labelTexture = nullptr;
            }
        }
        
        void init() override {
            entity->addGroup(Game::groupUI);
        }

        void SetLabelText(std::string text, std::string font) {
            if (labelTexture != nullptr) {
                SDL_DestroyTexture(labelTexture);
                labelTexture = nullptr;
            }
            
            labelText = text;
            labelFont = font;
            
            if (text.empty()) {
                return;
            }
            
            SDL_Surface* surf = TTF_RenderText_Blended(Game::assets->GetFont(font), text.c_str(), textColor);
            labelTexture = SDL_CreateTextureFromSurface(Game::renderer, surf);
            SDL_FreeSurface(surf);

            SDL_QueryTexture(labelTexture, nullptr, nullptr, &position.w, &position.h);
        }
        
        void SetLabelText(std::string text, std::string font, SDL_Color color) {
            if (labelTexture != nullptr) {
                SDL_DestroyTexture(labelTexture);
                labelTexture = nullptr;
            }
            
            textColor = color;
            originalColor = color;
            labelText = text;
            labelFont = font;
            
            if (text.empty()) {
                return;
            }
            
            SDL_Surface* surf = TTF_RenderText_Blended(Game::assets->GetFont(font), text.c_str(), textColor);
            labelTexture = SDL_CreateTextureFromSurface(Game::renderer, surf);
            SDL_FreeSurface(surf);

            SDL_QueryTexture(labelTexture, nullptr, nullptr, &position.w, &position.h);
        }
        
        void SetTextColor(SDL_Color color) {
            if (color.r == textColor.r && color.g == textColor.g && 
                color.b == textColor.b && color.a == textColor.a) {
                return;
            }
            
            textColor = color;
            
            if (labelText.empty()) {
                return;
            }
            
            if (labelTexture != nullptr) {
                SDL_DestroyTexture(labelTexture);
                labelTexture = nullptr;
            }
            
            SDL_Surface* surf = TTF_RenderText_Blended(Game::assets->GetFont(labelFont), labelText.c_str(), textColor);
            labelTexture = SDL_CreateTextureFromSurface(Game::renderer, surf);
            SDL_FreeSurface(surf);

            SDL_QueryTexture(labelTexture, nullptr, nullptr, &position.w, &position.h);
        }
        
        void SetClickable(bool clickable) {
            this->clickable = clickable;
        }
        
        void SetOnClick(std::function<void()> onClick) {
            this->onClick = onClick;
            this->clickable = true;
        }
        
        void SetHoverColor(SDL_Color color) {
            hoverColor = color;
        }
        
        bool IsMouseOver(int mouseX, int mouseY) const {
            return (mouseX >= position.x && mouseX < position.x + position.w &&
                    mouseY >= position.y && mouseY < position.y + position.h);
        }
        
        bool HandleEvent(const SDL_Event& event) {
            if (!clickable) return false;
            
            if (event.type == SDL_MOUSEMOTION) {
                bool wasHovered = isHovered;
                isHovered = IsMouseOver(event.motion.x, event.motion.y);
                
                if (wasHovered != isHovered) {
                    if (isHovered) {
                        SetTextColor(hoverColor);
                    } else {
                        SetTextColor(originalColor);
                    }
                    return true;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                if (IsMouseOver(event.button.x, event.button.y) && onClick) {
                    Game::assets->PlaySound("click", Game::volumeLevel);
                    
                    onClick();
                    return true;
                }
            }
            
            return false;
        }
        
        void ResetHoverState() {
            isHovered = false;
            SetTextColor(originalColor);
        }
        
        bool IsHovered() const {
            return isHovered;
        }

        void draw() override{
            if (labelTexture != nullptr) {
                SDL_RenderCopy(Game::renderer, this->labelTexture, nullptr, &this->position);
            }
        }
        
        int GetWidth() const {
            return position.w;
        }
        
        int GetHeight() const {
            return position.h;
        }
        
        void SetPosition(int x, int y) {
            position.x = x;
            position.y = y;
        }
        
        SDL_Rect GetPosition() const {
            return position;
        }
        
    private:
        SDL_Rect position;
        std::string labelText;
        std::string labelFont;
        SDL_Color textColor;
        SDL_Texture* labelTexture;
        
        bool clickable;
        std::function<void()> onClick;
        SDL_Color hoverColor;
        SDL_Color originalColor;
        bool isHovered;
};