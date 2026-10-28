#include "src/common/freetype/freetype.h"

#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <ft2build.h>

#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>
#include FT_FREETYPE_H
#include FT_SYNTHESIS_H  // 这个宏对应 ftsynth.h
namespace sdlms {


void Freetype::LoadSize(uint8_t i) {
  size_ = i;
  FT_Set_Pixel_Sizes(face_, 0, i);
}

void Freetype::LoadColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  color_.r = r;
  color_.g = g;
  color_.b = b;
  color_.a = a;
}

void Freetype::Init() {
  FT_Init_FreeType(&library_);
  FT_New_Face(library_, "./Font/simsun.ttc", 0, &face_);
  FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
  return;
}

float Freetype::LoadCharW(const char16_t c) {
  static std::flat_map<FreetypeCacheKey, float> w_cache;
  FreetypeCacheKey t{
      .c = c,
      .size = size_,
      .bold = bold_,
  };
  if (!w_cache.contains(t)) {
    FT_Load_Char(face_, c, FT_LOAD_DEFAULT);
    auto w = face_->glyph->advance.x >> 6;
    w_cache[t] = w;
  }
  return w_cache.at(t);
}

float Freetype::LoadW(const std::u16string& str) {
  float w = 0;
  for (auto c : str) {
    w += LoadCharW(c);
  }
  return w;
}

float Freetype::LoadLh() {
  return face_->size->metrics.height >> 6;
}

float Freetype::LoadCh(const std::u16string& str, float w, float h) {
  return DrawCstr(str, 0, 0, w, h, true).height;
}

void Freetype::LoadAligned(bool r) {
  aligned_ = r;
}

void Freetype::LoadBold(bool r) {
  bold_ = r;
}

float Freetype::DrawChar(float x, float y, char16_t c) {
  if (c == u'\n') {
    return 0;
  }
  SDL_Texture* texture = nullptr;
  auto lineHeight = face_->size->metrics.height >> 6;
  float advance = 0;
  int bearingY = 0;
  FreetypeCacheKey t{
      .c = c,
      .size = size_,
      .bold = bold_,
  };
  if (cache_.contains(t)) {
    auto v = cache_.at(t);
    advance = v.advance;
    bearingY = v.bearingY;
    texture = v.texture;
  } else {
    FT_Load_Char(face_, c, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO);
    if (bold_) {
      FT_GlyphSlot_Embolden(face_->glyph);
    }
    auto* bitmap = &face_->glyph->bitmap;
    advance = face_->glyph->advance.x >> 6;
    bearingY = face_->glyph->metrics.horiBearingY >> 6;
    auto cWidth = bitmap->width;
    auto cHeight = bitmap->rows;
    if (cWidth == 0 || cHeight == 0) {
      return advance;
    }
    // argb888
    uint8_t* buffer = (uint8_t*)SDL_calloc(1, cWidth * cHeight * 4);
    for (int y = 0; y < bitmap->rows; ++y) {
      for (int x = 0; x < bitmap->width; ++x) {
        auto value =
            bitmap->buffer[y * bitmap->pitch + (x >> 3)] & (0x80 >> (x & 7));
        if (value != 0) {
          buffer[(y * bitmap->width + x) * 4] = 255;      // B
          buffer[(y * bitmap->width + x) * 4 + 1] = 255;  // G
          buffer[(y * bitmap->width + x) * 4 + 2] = 255;  // R
          buffer[(y * bitmap->width + x) * 4 + 3] = 255;  // A
        }
      }
    }
    texture = SDL_CreateTexture(Window::renderer_, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STATIC, cWidth, cHeight);
    SDL_UpdateTexture(texture, NULL, buffer, cWidth * 4);
    SDL_free(buffer);
    cache_[t] = {advance, bearingY, texture};
  }
  // 纹理的顶部位置 = 基线位置 - bearingY
  auto posY = y + lineHeight - bearingY;

  auto posX = x + (advance - texture->w) / 2;
  SDL_FRect posRect{
      posX,
      posY,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  if (aligned_) {
    posRect.x = int(posRect.x);
    posRect.y = int(posRect.y);
  }
  SDL_SetTextureColorMod(texture, color_.r, color_.g, color_.b);
  SDL_SetTextureAlphaMod(texture, color_.a);
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &posRect);
  return advance;
}

