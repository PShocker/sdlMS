#include "text_input_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_scancode.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include <algorithm>

void text_input_ui_system::active(text_input &input) {
  input.active = true;
  SDL_Window *window = SDL_GetKeyboardFocus();
  /* Start-Stop */
  SDL_StartTextInput(window);
  SDL_SetTextInputArea(window, &input.r, 0);
}

void text_input_ui_system::render(text_input &input, int x, int y) {
  freetype::load_size(input.font_size);
  if (input.active) {
    auto delta = window::dt_now % 1000;
    if (delta <= 500) {
      auto lh = freetype::load_lh();
      auto str = input.text.substr(0, input.cur);
      auto w = freetype::load_w(str);
      // 绘制竖线
      SDL_FRect cursorRect = {
          static_cast<float>(input.r.x + x + w), // 稍微偏移使其看起来在字符间隙
          static_cast<float>(input.r.y + y + 2),
          1, // 宽度
          lh // 高度与字体匹配
      };
      SDL_SetRenderDrawColor(window::renderer, 255, 255, 255, 255);
      SDL_RenderFillRect(window::renderer, &cursorRect);
    }
  }
  freetype::load_aligned(true);
  freetype::load_bold(false);
  freetype::load_color(input.color.r, input.color.g, input.color.b,
                       input.color.a);

  auto str1 = input.text.substr(0, input.cur);
  auto str2 = input.composition;
  auto str3 = input.text.substr(input.cur);

  auto str = str1 + str2 + str3;
  freetype::draw_line(str, input.r.x + x, input.r.y + y);
  freetype::load_aligned(false);
}

bool text_input_ui_system::event(SDL_Event *event, text_input &input) {
  switch (event->type) {
  case SDL_EVENT_TEXT_EDITING: {
    auto text = event->edit.text;
    if (text[0] != '\0') {
      input.composition = freetype::load_u16str(text);
    } else {
      input.composition = u"";
    }
    break;
  }
  case SDL_EVENT_TEXT_INPUT: {
    if (input.text.length() >= input.max_size) {
      break;
    }
    auto text = event->edit.text;
    auto cur0 = input.text.length();
    auto str1 = input.text.substr(0, input.cur);
    auto str2 = freetype::load_u16str(text);
    auto str3 = input.text.substr(input.cur);

    input.text = str1 + str2 + str3;
    input.text = input.text.substr(0, input.max_size);
    auto cur1 = input.text.length();
    input.cur += cur1 - cur0;
    break;
  }
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_LEFT: {
      input.cur--;
      input.cur = std::clamp(input.cur, 0, (int)input.text.length());
      break;
    }
    case SDL_SCANCODE_RIGHT: {
      input.cur++;
      input.cur = std::clamp(input.cur, 0, (int)input.text.length());
      break;
    }
    case SDL_SCANCODE_BACKSPACE: {
      if (input.active && !input.text.empty() && input.cur > 0) {
        if (input.cur < input.text.length()) {
          input.text.erase(input.cur - 1, 1);
        } else {
          input.text.pop_back();
        }
        input.cur--;
      }
      break;
    }
    default: {
      break;
    }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      SDL_Point p = {
          static_cast<int>(window::mouse_pos.x),
          static_cast<int>(window::mouse_pos.y),
      };
      if (SDL_PointInRect(&p, &input.r)) {
        active(input);
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      SDL_Point p = {
          static_cast<int>(window::mouse_pos.x),
          static_cast<int>(window::mouse_pos.y),
      };
      if (SDL_PointInRect(&p, &input.r)) {
        auto dx = p.x - input.r.x;
        freetype::load_size(input.font_size);
        auto text_w = freetype::load_w(input.text);
        if (dx >= text_w) {
          input.cur = input.text.length();
          break;
        }
        auto w = 0;
        for (int i = 0; i < input.text.length(); i++) {
          auto c = input.text[i];
          auto cw = freetype::load_w({c});
          if (w <= dx && dx <= cw + w) {
            input.cur = i;
            break;
          }
          w += cw;
        }
      }
    }
    break;
  }
  }
  return true;
}