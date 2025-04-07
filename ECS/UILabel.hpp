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
            hoverColor = {255, 255, 0, 255}; // Default hover color (yellow)
            isHovered = false;
            originalColor = color; // Store original color
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
            // Clean up previous texture if it exists
            if (labelTexture != nullptr) {
                SDL_DestroyTexture(labelTexture);
                labelTexture = nullptr;
            }
            
            labelText = text;
            labelFont = font;
            
            // If the text is empty, don't create a texture
            if (text.empty()) {
                return;
            }
            
            SDL_Surface* surf = TTF_RenderText_Blended(Game::assets->GetFont(font), text.c_str(), textColor);
            labelTexture = SDL_CreateTextureFromSurface(Game::renderer, surf);
            SDL_FreeSurface(surf);

            SDL_QueryTexture(labelTexture, nullptr, nullptr, &position.w, &position.h);
        }
        
        void SetLabelText(std::string text, std::string font, SDL_Color color) {
            // Clean up previous texture if it exists
            if (labelTexture != nullptr) {
                SDL_DestroyTexture(labelTexture);
                labelTexture = nullptr;
            }
            
            textColor = color;
            originalColor = color; // Update original color when explicitly setting color
            labelText = text;
            labelFont = font;
            
            // If the text is empty, don't create a texture
            if (text.empty()) {
                return;
            }
            
            SDL_Surface* surf = TTF_RenderText_Blended(Game::assets->GetFont(font), text.c_str(), textColor);
            labelTexture = SDL_CreateTextureFromSurface(Game::renderer, surf);
            SDL_FreeSurface(surf);

            SDL_QueryTexture(labelTexture, nullptr, nullptr, &position.w, &position.h);
        }
        
        // Set the text color and recreate the texture
        void SetTextColor(SDL_Color color) {
            // If the colors are the same, no need to recreate the texture
            if (color.r == textColor.r && color.g == textColor.g && 
                color.b == textColor.b && color.a == textColor.a) {
                return;
            }
            
            textColor = color;
            
            // If the text is empty, don't create a texture
            if (labelText.empty()) {
                return;
            }
            
            // Clean up previous texture if it exists
            if (labelTexture != nullptr) {
                SDL_DestroyTexture(labelTexture);
                labelTexture = nullptr;
            }
            
            SDL_Surface* surf = TTF_RenderText_Blended(Game::assets->GetFont(labelFont), labelText.c_str(), textColor);
            labelTexture = SDL_CreateTextureFromSurface(Game::renderer, surf);
            SDL_FreeSurface(surf);

            SDL_QueryTexture(labelTexture, nullptr, nullptr, &position.w, &position.h);
        }
        
        // Make the label clickable with a callback function
        void SetClickable(bool clickable) {
            this->clickable = clickable;
        }
        
        // Set the function to call when the label is clicked
        void SetOnClick(std::function<void()> onClick) {
            this->onClick = onClick;
            this->clickable = true;
        }
        
        // Set the hover color (color that shows when mouse is over the text)
        void SetHoverColor(SDL_Color color) {
            hoverColor = color;
        }
        
        // Check if the mouse is over the label
        bool IsMouseOver(int mouseX, int mouseY) const {
            return (mouseX >= position.x && mouseX < position.x + position.w &&
                    mouseY >= position.y && mouseY < position.y + position.h);
        }
        
        // Handle mouse events (call this from Game::handleEvents)
        bool HandleEvent(const SDL_Event& event) {
            if (!clickable) return false;
            
            if (event.type == SDL_MOUSEMOTION) {
                bool wasHovered = isHovered;
                isHovered = IsMouseOver(event.motion.x, event.motion.y);
                
                // If hover state changed, update the texture
                if (wasHovered != isHovered) {
                    if (isHovered) {
                        // Save original color and set hover color
                        SetTextColor(hoverColor);
                    } else {
                        // Restore original color
                        SetTextColor(originalColor);
                    }
                    return true;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                if (IsMouseOver(event.button.x, event.button.y) && onClick) {
                    onClick();
                    return true;
                }
            }
            
            return false;
        }
        
        // Explicitly reset hover state and restore original color
        void ResetHoverState() {
            isHovered = false;
            SetTextColor(originalColor);
        }
        
        // Check if element is currently hovered
        bool IsHovered() const {
            return isHovered;
        }

        void draw() override{
            if (labelTexture != nullptr) {
                SDL_RenderCopy(Game::renderer, this->labelTexture, nullptr, &this->position);
            }
        }
        
        // Get width of the rendered text
        int GetWidth() const {
            return position.w;
        }
        
        // Get height of the rendered text
        int GetHeight() const {
            return position.h;
        }
        
        // Set position of the label
        void SetPosition(int x, int y) {
            position.x = x;
            position.y = y;
        }
        
        // Get the position of the label
        SDL_Rect GetPosition() const {
            return position;
        }
        
    private:
        SDL_Rect position;
        std::string labelText;
        std::string labelFont;
        SDL_Color textColor;
        SDL_Texture* labelTexture;
        
        // Clickable properties
        bool clickable;
        std::function<void()> onClick;
        SDL_Color hoverColor;
        SDL_Color originalColor;
        bool isHovered;
};