void Freetype::DrawLine(const std::u16string& str, float x, float y) {
  auto l = x;
  auto t = y;
  for (auto c : str) {
    l += DrawChar(l, t, c);
  }
}

void Freetype::DrawStr(const std::u16string& str, float x, float y, float w,
                       float h) {
  auto l = x;
  auto t = y;
  auto lineHeight = face_->size->metrics.height >> 6;
  lineHeight = lineHeight * h;
  for (uint32_t i = 0; i < str.size(); i++) {
    auto c = str[i];
    if (l >= x + w || c == u'\n') {
      t += lineHeight;
      l = x;
    }
    l += DrawChar(l, t, c);
  }
}

void Freetype::DrawDashLine(float x1, float y1, float x2, float y2,
                            float dash_len, float gap_len) {
  float dx = x2 - x1;
  float dy = y2 - y1;
  float total_len = SDL_sqrtf(dx * dx + dy * dy);

  if (total_len <= 0.0f)
    return;

  // 计算单位方向向量
  float ux = dx / total_len;
  float uy = dy / total_len;

  float step = dash_len + gap_len;  // 一个完整的“画+空”周期
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
    SDL_RenderLine(Window::renderer_, sx, sy, ex, ey);

    progress += step;
  }
}

Freetype::DrawData Freetype::DrawRstr(const std::u16string& str, float x,
                                      float y, float w, float h,
                                      std::optional<SDL_FRect> obstacle,
                                      int default_select, bool dryRun,
                                      int visibleCount /* = -1 */) {
  // === 常量 ===
  constexpr float kBulletOffsetX = 15.0f;
  constexpr float kDashOn = 3.0f;
  constexpr float kDashOff = 3.0f;
  constexpr float kIconLineHeight = 32.0f;
  constexpr SDL_Color kDefaultLineColor{128, 128, 128, 255};
  constexpr SDL_Color kSelectedLineColor{0, 0, 0, 255};

  // === 初始化 ===
  const float baseLineHeight =
      static_cast<float>(face_->size->metrics.height >> 6) * h;
  const float rightEdge = x + w;
  const float bottomEdge = y + h;

  auto l = x;
  auto t = y;
  float lineHeight = baseLineHeight;

  int select = -1;
  int selected = -1;
  std::vector<SDL_FRect> select_r;
  std::vector<SDL_FPoint> select_dot;
  select_r.reserve(8);
  select_dot.reserve(8);

  const bool typing = (visibleCount >= 0) && !dryRun;
  size_t drawn = 0;
  bool inOption = false;

  // === 辅助 ===
  const SDL_FRect* obs = obstacle ? &*obstacle : nullptr;

  const auto isBlocked = [obs](float px, float py, float pw,
                               float ph) noexcept {
    if (!obs)
      return false;
    const SDL_FRect rect{px, py, pw, ph};
    return SDL_HasRectIntersectionFloat(&rect, obs);
  };

  // 探测从索引 start 开始、到“下一次实际换行”之前，这一渲染行是否含图标。
  // 模拟实际渲染：累加普通字符宽度，遇到 \n 或超过右边界即停止。
  // startX 用于处理选项缩进（l 的起始位置）。
  const auto lineHasIconFrom = [&](size_t Start, float startX) -> bool {
    float used = startX - x;
    for (size_t j = Start; j < str.size(); ++j) {
      const char16_t c = str[j];

      if (c == u'\n')
        break;

      if (c == u'#') {
        if (j + 1 >= str.size())
          continue;
        const char16_t d = str[j + 1];
        if (d == u'i')
          return true;

        // 跳过转义指令
        switch (d) {
          case u'c':
          case u'b':
          case u'k':
          case u'r':
          case u'e':
          case u'n':
            ++j;
            continue;
          case u'L':
            if (j + 3 < str.size())
              j += 3;
            continue;
          case u'l':
            ++j;
            continue;
          default:
            continue;
        }
      }

      // 普通字符：累加宽度，超过右边界即视为换行
      used += LoadCharW(c);
      if (used > rightEdge - x)
        break;
    }
    return false;
  };

  const auto newline = [&] {
    t += lineHeight;
    l = x;
    lineHeight = baseLineHeight;
  };

  const auto findPlacement = [&](float charWidth) -> bool {
    while (l + charWidth <= rightEdge) {
      l += 1.0f;
      if (!isBlocked(l, t, charWidth, lineHeight))
        return true;
    }
    newline();
    while (t + lineHeight <= bottomEdge) {
      if (!isBlocked(l, t, charWidth, lineHeight))
        return true;
      t += lineHeight;
    }
    return false;
  };

  const auto shouldShow = [&]() -> bool {
    if (inOption)
      return true;
    return !typing || drawn < static_cast<size_t>(visibleCount);
  };

  SDL_Texture* icon = nullptr;

  // 进入一行时，先根据“本行是否含图标”决定行高
  bool lineChecked = false;

  // === 主循环 ===
  for (size_t i = 0; i < str.size(); ++i) {
    const char16_t c = str[i];

    // 每进入新的一行，先探测行高
    if (!lineChecked && l == x) {
      // 普通行从 x 开始探测；选项行在 #L 处理时会重新设置 l 并再次探测
      if (lineHasIconFrom(i, l)) {
        lineHeight = kIconLineHeight;
      } else {
        lineHeight = baseLineHeight;
      }
      lineChecked = true;
    }

    if (c == u'\n') {
      if (select >= 0) {
        select_r[select].w = l - x;
      }
      newline();
      lineChecked = false;
      continue;
    }

    if (c == u'#') {
      if (i + 1 >= str.size()) {
        continue;
      }
      const char16_t d = str[i + 1];
      switch (d) {
        case u'c':
          LoadColor(240, 224, 104, 255);
          ++i;
          break;
        case u'b':
          LoadColor(0, 0, 255, 255);
          ++i;
          break;
        case u'k':
          LoadColor(0, 0, 0, 255);
          ++i;
          break;
        case u'r':
          LoadColor(255, 0, 0, 255);
          ++i;
          break;
        case u'e':
          LoadBold(true);
          ++i;
          break;
        case u'n':
          LoadBold(false);
          ++i;
          break;
        case u'L': {
          if (i + 3 >= str.size())
            break;
          i += 3;
          ++select;
          l = x + kBulletOffsetX;
          inOption = true;

          // 选项行缩进后，重新探测本行是否含图标
          lineHeight =
              lineHasIconFrom(i + 1, l) ? kIconLineHeight : baseLineHeight;

          select_r.emplace_back(SDL_FRect{l, t, 0.0f, lineHeight});
          select_dot.emplace_back(
              SDL_FPoint{l - size_ * 0.5f, t + size_ * 0.5f});
          break;
        }
        case u'i': {
          auto itm_id = str.substr(i + 2, 7);
          itm_id = u"0" + itm_id;
          auto node = ItemGameInstance::LoadItemInfo(itm_id, 1);
          if (!ItemGameInstance::CheckItem(itm_id)) {
            node = EquipGameInstance::LoadEquipInfo(itm_id);
          }
          icon = WzResource::LoadTexture(node->get_child(u"icon"));

          // 行高已在本行开始时确定为 32，这里直接底部对齐绘制
          SDL_FRect pos{
              static_cast<float>((int)l),
              static_cast<float>((int)(t + lineHeight - icon->h)),
              static_cast<float>(icon->w),
              static_cast<float>(icon->h),
          };
          if (shouldShow() && !dryRun) {
            SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos);
          }

          l += static_cast<float>(icon->w);
          i += 9;
          break;
        }
        case u'l': {
          if (select >= 0) {
            select_r[select].w = l - x;
          }
          ++i;
          select = -1;
          inOption = false;
          break;
        }
        default:
          break;
      }
      continue;
    }

    // --- 普通字符 ---
    const float charWidth = LoadCharW(c);
    const float charHeight = lineHeight;

    if (isBlocked(l, t, charWidth, charHeight)) {
      if (!findPlacement(charWidth)) {
        if (!inOption)
          ++drawn;
        continue;
      }
      // 因障碍物移动后可能进入新行，重新探测行高
      if (l == x && !lineChecked) {
        lineHeight = lineHasIconFrom(i, l) ? kIconLineHeight : baseLineHeight;
        lineChecked = true;
      }
    }

    if (l + charWidth > rightEdge) {
      newline();
      lineChecked = false;
      // 换行后重新探测行高：从当前字符 i 开始，因为它要放到新行
      lineHeight = lineHasIconFrom(i, l) ? kIconLineHeight : baseLineHeight;
      lineChecked = true;
      if (isBlocked(l, t, charWidth, charHeight)) {
        if (!inOption)
          ++drawn;
        continue;
      }
    }

    const bool show = shouldShow();

    if (show) {
      if (!dryRun) {
        if (select >= 0 && selected == -1) {
          const SDL_FRect pos{l, t, charWidth, charHeight};
          if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos)) {
            selected = select;
          }
        }
        // 文字按当前行高底部对齐：draw_char 的 y 为文字顶部
        const float drawY = t + lineHeight - baseLineHeight;
        l += DrawChar(l, drawY, c);
      } else {
        l += charWidth;
      }
    } else {
      l += charWidth;
    }

    if (!inOption)
      ++drawn;
  }

  // === 选择按钮 ===
  const auto loadDot = [](bool Active) -> SDL_Texture* {
    static const auto dot0 = WzResource::LoadTexture(
        WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/dot0"));
    static const auto dot1 = WzResource::LoadTexture(
        WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/dot1"));
    return Active ? dot1 : dot0;
  };

  for (size_t i = 0; i < select_dot.size(); ++i) {
    const int idx = static_cast<int>(i);
    SDL_Texture* tex = loadDot(idx == selected || idx == default_select);
    const SDL_FRect pos{
        select_dot[i].x,
        select_dot[i].y,
        static_cast<float>(tex->w),
        static_cast<float>(tex->h),
    };
    SDL_RenderTexture(Window::renderer_, tex, nullptr, &pos);
  }

  // === 下划线 ===
  const auto drawUnderline = [&](int idx, SDL_Color color, bool dashed) {
    if (idx < 0 || idx >= static_cast<int>(select_r.size()))
      return;
    const auto& r = select_r[idx];
    const float y1 = r.y + lineHeight;
    SDL_SetRenderDrawColor(Window::renderer_, color.r, color.g, color.b,
                           color.a);
    if (dashed) {
      DrawDashLine(r.x, y1, r.x + r.w, y1, kDashOn, kDashOff);
    } else {
      SDL_RenderLine(Window::renderer_, r.x, y1, r.x + r.w, y1);
    }
  };

  drawUnderline(default_select, kDefaultLineColor, true);
  drawUnderline(selected, kSelectedLineColor, false);

  // === 返回 ===
  DrawData r;
  r.height = t - y + lineHeight;
  r.select = selected;
  return r;
}

Freetype::DrawData Freetype::DrawCstr(const std::u16string& str, float x,
                                      float y, float w, float h, bool dryRun) {
  DrawData d{.height = 0};
  if (str.empty())
    return d;

  // 行高
  float lineHeight = static_cast<float>(face_->size->metrics.height >> 6) * h;

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
        DrawLine(currentLine, midX, currentY);
        currentLine.clear();
        lineWidth = 0.0f;
      }
      currentY += lineHeight;
      continue;
    }

    // 获取字符宽度
    float charWidth = static_cast<float>(LoadW({c}));

    // 检查是否需要换行（当前行已有内容且加上新字符会超出宽度）
    if (!currentLine.empty() && lineWidth + charWidth >= w) {
      // 绘制当前行（居中）
      auto dx = (w - lineWidth) / 2.0f;
      float midX = (int)x + (int)dx;
      if (!dryRun) {
        DrawLine(currentLine, midX, currentY);
      }

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
    if (!dryRun) {
      DrawLine(currentLine, midX, currentY);
    }
  }
  d.height = currentY - y;
  d.height += lineHeight;
  return d;
}

float Freetype::LoadRh(const std::u16string& str, float w, float h,
                       std::optional<SDL_FRect> obstacle) {
  return DrawRstr(str, 0, 0, w, h, obstacle, -1, true, INT32_MAX)
      .height;  // 添加 dryRun 参数
}

Freetype::DrawData Freetype::DrawRstr(const std::u16string& str, float x,
                                      float y, float w, float h,
                                      std::optional<SDL_FRect> obstacle) {
  return DrawRstr(str, x, y, w, h, obstacle, -1, false, INT32_MAX);
}

}  // namespace sdlms
