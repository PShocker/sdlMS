#pragma once
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_render.h"
#include <cstdint>
#include <flat_map>
#include <ft2build.h>

#include FT_FREETYPE_H

class freetype {
  struct freetype_cache_key {
    char16_t c;
    uint8_t size;
    SDL_Color color;
  };

  struct freetype_cache_value {
    float advance;
    int32_t bearingY;
    SDL_Texture *texture;
  };

private:
  static inline FT_Library library = nullptr;
  static inline FT_Face face = nullptr;
  static inline uint8_t size;
  static inline SDL_Color color;
  static inline std::flat_map<freetype_cache_key, freetype_cache_value> cache;

public:
  static void init();
};