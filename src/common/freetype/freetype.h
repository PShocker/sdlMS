#pragma once

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <ft2build.h>

#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>

#include FT_FREETYPE_H

namespace sdlms {

class Freetype {
  struct FreetypeCacheKey {
    char16_t c;
    uint8_t size;
    bool bold;

    auto operator<=>(const FreetypeCacheKey&) const = default;
  };

  struct FreetypeCacheValue {
    float advance;
    int32_t bearingY;
    SDL_Texture* texture;
  };

 private:
  static inline FT_Library library_ = nullptr;
  static inline FT_Face face_ = nullptr;
  static inline uint8_t size_;
  static inline SDL_Color color_;
  static inline std::flat_map<FreetypeCacheKey, FreetypeCacheValue> cache_;

  static inline bool aligned_;
  static inline bool bold_;

  static float LoadCharW(const char16_t c);

  static float DrawChar(float x, float y, char16_t c);

  static void DrawDashLine(float x1, float y1, float x2, float y2,
                           float dash_len, float gap_len);

 public:
  static void Init();
  static void LoadSize(uint8_t i);
  static void LoadColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
  static float LoadW(const std::u16string& str);
  static float LoadLh();

  static void LoadAligned(bool r);
  static void LoadBold(bool r);

  static float LoadCh(const std::u16string& str, float w, float h);
  static float LoadRh(const std::u16string& str, float w, float h,
                      std::optional<SDL_FRect> obstacle);

  static void DrawLine(const std::u16string& str, float x, float y);

  static void DrawStr(const std::u16string& str, float x, float y, float w,
                      float h);
  struct DrawData {
    float height;
    int select;
  };
  static DrawData DrawRstr(const std::u16string& str, float x, float y, float w,
                           float h, std::optional<SDL_FRect> obstacle);
  static DrawData DrawRstr(const std::u16string& str, float x, float y, float w,
                           float h, std::optional<SDL_FRect> obstacle,
                           int default_select, bool dryRun, int visibleCount);
  static DrawData DrawCstr(const std::u16string& str, float x, float y, float w,
                           float h, bool dryRun = false);
};

}  // namespace sdlms
