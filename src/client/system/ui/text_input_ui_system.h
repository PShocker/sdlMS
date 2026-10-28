#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include <bitset>
#include <optional>
#include <string>

// 定义标志位索引

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
    kDigit = 0,  // 第0位：允许数字
    kLetter = 1, // 第1位：允许字母
    kSymbol = 2, // 第2位：允许符号
    kIme = 3,    // 第3位：允许中文
    kCount,
  };
  std::bitset<kCount> type;
};

class TextInputUiSystem {
public:
  static void close(TextInput &input);
  static void render(TextInput &input, int x, int y);
  static void active(TextInput &input);
  static bool event(SDL_Event *event, TextInput &input);
};