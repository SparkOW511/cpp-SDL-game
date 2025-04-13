#include "TextureManager.hpp"

SDL_Texture* TextureManager::LoadTexture(const char* texture) {
    SDL_Surface* tempSurface = IMG_Load(texture);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(Game::renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    return tex;
}

void TextureManager::Draw(SDL_Texture* tex, SDL_Rect src, SDL_Rect dest, SDL_RendererFlip flip) {
    SDL_RenderCopyEx(Game::renderer, tex, &src, &dest, 0, NULL, flip);
}

void TextureManager::Draw(SDL_Texture* tex, SDL_Rect src, SDL_Rect dest, SDL_RendererFlip flip, Uint8 alpha) {
    // Save the current alpha mode
    Uint8 originalAlpha;
    SDL_BlendMode originalBlendMode;
    SDL_GetTextureAlphaMod(tex, &originalAlpha);
    SDL_GetTextureBlendMode(tex, &originalBlendMode);
    
    // Set the alpha value for this texture
    SDL_SetTextureAlphaMod(tex, alpha);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    
    // Draw the texture with alpha
    SDL_RenderCopyEx(Game::renderer, tex, &src, &dest, 0, NULL, flip);
    
    // Restore the original alpha mode
    SDL_SetTextureAlphaMod(tex, originalAlpha);
    SDL_SetTextureBlendMode(tex, originalBlendMode);
}