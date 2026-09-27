#include "freetype.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <cstdint>
#include <flat_map>
#include <ft2build.h>
#include <string>
#include <vector>
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

void freetype::draw_str(const std::u16string &str, float x, float y, float w,
                        float h) {
  auto l = x;
  auto t = y;
  auto lineHeight = face->size->metrics.height >> 6;
  lineHeight = lineHeight * h;
  for (uint32_t i = 0; i < str.size(); i++) {
    auto c = str[i];
    if (l >= x + w || c == u'\n') {
      t += lineHeight;
      l = x;
    }
    l += draw_char(l, t, c);
  }
}

void freetype::draw_dash_line(float x1, float y1, float x2, float y2,
                              float dash_len, float gap_len) {
  float dx = x2 - x1;
  float dy = y2 - y1;
  float total_len = SDL_sqrtf(dx * dx + dy * dy);

  if (total_len <= 0.0f)
    return;

  // 计算单位方向向量
  float ux = dx / total_len;
  float uy = dy / total_len;

  float step = dash_len + gap_len; // 一个完整的“画+空”周期
  float progress = 0.0f;

  while (progress < total_len) {
    // 当前段起点
    float sx = x1 + ux * progress;
    float sy = y1 + uy * progress;

    // 当前段终点（确保不超过总长）
    float remain = total_len - progress;
    float current_dash = (remain < dash_len) ? remain : dash_len;

    float ex = sx + ux * current_dash;
    float ey = sy + uy * current_dash;

    // 绘制这一小段
    SDL_RenderLine(window::renderer, sx, sy, ex, ey);

    progress += step;
  }
}

freetype::rstr_return_data
freetype::draw_rstr(const std::u16string &str, float x, float y, float w,
                    float h, std::optional<SDL_FRect> obstacle,
                    int default_select, bool dryRun) {
  auto l = x;
  auto t = y;
  auto lineHeight = face->size->metrics.height >> 6;
  lineHeight = lineHeight * h;
  // 辅助函数
  const auto isBlocked = [&](float px, float py, float pw, float ph) -> bool {
    if (!obstacle.has_value())
      return false;
    SDL_FRect rect = {px, py, pw, ph};
    const auto &obs = obstacle.value();
    return SDL_HasRectIntersectionFloat(&rect, &obs);
  };

  int select = -1;
  int selected = -1;
  std::vector<SDL_FRect> select_r;
  std::vector<SDL_FPoint> select_dot;

  for (uint32_t i = 0; i < str.size(); i++) {
    auto c = str[i];
    if (c == u'\n') {
      if (select >= 0) {
        select_r[select].w = l - x;
      }
      t += lineHeight;
      l = x;
      continue;
    }

    // 控制字符处理
    if (c == u'#') {
      if (i + 1 < str.size()) {
        auto d = str[i + 1];
        switch (d) {
        case u'c': {
          load_color(240, 224, 104, 255);
          i++;
          break;
        }
        case u'b': {
          load_color(0, 0, 255, 255);
          i++;
          break;
        }
        case u'k': {
          load_color(0, 0, 0, 255);
          i++;
          break;
        }
        case u'r': {
          load_color(255, 0, 0, 255);
          i++;
          break;
        }
        case u'e': {
          load_bold(true);
          i++;
          break;
        }
        case u'n': {
          load_bold(false);
          i++;
          break;
        }
        case u'L': {
          i += 3;
          select++;
          l = x + 15;

          SDL_FRect r{
              .x = l,
              .y = t,
              .w = 0,
              .h = static_cast<float>(lineHeight),
          };
          select_r.emplace_back(r);
          select_dot.push_back({
              .x = l - size / 2,
              .y = t + size / 2,
          });
          break;
        }
        case u'l': {
          select_r[select].w = l - x;
          i++;
          select = -1;
          break;
        }
        }
      }
      continue;
    }

    // 获取字符宽度
    float charWidth = load_char_w(c);
    float charHeight = lineHeight;

    // 检查是否被遮挡
    if (isBlocked(l, t, charWidth, charHeight)) {
      bool placed = false;

      // 尝试向右移动
      while (l + charWidth <= x + w) {
        l += 1.0f;
        if (!isBlocked(l, t, charWidth, charHeight)) {
          placed = true;
          break;
        }
      }

      if (!placed) {
        // 换行
        t += lineHeight;
        l = x;

        // 继续换行直到找到可用位置
        while (t + lineHeight <= y + h) {
          if (!isBlocked(l, t, charWidth, charHeight)) {
            placed = true;
            break;
          }
          t += lineHeight;
        }

        if (!placed) {
          continue; // 无法放置，跳过此字符
        }
      }
    }

    // 边界检查
    if (l + charWidth > x + w) {
      t += lineHeight;
      l = x;
      if (isBlocked(l, t, charWidth, charHeight)) {
        continue;
      }
    }

    // 根据模式决定是绘制还是仅计算
    if (!dryRun) {
      if (select >= 0 && selected == -1) {
        const auto &mouse_pos = window::mouse_pos;
        SDL_FRect pos{.x = l, .y = t, .w = charWidth, .h = charHeight};
        if (SDL_PointInRectFloat(&mouse_pos, &pos)) {
          selected = select;
        }
      }
      l += draw_char(l, t, c);

    } else {
      l += charWidth; // 仅移动位置
    }
  }
  for (int i = 0; i < select_dot.size(); i++) {
    SDL_Texture *t;
    if (i == selected || i == default_select) {
      static auto dot1 = wz_resource::load_texture(
          wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/dot1"));
      t = dot1;
    } else {
      static auto dot0 = wz_resource::load_texture(
          wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/dot0"));
      t = dot0;
    }
    SDL_FRect pos{
        select_dot[i].x,
        select_dot[i].y,
        static_cast<float>(t->w),
        static_cast<float>(t->h),
    };
    SDL_RenderTexture(window::renderer, t, nullptr, &pos);
  }
  // render default_line
  if (default_select >= 0) {
    auto &r = select_r[default_select];
    SDL_SetRenderDrawColor(window::renderer, 128, 128, 128, 255);
    auto x1 = r.x;
    auto y1 = r.y + lineHeight;
    auto x2 = x1 + r.w;
    auto y2 = y1;
    draw_dash_line(x1, y1, x2, y2, 3, 3);
  }

  if (selected >= 0) {
    auto &r = select_r[selected];
    SDL_SetRenderDrawColor(window::renderer, 0, 0, 0, 255);
    auto x1 = r.x;
    auto y1 = r.y + lineHeight;
    auto x2 = x1 + r.w;
    auto y2 = y1;
    SDL_RenderLine(window::renderer, x1, y1, x2, y2);
  }
  rstr_return_data r;
  r.height = (t - y + lineHeight);
  r.select = select;
  return r;
}

void freetype::draw_cstr(const std::u16string &str, float x, float y, float w,
                         float h) {
  if (str.empty())
    return;

  // 行高
  float lineHeight = static_cast<float>(face->size->metrics.height >> 6) * h;

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

float freetype::load_rh(const std::u16string &str, float w, float h,
                        std::optional<SDL_FRect> obstacle) {
  return draw_rstr(str, 0, 0, w, h, obstacle, true).height; // 添加 dryRun 参数
}