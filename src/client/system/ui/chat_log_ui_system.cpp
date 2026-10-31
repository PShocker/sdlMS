#include "src/client/system/ui/chat_log_ui_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/chat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/component/scroll.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <algorithm>
#include <flat_map>
#include <format>
#include <vector>

namespace sdlms {

static std::vector<std::u16string> WrapChatLine(const std::u16string& str) {
  constexpr float kMaxWidth = 550.0f;
  std::vector<std::u16string> lines;
  std::u16string line;
  float width = 0;
  for (auto c : str) {
    if (c == u'\r') {
      continue;
    }
    if (c == u'\n') {
      lines.push_back(line);
      line.clear();
      width = 0;
      continue;
    }
    const float cw = Freetype::LoadW(std::u16string(1, c));
    if (!line.empty() && width + cw > kMaxWidth) {
      lines.push_back(line);
      line.clear();
      width = 0;
    }
    line.push_back(c);
    width += cw;
  }
  if (!line.empty() || lines.empty()) {
    lines.push_back(line);
  } 
  return lines;
}

std::vector<std::u16string> ChatLogUiSystem::LoadChatLines() {
  Freetype::LoadSize(chat_font_size_);
  std::vector<std::u16string> lines;
  for (const auto& chat : ChatGameInstance::chats_) {
    auto str = chat.owner + u":" + chat.text;
    auto wrapped = WrapChatLine(str);
    lines.insert(lines.end(), wrapped.begin(), wrapped.end());
  }
  return lines;
}

void ChatLogUiSystem::RenderChats() {
  const auto lines = LoadChatLines();
  constexpr int kVisibleLines = 5;
  const int total = static_cast<int>(lines.size());
  const int max_page = std::max(0, total - kVisibleLines);
  page_ = std::clamp(page_, 0, max_page);

  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);

  const int end = total - page_;
  const int start = std::max(0, end - kVisibleLines);
  Freetype::LoadAligned(true);
  Freetype::LoadSize(chat_font_size_);
  Freetype::LoadColor(255, 255, 255, 255);
  constexpr float kRowHeight = 15.0f;
  for (int i = start; i < end; i++) {
    const float y = base_y - 75 + 2 + (i - start) * kRowHeight;
    if (i == selected_line_) {
      SDL_SetRenderDrawBlendMode(Window::renderer_, SDL_BLENDMODE_BLEND);
      SDL_SetRenderDrawColor(Window::renderer_, 70, 110, 180, 150);
      SDL_FRect select_rect{
          static_cast<float>(base_x + 8),
          y,
          550.0f,
          kRowHeight,
      };
      SDL_RenderFillRect(Window::renderer_, &select_rect);
    }
    Freetype::DrawStr(lines[i], base_x + 8, y, 550, 1.1);
  }
}

void ChatLogUiSystem::RenderBackgrnd() {
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  SDL_FRect pos_rect;
  static auto chatbackgrnd1 = WzResource::LoadTexture(
      WzResource::ui_->find(u"StatusBar.img/chat/canvas:chatbackgrnd1"));
  pos_rect.x = base_x + 6;
  pos_rect.y = base_y - 75;
  pos_rect.w = chatbackgrnd1->w;
  pos_rect.h = 80;

  SDL_RenderTexture(Window::renderer_, chatbackgrnd1, nullptr, &pos_rect);
}

bool ChatLogUiSystem::Render() {
  RenderBackgrnd();
  RenderChats();
  RenderUI();
  return true;
}

void ChatLogUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void ChatLogUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void ChatLogUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

bool ChatLogUiSystem::CursorIn() {
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  SDL_FRect rect{
      static_cast<float>(base_x + 6),
      static_cast<float>(base_y - 75),
      580.0f,
      80.0f,
  };
  return SDL_PointInRectFloat(&Window::mouse_pos_, &rect);
}

