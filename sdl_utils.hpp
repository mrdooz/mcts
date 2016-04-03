#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

SDL_Texture* RenderText(const char* message, const char* fontFile, SDL_Color color, int fontSize);
