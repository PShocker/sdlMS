#include "src/client/system/ui/text_input_ui_system.h"

#include "src/client/game_instance/text_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_scancode.h>

#include <algorithm>
#include <optional>

namespace sdlms {

void TextInputUiSystem::Close(TextInput& input) {
  input.active = false;
  if (input.type.test(TextInput::kIme)) {
    SDL_StopTextInput(SDL_GetKeyboardFocus());
  }
}

void TextInputUiSystem::Active(TextInput& input) {
  input.active = true;
  if (input.type.test(TextInput::kIme)) {
    SDL_Window* sdl_window = SDL_GetKeyboardFocus();
    SDL_StartTextInput(sdl_window);
    SDL_SetTextInputArea(sdl_window, &input.r, 0);
  }
}

void TextInputUiSystem::Render(TextInput& input, int x, int y) {
  // ============================================================
  // 1. 基础设置和坐标计算
  // ============================================================
  Freetype::LoadSize(input.font_size);

  const float line_height = Freetype::LoadLh();
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
  // 4. 文本裁剪（保持光标/选区可见）
  // ============================================================
  std::u16string display_text;
  size_t display_offset = 0;

  if (full_text.empty()) {
    display_text = full_text;
  } else {
    const size_t focus_pos = has_selection ? sel_start : input.cur;
    float full_width = Freetype::LoadW(full_text);

    if (full_width > max_width) {
      // 以 focus_pos 为中心，向两侧扩展
      size_t left = focus_pos;
      size_t right = focus_pos;

      // 尝试向左扩展
      while (left > 0) {
        size_t new_left = left - 1;
        auto test_text = full_text.substr(new_left, right - new_left);
        if (Freetype::LoadW(test_text) > max_width)
          break;
        left = new_left;
      }

      // 尝试向右扩展
      while (right < full_text.length()) {
        size_t new_right = right + 1;
        auto test_text = full_text.substr(left, new_right - left);
        if (Freetype::LoadW(test_text) > max_width)
          break;
        right = new_right;
      }

      // 如果仍然超出，从右侧逐步裁剪
      float current_width =
          Freetype::LoadW(full_text.substr(left, right - left));
      while (current_width > max_width && right > left + 1) {
        right--;
        current_width = Freetype::LoadW(full_text.substr(left, right - left));
      }

      // 如果还是超出，从左侧裁剪（极端情况）
      while (current_width > max_width && left < right - 1) {
        left++;
        current_width = Freetype::LoadW(full_text.substr(left, right - left));
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
  // 5. 光标渲染（仅在激活且无选区时显示闪烁光标）
  // ============================================================
  if (input.active && !has_selection) {
    const int blink_period = 500;
    if ((Window::dt_now_ % (blink_period * 2)) < blink_period) {
      // 计算光标在显示文本中的相对位置
      const int cursor_display_pos =
          static_cast<int>(input.cur - display_offset);

      // 检查光标是否在显示区域内
      if (cursor_display_pos >= 0 &&
          cursor_display_pos <= static_cast<int>(display_text.length())) {
        // 计算光标前文本在显示区域中的宽度
        const auto text_before_cursor_display =
            display_text.substr(0, cursor_display_pos);
        const float cursor_x = Freetype::LoadW(text_before_cursor_display);

        // 确保光标不会超出显示区域右边界
        if (cursor_x <= max_width) {
          SDL_FRect cursor_rect = {rect_x + cursor_x, rect_y + 2.0f, 1.0f,
                                   line_height};

          SDL_SetRenderDrawColor(Window::renderer_, input.cur_color.r,
                                 input.cur_color.g, input.cur_color.b,
                                 input.cur_color.a);
          SDL_RenderFillRect(Window::renderer_, &cursor_rect);
        }
      }
      // 可选：当光标在显示区域外时，在边缘显示指示符
      else if (cursor_display_pos < 0) {
        // 光标在左侧被裁剪，在左边缘显示小标记
        SDL_FRect cursor_rect = {rect_x, rect_y + 2.0f, 2.0f, line_height};
        SDL_SetRenderDrawColor(Window::renderer_, input.cur_color.r,
                               input.cur_color.g, input.cur_color.b,
                               input.cur_color.a);
        SDL_RenderFillRect(Window::renderer_, &cursor_rect);
      } else if (cursor_display_pos > static_cast<int>(display_text.length())) {
        // 光标在右侧被裁剪，在右边缘显示小标记
        SDL_FRect cursor_rect = {rect_x + max_width - 2.0f, rect_y + 2.0f, 2.0f,
                                 line_height};
        SDL_SetRenderDrawColor(Window::renderer_, input.cur_color.r,
                               input.cur_color.g, input.cur_color.b,
                               input.cur_color.a);
        SDL_RenderFillRect(Window::renderer_, &cursor_rect);
      }
    }
  }

  // ============================================================
  // 6. 文本渲染配置
  // ============================================================
  Freetype::LoadAligned(true);
  Freetype::LoadBold(false);
  Freetype::LoadColor(input.font_color.r, input.font_color.g,
                      input.font_color.b, input.font_color.a);

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

      const float sel_x = Freetype::LoadW(text_before_sel);
      const float sel_width = Freetype::LoadW(text_selected);

      SDL_FRect sel_rect = {rect_x + sel_x, rect_y + 2.0f, sel_width,
                            line_height};

      SDL_SetRenderDrawColor(Window::renderer_, 50, 102, 208, 255);
      SDL_RenderFillRect(Window::renderer_, &sel_rect);
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
      Freetype::DrawLine(text_before, draw_x, rect_y);
      draw_x += Freetype::LoadW(text_before);
    }

    // 8.2 绘制选区文本（白色高亮）
    if (clip_start < clip_end) {
      const auto text_selected =
          display_text.substr(clip_start, clip_end - clip_start);
      const auto old_color = input.font_color;
      Freetype::LoadColor(255, 255, 255, 255);
      Freetype::DrawLine(text_selected, draw_x, rect_y);
      draw_x += Freetype::LoadW(text_selected);
      Freetype::LoadColor(old_color.r, old_color.g, old_color.b, old_color.a);
    }

    // 8.3 绘制选区后的文本
    if (clip_end < static_cast<int>(display_text.length())) {
      const auto text_after = display_text.substr(clip_end);
      Freetype::DrawLine(text_after, draw_x, rect_y);
    }
  } else {
    // 无选区，直接绘制
    if (!display_text.empty()) {
      Freetype::DrawLine(display_text, rect_x, rect_y);
    }
  }

  // ============================================================
  // 9. 恢复设置
  // ============================================================
  Freetype::LoadAligned(false);
  Freetype::LoadBold(false);
}

bool TextInputUiSystem::Event(SDL_Event* Event, TextInput& input) {
  bool r = false;
  switch (Event->type) {
    case SDL_EVENT_TEXT_EDITING: {
      auto text = Event->edit.text;
      if (text[0] != '\0') {
        input.composition = TextGameInstance::LoadU16str(text);
      } else {
        input.composition = u"";
      }
      break;
    }
    case SDL_EVENT_TEXT_INPUT: {
      if (input.text.length() >= input.max_size) {
        break;
      }
      auto text = Event->edit.text;
      auto cur0 = input.text.length();
      auto str1 = input.text.substr(0, input.cur);
      auto str2 = TextGameInstance::LoadU16str(text);
      auto str3 = input.text.substr(input.cur);
      if (input.cur_next.has_value()) {
        auto min_index = std::min(input.cur, input.cur_next.value());
        auto max_index = std::max(input.cur, input.cur_next.value());
        input.text.replace(min_index, max_index - min_index, str2);
        input.cur = min_index + str2.size();
        input.cur_next = std::nullopt;
        input.text = input.text.substr(0, input.max_size);
      } else {
        input.text = str1 + str2 + str3;
        input.text = input.text.substr(0, input.max_size);
        auto cur1 = input.text.length();
        input.cur += cur1 - cur0;
      }
      break;
    }
    case SDL_EVENT_KEY_DOWN: {
      if (input.active == false) {
        break;
      }
      auto scan_code = Event->key.scancode;
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
              auto cur_max = std::max(input.cur, input.cur_next.value());
              input.text.erase(cur_min, cur_max - cur_min);
              input.cur = cur_min + 1;
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
              SDL_GetKeyFromScancode(Event->key.scancode, Event->key.mod, true);
          // 过滤
          bool digit = (key >= SDLK_0 && key <= SDLK_9);
          bool letter = (key >= SDLK_A && key <= SDLK_Z);
          bool symbol = !digit && !letter;
          if (!input.type.test(TextInput::kSymbol)) {
            if (symbol) {
              break;
            }
          }
          if (!input.type.test(TextInput::kDigit)) {
            if (digit) {
              break;
            }
          }
          if (!input.type.test(TextInput::kLetter)) {
            if (letter) {
              break;
            }
          }
          SDL_Event e;
          e.type = SDL_EVENT_TEXT_INPUT;
          // 如果是字母且 Caps Lock 开启，手动转换
          if ((Event->key.mod & SDL_KMOD_CAPS) && (key >= 'a' && key <= 'z')) {
            key -= 32;  // 小写转大写
          }
          char c[2] = {0, 0};
          c[0] = key;
          e.edit.text = c;
          TextInputUiSystem::Event(&e, input);
          break;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (Event->button.button == SDL_BUTTON_LEFT) {
        SDL_Point p = {
            static_cast<int>(Window::mouse_pos_.x),
            static_cast<int>(Window::mouse_pos_.y),
        };
        input.cur_next = std::nullopt;
        if (SDL_PointInRect(&p, &input.r)) {
          Active(input);
          auto dx = p.x - input.r.x;
          Freetype::LoadSize(input.font_size);
          auto text_w = Freetype::LoadW(input.text);
          if (dx >= text_w) {
            input.cur = input.text.length();
            break;
          }
          auto w = 0;
          for (int i = 0; i < input.text.length(); i++) {
            auto c = input.text[i];
            auto cw = Freetype::LoadW({c});
            if (w <= dx && dx <= cw + w) {
              input.cur = i;
              break;
            }
            w += cw;
          }
        } else {
          Close(input);
          r = true;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      if (Event->motion.state & SDL_BUTTON_LMASK) {
        if (input.active) {
          SDL_Point p = {
              static_cast<int>(Window::mouse_pos_.x),
              static_cast<int>(Window::mouse_pos_.y),
          };
          if (SDL_PointInRect(&p, &input.r)) {
            auto dx = p.x - input.r.x;
            Freetype::LoadSize(input.font_size);
            auto text_w = Freetype::LoadW(input.text);
            if (dx >= text_w) {
              input.cur_next = input.text.length();
              break;
            }
            auto w = 0;
            for (int i = 0; i < input.text.length(); i++) {
              auto c = input.text[i];
              auto cw = Freetype::LoadW({c});
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

}  // namespace sdlms
