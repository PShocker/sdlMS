#pragma once
#include <string>
#include <ft2build.h>
#include <SDL3/SDL.h>
#include "Window.h"
#include FT_FREETYPE_H

struct FreeType
{
    static inline FT_Library *library = new FT_Library{};
    static inline FT_Face *face = new FT_Face{};

    static void init(const std::string &filename_prefix);
    static void renderGlyphToTexture(SDL_Texture *texture, char16_t c,
                                     SDL_Color color, int offsetX, int offsetY);
    static SDL_Texture *load(const std::u16string &s, SDL_Color color, int w, int size);
};