UIComponents& ChatLogUiSystem::GetComponent() {
  components_.clear();
  const uint32_t length = 80;
  const uint32_t size = 5;
  const int total = static_cast<int>(LoadChatLines().size());
  const int max_page = std::max(0, total - static_cast<int>(size));
  page_ = std::clamp(page_, 0, max_page);
  const uint32_t first = static_cast<uint32_t>(max_page - page_);
  auto cursor_in = CursorGameInstance::cursor_ui_;
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  bool top = cursor_in == Render && !CursorGameInstance::modal_overlay_;
  Scroll scroll(first, total, length, size);
  scroll.pos = {base_x + 570, base_y - 75};
  scroll.top = top;
  components_.push_back(scroll);
  return components_;
}

bool ChatLogUiSystem::EventUI(SDL_Event* event) {
  const int total = static_cast<int>(LoadChatLines().size());
  const int max_page = std::max(0, total - 5);
  for (auto& component : GetComponent()) {
    if (auto* scroll = std::get_if<Scroll>(&component)) {
      auto first = scroll->Click();
      page_ = max_page - static_cast<int>(first);
    }
  }
  return false;
}

bool ChatLogUiSystem::Event(SDL_Event* event) {
  static int vscroll_drag = -1;
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      if (!StatusbarUiSystem::chat_.active &&
          (event->key.mod & SDL_KMOD_CTRL) &&
          event->key.scancode == SDL_SCANCODE_C) {
        auto lines = LoadChatLines();
        std::u16string copied;
        if (selected_line_ >= 0 &&
            selected_line_ < static_cast<int>(lines.size())) {
          copied = lines[selected_line_];
        } else {
          const int total = static_cast<int>(lines.size());
          const int max_page = std::max(0, total - 5);
          const int end = total - std::clamp(page_, 0, max_page);
          const int start = std::max(0, end - 5);
          for (int i = start; i < end; i++) {
            if (!copied.empty()) {
              copied.push_back(u'\n');
            }
            copied += lines[i];
          }
        }
        if (!copied.empty()) {
          auto utf8 = TextGameInstance::LoadU8str(copied);
          SDL_SetClipboardText(utf8.c_str());
        }
        r = false;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          auto screen_w = CameraGameInstance::camera_.w;
          auto screen_h = CameraGameInstance::camera_.h;
          auto base_x = (screen_w - 808) / 2;
          auto base_y = (screen_h - 73);
          auto lines = LoadChatLines();
          const int total = static_cast<int>(lines.size());
          const int max_page = std::max(0, total - 5);
          const int end = total - std::clamp(page_, 0, max_page);
          const int start = std::max(0, end - 5);
          if (event->button.x >= base_x + 8 &&
              event->button.x < base_x + 565) {
            auto local = static_cast<int>(
                (event->button.y - (base_y - 75 + 2)) / 15.0f);
            if (local >= 0 && local < 5 && start + local < end) {
              selected_line_ = start + local;
            }
          }
          DispatchScrollDragStart(GetComponent(), vscroll_drag,
                                  {event->button.x, event->button.y});
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = !EventUI(event);
        }
        vscroll_drag = -1;
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      if (vscroll_drag >= 0) {
        const int total = static_cast<int>(LoadChatLines().size());
        const int max_page = std::max(0, total - 5);
        int first = max_page - std::clamp(page_, 0, max_page);
        uint32_t val = static_cast<uint32_t>(std::max(first, 0));
        if (DispatchScrollDragMove(GetComponent(), vscroll_drag,
                                   {event->motion.x, event->motion.y}, val)) {
          page_ = max_page - static_cast<int>(val);
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_WHEEL: {
      if (CursorGameInstance::cursor_ui_ != Render ||
          !ChatLogUiSystem::CursorIn()) {
        break;
      }
      const int total = static_cast<int>(LoadChatLines().size());
      const int max_page = std::max(0, total - 5);
      if (event->wheel.integer_y > 0) {
        page_ = std::min(max_page, page_ + 1);
      } else if (event->wheel.integer_y < 0) {
        page_ = std::max(0, page_ - 1);
      }
      break;
    }
    default: {
      break;
    }
  }

  return r;
}

}  // namespace sdlms
