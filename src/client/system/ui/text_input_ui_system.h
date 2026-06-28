#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include <bitset>
#include <string>

// 定义标志位索引

struct text_input {
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

  enum text_input_type : size_t {
    digit = 0,  // 第0位：允许数字
    letter = 1, // 第1位：允许字母
    symbol = 2, // 第2位：允许符号
    ime = 3,    // 第3位：允许中文
    count,
  };
  std::bitset<count> type;
};

class text_input_ui_system {
public:
  static void close(text_input &input);
  static void render(text_input &input, int x, int y);
  static void active(text_input &input);
  static bool event(SDL_Event *event, text_input &input);
};