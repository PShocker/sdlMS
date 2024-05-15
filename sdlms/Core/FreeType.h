#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>
#include <cstdint>
#include <string>
#include <SDL2/SDL.h>
#include "Components/Sprite.h"

class FreeType
{
public:
    static void init();
    static inline FT_Library *library;
    static inline FT_Face *face;
    static Sprite *str(const std::u16string &s, int font_size = 12, SDL_Renderer *renderer = Window::get_renderer());
};