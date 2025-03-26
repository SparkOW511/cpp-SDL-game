#pragma once

#include "ECS.hpp"
#include "../AssetManager.hpp"
#include "../Game.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

class UILabel : public Component {
    public:
        UILabel(int xpos, int ypos, std::string text, std::string font, SDL_Color& color) : labelText(text),  labelFont(font), textColor(color){
            position.x = xpos; 
            position.y = ypos;

            SetLabelText(labelText, labelFont);
        }
        
        ~UILabel() {}
        
        void init() override {
            entity->addGroup(Game::groupUI);
        }

        void SetLabelText(std::string text, std::string font) {
            SDL_Surface* surf = TTF_RenderText_Blended(Game::assets->GetFont(font), text.c_str(), textColor);
            labelTexture = SDL_CreateTextureFromSurface(Game::renderer, surf);
            SDL_FreeSurface(surf);

            SDL_QueryTexture(labelTexture, nullptr, nullptr, &position.w, &position.h);
        }
        
        void SetLabelText(std::string text, std::string font, SDL_Color color) {
            textColor = color;
            SDL_Surface* surf = TTF_RenderText_Blended(Game::assets->GetFont(font), text.c_str(), textColor);
            labelTexture = SDL_CreateTextureFromSurface(Game::renderer, surf);
            SDL_FreeSurface(surf);

            SDL_QueryTexture(labelTexture, nullptr, nullptr, &position.w, &position.h);
        }

        void draw() override{
            SDL_RenderCopy(Game::renderer, this->labelTexture, nullptr, &this->position);
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
        
    private:
        SDL_Rect position;
        std::string labelText;
        std::string labelFont;
        SDL_Color textColor;
        SDL_Texture* labelTexture;
};