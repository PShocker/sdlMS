#include "text_input_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"

void text_input_ui_system::active(text_input &input) {
  SDL_Window *window = SDL_GetKeyboardFocus();
  /* Start-Stop */
  SDL_StartTextInput(window);
  SDL_SetTextInputArea(window, &input.r, 0);
}

void text_input_ui_system::render(text_input &input) {
  auto str = input.text + input.composition;
  if (input.active) {
    auto delta = window::dt_now % 2000;
    if (delta <= 1000) {
      str += u'|';
    }
  }
  freetype::load_aligned(true);
  freetype::load_bold(false);
  freetype::load_color(255, 255, 255, 255);
  freetype::load_size(13);
  freetype::draw_line(str, input.r.x, input.r.y);
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
    auto text = event->edit.text;
    input.text = input.text + freetype::load_u16str(text);
    input.text = input.text.substr(0, input.max_size);
    break;
  }
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_BACKSPACE: {
      if (input.active && !input.text.empty()) {
        input.text.pop_back();
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
  }
  return true;
}