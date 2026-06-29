#include "text_input_ui_system.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_scancode.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include <algorithm>
#include <optional>

void text_input_ui_system::close(text_input &input) {
  input.active = false;
  if (input.type.test(text_input::ime)) {
    SDL_StopTextInput(SDL_GetKeyboardFocus());
  }
}

void text_input_ui_system::active(text_input &input) {
  input.active = true;
  if (input.type.test(text_input::ime)) {
    SDL_Window *window = SDL_GetKeyboardFocus();
    SDL_StartTextInput(window);
    SDL_SetTextInputArea(window, &input.r, 0);
  }
}

void text_input_ui_system::render(text_input &input, int x, int y) {
  // ============================================================
  // 1. 基础设置和坐标计算
  // ============================================================
  freetype::load_size(input.font_size);

  const float line_height = freetype::load_lh();
  const float rect_x = static_cast<float>(input.r.x + x);
  const float rect_y = static_cast<float>(input.r.y + y);
  const float max_width = static_cast<float>(input.r.w);

  // ============================================================
  // 2. 构建完整文本（当前文本 + 输入法组合文本）
  // ============================================================
  const std::u16string text_before_cursor = input.text.substr(0, input.cur);
  const std::u16string text_after_cursor = input.text.substr(input.cur);
  const std::u16string full_text =
      text_before_cursor + input.composition + text_after_cursor;

  // ============================================================
  // 3. 确定选区范围
  // ============================================================
  const bool has_selection = input.cur_next.has_value();
  const size_t sel_start =
      has_selection ? std::min(input.cur, input.cur_next.value()) : input.cur;
  const size_t sel_end =
      has_selection ? std::max(input.cur, input.cur_next.value()) : input.cur;

  // ============================================================
  // 4. 光标渲染（仅在激活且无选区时显示闪烁光标）
  // ============================================================
  if (input.active && !has_selection) {
    const int blink_period = 500;
    if ((window::dt_now % (blink_period * 2)) < blink_period) {
      const float cursor_x = freetype::load_w(text_before_cursor);

      SDL_FRect cursor_rect = {rect_x + cursor_x, rect_y + 2.0f, 1.0f,
                               line_height};

      SDL_SetRenderDrawColor(window::renderer, input.cur_color.r,
                             input.cur_color.g, input.cur_color.b,
                             input.cur_color.a);
      SDL_RenderFillRect(window::renderer, &cursor_rect);
    }
  }

  // ============================================================
  // 5. 文本渲染配置
  // ============================================================
  freetype::load_aligned(true);
  freetype::load_bold(false);
  freetype::load_color(input.font_color.r, input.font_color.g,
                       input.font_color.b, input.font_color.a);

  // ============================================================
  // 6. 文本裁剪（保持光标/选区可见）
  // ============================================================
  std::u16string display_text;
  size_t display_offset = 0;

  if (full_text.empty()) {
    display_text = full_text;
  } else {
    const size_t focus_pos = has_selection ? sel_start : input.cur;
    float full_width = freetype::load_w(full_text);

    if (full_width > max_width) {
      // 以 focus_pos 为中心，向两侧扩展
      size_t left = focus_pos;
      size_t right = focus_pos;

      // 尝试向左扩展
      while (left > 0) {
        size_t new_left = left - 1;
        auto test_text = full_text.substr(new_left, right - new_left);
        if (freetype::load_w(test_text) > max_width)
          break;
        left = new_left;
      }

      // 尝试向右扩展
      while (right < full_text.length()) {
        size_t new_right = right + 1;
        auto test_text = full_text.substr(left, new_right - left);
        if (freetype::load_w(test_text) > max_width)
          break;
        right = new_right;
      }

      // 如果仍然超出，从右侧逐步裁剪
      float current_width =
          freetype::load_w(full_text.substr(left, right - left));
      while (current_width > max_width && right > left + 1) {
        right--;
        current_width = freetype::load_w(full_text.substr(left, right - left));
      }

      // 如果还是超出，从左侧裁剪（极端情况）
      while (current_width > max_width && left < right - 1) {
        left++;
        current_width = freetype::load_w(full_text.substr(left, right - left));
      }

      display_offset = left;
      display_text = full_text.substr(left, right - left);

      if (display_text.empty() && !full_text.empty()) {
        display_text = full_text.substr(0, 1);
        display_offset = 0;
      }
    } else {
      display_text = full_text;
      display_offset = 0;
    }
  }

  // ============================================================
  // 7. 绘制选区背景（蓝色高亮）
  // ============================================================
  if (has_selection) {
    // 计算选区在显示文本中的相对位置
    const int sel_start_adj = static_cast<int>(sel_start - display_offset);
    const int sel_end_adj = static_cast<int>(sel_end - display_offset);

    const int clip_start = std::max(0, sel_start_adj);
    const int clip_end =
        std::min(static_cast<int>(display_text.length()), sel_end_adj);

    if (clip_start < clip_end) {
      const auto text_before_sel = display_text.substr(0, clip_start);
      const auto text_selected =
          display_text.substr(clip_start, clip_end - clip_start);

      const float sel_x = freetype::load_w(text_before_sel);
      const float sel_width = freetype::load_w(text_selected);

      SDL_FRect sel_rect = {rect_x + sel_x, rect_y + 2.0f, sel_width,
                            line_height};

      SDL_SetRenderDrawColor(window::renderer, 60, 140, 255, 80);
      SDL_RenderFillRect(window::renderer, &sel_rect);
    }
  }

  // ============================================================
  // 8. 分段绘制文本（支持选中高亮）
  // ============================================================
  if (has_selection) {
    const int sel_start_adj = static_cast<int>(sel_start - display_offset);
    const int sel_end_adj = static_cast<int>(sel_end - display_offset);

    const int clip_start = std::max(0, sel_start_adj);
    const int clip_end =
        std::min(static_cast<int>(display_text.length()), sel_end_adj);

    float draw_x = rect_x;

    // 8.1 绘制选区前的文本
    if (clip_start > 0) {
      const auto text_before = display_text.substr(0, clip_start);
      freetype::draw_line(text_before, draw_x, rect_y);
      draw_x += freetype::load_w(text_before);
    }

    // 8.2 绘制选区文本（白色高亮）
    if (clip_start < clip_end) {
      const auto text_selected =
          display_text.substr(clip_start, clip_end - clip_start);
      const auto old_color = input.font_color;
      freetype::load_color(255, 255, 255, 255);
      freetype::draw_line(text_selected, draw_x, rect_y);
      draw_x += freetype::load_w(text_selected);
      freetype::load_color(old_color.r, old_color.g, old_color.b, old_color.a);
    }

    // 8.3 绘制选区后的文本
    if (clip_end < static_cast<int>(display_text.length())) {
      const auto text_after = display_text.substr(clip_end);
      freetype::draw_line(text_after, draw_x, rect_y);
    }
  } else {
    // 无选区，直接绘制
    if (!display_text.empty()) {
      freetype::draw_line(display_text, rect_x, rect_y);
    }
  }

  // ============================================================
  // 9. 恢复设置
  // ============================================================
  freetype::load_aligned(false);
  freetype::load_bold(false);
}

