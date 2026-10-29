#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>

#include <bitset>
#include <optional>
#include <string>

namespace sdlms {

// A single-line text field with selection, cursor blinking and IME support.
//
// The owning UI stores the component, calls Active()/Close() to switch focus
// and forwards events to Event(); Render() draws the field at an offset.
struct TextInput {
  int max_size;
  std::u16string text;
  std::u16string composition;
  bool disable = false;
  bool active = false;
  SDL_Rect r;

  SDL_Color font_color;
  SDL_Color cur_color = {255, 255, 255, 255};
  int font_size;
  int cur = 0;
  std::optional<int> cur_next = std::nullopt;

  enum TextInputType : size_t {
    kDigit = 0,   // 第0位：允许数字
    kLetter = 1,  // 第1位：允许字母
    kSymbol = 2,  // 第2位：允许符号
    kIme = 3,     // 第3位：允许中文
    kCount,
  };
  std::bitset<kCount> type;

  // Leaves the field focused and closes its IME composition.
  void Close();

  // Focuses the field and starts IME input when the field allows it.
  void Active();

  // Draws the field content at (x, y), relative to `r`.
  void Render(int x, int y) const;

  // Handles one event for this field; returns true when a click fell outside.
  bool Event(SDL_Event* event);
};

}  // namespace sdlms
