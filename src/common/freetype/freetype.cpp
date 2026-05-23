#include "freetype.h"
#include "src/client/window/window.h"
#include <cstdint>
#include <flat_map>
#include <string>

void freetype::load_size(uint8_t i) {
  size = i;
  FT_Set_Pixel_Sizes(face, 0, i);
}

void freetype::load_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  color.r = r;
  color.g = g;
  color.b = b;
  color.a = a;
}

void freetype::init() {
  FT_Init_FreeType(&library);
  FT_New_Face(library, "./Font/simsun.ttc", 0, &face);
  FT_Select_Charmap(face, FT_ENCODING_UNICODE);
  return;
}

float freetype::load_char_w(const char16_t c) {
  static std::flat_map<char16_t, float> cache;
  if (!cache.contains(c)) {
    FT_Load_Char(face, c, FT_LOAD_DEFAULT);
    auto w = face->glyph->advance.x >> 6;
    cache[c] = w;
  }
  return cache.at(c);
}

float freetype::load_w(const std::u16string &str) {
  float w = 0;
  for (auto c : str) {
    w += load_char_w(c);
  }
  return w;
}

float freetype::load_lh() { return face->size->metrics.height >> 6; }

float freetype::load_h(const std::u16string &str, float w) {
  uint32_t line = 1;
  auto lineHeight = face->size->metrics.height >> 6;
  float lineWidth = 0;
  for (auto c : str) {
    lineWidth += load_char_w(c);
    if (lineWidth >= w) {
      // 换行
      line++;
      lineWidth = 0;
    }
  }
  return line * lineHeight;
}

void freetype::load_aligned(bool r) { aligned = r; }

float freetype::draw_char(float x, float y, char16_t c) {
  SDL_Texture *texture = nullptr;
  auto lineHeight = face->size->metrics.height >> 6;
  float advance = 0;
  int bearingY = 0;
  freetype_cache_key t{
      .c = c,
      .size = size,
      .r = color.r,
      .g = color.g,
      .b = color.b,
      .a = color.a,
  };
  if (cache.contains(t)) {
    auto v = cache.at(t);
    advance = v.advance;
    bearingY = v.bearingY;
    texture = v.texture;
  } else {
    FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO);
    auto *bitmap = &face->glyph->bitmap;
    advance = face->glyph->advance.x >> 6;
    bearingY = face->glyph->metrics.horiBearingY >> 6;
    auto cWidth = bitmap->width;
    auto cHeight = bitmap->rows;
    if (cWidth == 0 || cHeight == 0) {
      return advance;
    }
    // argb888
    uint8_t *buffer = (uint8_t *)SDL_calloc(1, cWidth * cHeight * 4);
    for (int y = 0; y < bitmap->rows; ++y) {
      for (int x = 0; x < bitmap->width; ++x) {
        auto value =
            bitmap->buffer[y * bitmap->pitch + (x >> 3)] & (0x80 >> (x & 7));
        if (value != 0) {
          buffer[(y * bitmap->width + x) * 4] = color.b;     // B
          buffer[(y * bitmap->width + x) * 4 + 1] = color.g; // G
          buffer[(y * bitmap->width + x) * 4 + 2] = color.r; // R
          buffer[(y * bitmap->width + x) * 4 + 3] = color.a; // A
        }
      }
    }
    texture = SDL_CreateTexture(window::renderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STATIC, cWidth, cHeight);
    SDL_UpdateTexture(texture, NULL, buffer, cWidth * 4);
    SDL_free(buffer);
    cache[t] = {advance, bearingY, texture};
  }
  // 纹理的顶部位置 = 基线位置 - bearingY
  auto posY = y + lineHeight - bearingY;

  auto posX = x + (advance - texture->w) / 2;
  SDL_FRect posRect{posX, posY, static_cast<float>(texture->w),
                    static_cast<float>(texture->h)};
  if (aligned) {
    posRect.x = (int)posRect.x;
    posRect.y = (int)posRect.y;
  }
  SDL_RenderTexture(window::renderer, texture, nullptr, &posRect);
  return advance;
}

void freetype::draw_line(const std::u16string &str, float x, float y) {
  auto l = x;
  auto t = y;
  auto lineHeight = face->size->metrics.height >> 6;

  for (auto c : str) {
    l += draw_char(l, t, c);
  }
}

void freetype::draw_str(const std::u16string &str, float x, float y, float w) {
  auto l = x;
  auto t = y;
  auto lineHeight = face->size->metrics.height >> 6;
  for (auto c : str) {
    l += draw_char(l, t, c);
    if (l >= x + w) {
      t += lineHeight;
      l = x;
    }
  }
}