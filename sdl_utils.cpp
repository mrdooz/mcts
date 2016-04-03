#include "sdl_utils.hpp"

extern SDL_Renderer* g_renderer;

//------------------------------------------------------------------------------
SDL_Texture* RenderText(const char* message, const char* fontFile, SDL_Color color, int fontSize)
{
  TTF_Font* font = TTF_OpenFont(fontFile, fontSize);
  if (font == nullptr)
  {
    return nullptr;
  }

  SDL_Surface* surface = TTF_RenderText_Blended(font, message, color);
  if (surface == nullptr)
  {
    TTF_CloseFont(font);
    return nullptr;
  }

  SDL_Texture* texture = SDL_CreateTextureFromSurface(g_renderer, surface);

  SDL_FreeSurface(surface);
  TTF_CloseFont(font);
  return texture;
}