bool text_input_ui_system::event(SDL_Event *event, text_input &input) {
  bool r = false;
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
    if (input.active == false) {
      break;
    }
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
        if (input.cur_next.has_value()) {
          auto cur_min = std::min(input.cur, input.cur_next.value());
          auto cur_max = std::min(input.cur, input.cur_next.value());
          input.text.erase(cur_min, cur_max - cur_min);
          input.cur = cur_min;
          input.cur_next = std::nullopt;
        } else if (input.cur < input.text.length()) {
          input.text.erase(input.cur - 1, 1);
        } else {
          input.text.pop_back();
        }
        input.cur--;
      }
      break;
    }
    default: {
      SDL_Keycode key =
          SDL_GetKeyFromScancode(event->key.scancode, event->key.mod, true);
      // 过滤
      bool digit = (key >= SDLK_0 && key <= SDLK_9);
      bool letter = (key >= SDLK_A && key <= SDLK_Z);
      bool symbol = !digit && !letter;
      if (!input.type.test(text_input::symbol)) {
        if (symbol) {
          break;
        }
      }
      if (!input.type.test(text_input::digit)) {
        if (digit) {
          break;
        }
      }
      if (!input.type.test(text_input::letter)) {
        if (letter) {
          break;
        }
      }
      SDL_Event e;
      e.type = SDL_EVENT_TEXT_INPUT;
      // 如果是字母且 Caps Lock 开启，手动转换
      if ((event->key.mod & SDL_KMOD_CAPS) && (key >= 'a' && key <= 'z')) {
        key -= 32; // 小写转大写
      }
      char c[2] = {0, 0};
      c[0] = key;
      e.edit.text = c;
      text_input_ui_system::event(&e, input);
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
      } else {
        input.cur_next = std::nullopt;
        close(input);
        r = true;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    if (event->motion.state & SDL_BUTTON_LMASK) {
      if (input.active) {
        SDL_Point p = {
            static_cast<int>(window::mouse_pos.x),
            static_cast<int>(window::mouse_pos.y),
        };
        if (SDL_PointInRect(&p, &input.r)) {
          auto dx = p.x - input.r.x;
          freetype::load_size(input.font_size);
          auto text_w = freetype::load_w(input.text);
          if (dx >= text_w) {
            input.cur_next = input.text.length();
            break;
          }
          auto w = 0;
          for (int i = 0; i < input.text.length(); i++) {
            auto c = input.text[i];
            auto cw = freetype::load_w({c});
            if (w <= dx && dx <= cw + w) {
              input.cur_next = i;
              break;
            }
            w += cw;
          }
        }
      }
    }
  }
  }
  return r;
}