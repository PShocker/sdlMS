#include "freetype.h"
#include "src/client/window/window.h"
#include <cmath>
#include <cstdint>
#include <flat_map>
#include <ft2build.h>
#include <string>
#include FT_FREETYPE_H
#include FT_SYNTHESIS_H // 这个宏对应 ftsynth.h

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
  static std::flat_map<freetype_cache_key, float> w_cache;
  freetype_cache_key t{
      .c = c,
      .size = size,
      .r = 0,
      .g = 0,
      .b = 0,
      .a = 0,
      .bold = bold,
  };
  if (!w_cache.contains(t)) {
    FT_Load_Char(face, c, FT_LOAD_DEFAULT);
    auto w = face->glyph->advance.x >> 6;
    w_cache[t] = w;
  }
  return w_cache.at(t);
}

float freetype::load_w(const std::u16string &str) {
  float w = 0;
  for (auto c : str) {
    w += load_char_w(c);
  }
  return w;
}

float freetype::load_lh() { return face->size->metrics.height >> 6; }

float freetype::load_h(const std::u16string &str, float w, float h) {
  if (str.empty()) {
    return 0.0f;
  }

  // 行高 = 基础行高 × 倍数
  float lineHeight = static_cast<float>(face->size->metrics.height >> 6) * h;

  // 分行统计
  std::u16string current_line;
  float current_width = 0.0f;
  int line_count = 0;

  for (uint32_t i = 0; i < str.size(); i++) {
    auto c = str[i];
    float char_width = load_w({c});

    bool need_newline = false;
    if (c == u'\n') {
      need_newline = true;
    } else if (!current_line.empty() && current_width + char_width > w) {
      need_newline = true;
    }

    if (need_newline) {
      if (!current_line.empty()) {
        line_count++;
        current_line.clear();
        current_width = 0.0f;
      }
      if (c == u'\n') {
        continue;
      }
    }

    if (c == u'\n') {
      continue;
    }

    current_line.push_back(c);
    current_width += char_width;
  }

  if (!current_line.empty()) {
    line_count++;
  }

  if (line_count == 0) {
    return 0.0f;
  }

  return static_cast<float>(line_count) * lineHeight;
}

void freetype::load_aligned(bool r) { aligned = r; }

void freetype::load_bold(bool r) { bold = r; }

float freetype::draw_char(float x, float y, char16_t c) {
  if (c == u'\n') {
    return 0;
  }
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
      .bold = bold,
  };
  if (cache.contains(t)) {
    auto v = cache.at(t);
    advance = v.advance;
    bearingY = v.bearingY;
    texture = v.texture;
  } else {
    FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO);
    if (bold) {
      FT_GlyphSlot_Embolden(face->glyph);
    }
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
    posRect.x = int(posRect.x);
    posRect.y = int(posRect.y);
  }
  SDL_RenderTexture(window::renderer, texture, nullptr, &posRect);
  return advance;
}

void freetype::draw_line(const std::u16string &str, float x, float y) {
  auto l = x;
  auto t = y;
  for (auto c : str) {
    l += draw_char(l, t, c);
  }
}

void freetype::draw_str(const std::u16string &str, float x, float y, float w) {
  auto l = x;
  auto t = y;
  auto lineHeight = face->size->metrics.height >> 6;
  lineHeight = lineHeight * 1.1;
  for (uint32_t i = 0; i < str.size(); i++) {
    auto c = str[i];
    if (l >= x + w || c == u'\n') {
      t += lineHeight;
      l = x;
    }
    l += draw_char(l, t, c);
  }
}

void freetype::draw_rstr(const std::u16string &str, float x, float y, float w) {
  auto [r, g, b, a] = color;
  auto l = x;
  auto t = y;
  auto lineHeight = face->size->metrics.height >> 6;
  lineHeight = lineHeight * 1.1;
  for (uint32_t i = 0; i < str.size(); i++) {
    auto c = str[i];
    if (l >= x + w || c == u'\n') {
      t += lineHeight;
      l = x;
    }
    if (c == u'#') {
      if (i + 1 < str.size()) {
        auto d = str[i + 1];
        if (d == u'c') {
          load_color(240, 224, 104, 255);
        }
        i++;
      }
      continue;
    }
    l += draw_char(l, t, c);
  }
  load_color(r, g, b, a);
}

void freetype::draw_cstr(const std::u16string &str, float x, float y, float w) {
  if (str.empty())
    return;

  // 行高
  float lineHeight = static_cast<float>(face->size->metrics.height >> 6) * 1.3f;

  // 当前行
  std::u16string currentLine;
  float lineWidth = 0.0f;
  float currentY = y;

  for (uint32_t i = 0; i < str.size(); i++) {
    char16_t c = str[i];

    // 处理换行符
    if (c == u'\n') {
      if (!currentLine.empty()) {
        // 水平居中
        float midX = x + (w - lineWidth) / 2.0f;
        draw_line(currentLine, midX, currentY);
        currentLine.clear();
        lineWidth = 0.0f;
      }
      currentY += lineHeight;
      continue;
    }

    // 获取字符宽度
    float charWidth = static_cast<float>(load_w({c}));

    // 检查是否需要换行（当前行已有内容且加上新字符会超出宽度）
    if (!currentLine.empty() && lineWidth + charWidth >= w) {
      // 绘制当前行（居中）
      auto dx = (w - lineWidth) / 2.0f;
      float midX = (int)x + (int)dx;
      draw_line(currentLine, midX, currentY);

      // 重置当前行
      currentLine.clear();
      lineWidth = 0.0f;
      currentY += lineHeight;
    }

    // 添加字符到当前行
    currentLine.push_back(c);
    lineWidth += charWidth;
  }

  // 绘制最后一行（如果有内容）
  if (!currentLine.empty()) {
    auto dx = (w - lineWidth) / 2.0f;
    float midX = (int)x + (int)dx;
    draw_line(currentLine, midX, currentY);
  }
}

std::u16string freetype::load_u16str(const char *text) {
  size_t utf8Len = std::strlen(text);
  char *utf16Data = SDL_iconv_string("UTF-16LE", "UTF-8", text, utf8Len);
  // 直接构造，SDL已经添加了null终止符
  auto u16Ptr = reinterpret_cast<const char16_t *>(utf16Data);
  std::u16string result(u16Ptr); // 自动查找null终止符
  SDL_free(utf16Data);
  return result;
}