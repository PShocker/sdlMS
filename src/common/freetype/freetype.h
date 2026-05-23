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
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    auto operator<=>(const freetype_cache_key &) const = default;
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

  static inline bool aligned;

  static float load_char_w(const char16_t c);

  static float draw_char(float x, float y, char16_t c);

public:
  static void init();
  static void load_size(uint8_t i);
  static void load_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
  static float load_w(const std::u16string &str);
  static float load_lh();
  static float load_h(const std::u16string &str, float w);
  static void load_aligned(bool r);

  static void draw_line(const std::u16string &str, float x, float y);
  static void draw_str(const std::u16string &str, float x, float y, float w);
};