#include "src/client/system/ui/minigame_setup_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <algorithm>
#include <string>

namespace sdlms {

namespace {

// 布局基于素材实测（Popup/背景 266x181 与 266x208）：
// 背景自带输入框槽位（omok y48-72 / match_cards y48-64）、
// 底部按钮条（omok y141-172 / match_cards y176-204）。
constexpr float kWinW = 266;
constexpr float kOmokH = 181;
constexpr float kMatchH = 208;

constexpr SDL_FRect kTextinput{68, 51, 130, 17};
constexpr SDL_FRect kIcon{198, 45, 43, 40};
constexpr SDL_FRect kPasswordBar{108, 74, 49, 7};
constexpr SDL_FRect kButtonOkOmok{59, 146, 39, 17};
constexpr SDL_FRect kButtonOkMatch{59, 182, 39, 17};
constexpr SDL_FRect kButtonCancelOmok{161, 146, 41, 17};
constexpr SDL_FRect kButtonCancelMatch{161, 182, 41, 17};

}  // namespace

SDL_FPoint MinigameSetupUiSystem::LoadWh() {
  return {kWinW, static_cast<float>(game_type_ == 1 ? kMatchH : kOmokH)};
}

bool MinigameSetupUiSystem::IsOpen() {
  return std::ranges::contains(System::render_systems_, &Render);
}

bool MinigameSetupUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect);
}

void MinigameSetupUiSystem::Open(int game_type) {
  game_type_ = game_type;
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it == System::render_systems_.end()) {
    return;
  }
  auto& camera = CameraGameInstance::camera_;
  auto [w, h] = LoadWh();
  pos_.x = static_cast<float>((camera.w - w) / 2);
  pos_.y = static_cast<float>((camera.h - h) / 2);

  use_password_ = false;
  password_input_ = TextInput{
      .max_size = 13,
      .text = u"",
      .composition = u"",
      .disable = false,
      .active = false,
      .r = SDL_Rect{
          static_cast<int>(pos_.x + kTextinput.x + 3),
          static_cast<int>(pos_.y + kTextinput.y),
          static_cast<int>(kTextinput.w - 6),
          static_cast<int>(kTextinput.h),
      },
      .pos = {0, 0},
      .font_color = {0, 0, 0, 255},
      .cur_color = {0, 0, 0, 255},
      .font_size = 11,
  };
  password_input_.type.set(TextInput::kIme);

  System::render_systems_.insert(it, Render);
  System::event_systems_.insert(System::event_systems_.begin(), Event);
}

void MinigameSetupUiSystem::RestoreKeyboard() {
  password_input_.Close();
  if (std::ranges::find(System::event_systems_, &KeyboardInputSystem::Event) ==
      System::event_systems_.end()) {
    System::event_systems_.push_back(KeyboardInputSystem::Event);
  }
}

void MinigameSetupUiSystem::Close() {
  RestoreKeyboard();
  drag_ = std::nullopt;
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void MinigameSetupUiSystem::RenderBackgrnd() {
  // 背景：五子棋桌用 backgrnd_omok，配对卡桌用 backgrnd_match_cards。
  auto* node = WzResource::ui_->find(game_type_ == 1
                                        ? u"Minigame.img/Popup/backgrnd_match_cards"
                                        : u"Minigame.img/Popup/backgrnd_omok");
  if (node == nullptr) {
    return;
  }
  auto texture = WzResource::LoadTexture(node);
  SDL_FRect pos_rect{
      pos_.x,
      pos_.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

void MinigameSetupUiSystem::RenderInputs() {
  // 公开/私房图标：公开=开锁（icon_public），私房=上锁（icon_private）。
  auto* icon_node = WzResource::ui_->find(
      use_password_ ? u"Minigame.img/Popup/icon_private"
                    : u"Minigame.img/Popup/icon_public");
  if (icon_node != nullptr) {
    auto icon = WzResource::LoadTexture(icon_node);
    SDL_FRect pos_rect{
        pos_.x + kIcon.x,
        pos_.y + kIcon.y,
        static_cast<float>(icon->w),
        static_cast<float>(icon->h),
    };
    SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
  }
  // 密码开关轨道：enabled 亮条 = 密码房。
  auto* bar_node = WzResource::ui_->find(
      use_password_ ? u"Minigame.img/Popup/password_enabled"
                    : u"Minigame.img/Popup/password_disabled");
  if (bar_node != nullptr) {
    auto bar = WzResource::LoadTexture(bar_node);
    SDL_FRect pos_rect{
        pos_.x + kPasswordBar.x,
        pos_.y + kPasswordBar.y,
        static_cast<float>(bar->w),
        static_cast<float>(bar->h),
    };
    SDL_RenderTexture(Window::renderer_, bar, nullptr, &pos_rect);
  }
  // 密码输入框：私房可输入（enabled），公开置灰（disabled）。
  auto* input_node = WzResource::ui_->find(
      use_password_ ? u"Minigame.img/Popup/textinput_enabled"
                    : u"Minigame.img/Popup/textinput_disabled");
  if (input_node != nullptr) {
    auto input = WzResource::LoadTexture(input_node);
    SDL_FRect pos_rect{
        pos_.x + kTextinput.x,
        pos_.y + kTextinput.y,
        static_cast<float>(input->w),
        static_cast<float>(input->h),
    };
    SDL_RenderTexture(Window::renderer_, input, nullptr, &pos_rect);
  }
  if (use_password_) {
    password_input_.Render();
  }
}

UIComponents& MinigameSetupUiSystem::GetComponent() {
  components_.clear();
  const bool match = game_type_ == 1;
  auto add_button = [&](const char16_t* path, SDL_FRect rect,
                        void (*callback)()) {
    Button b{.node = path != nullptr ? WzResource::ui_->find(path) : nullptr,
             .rect = rect,
             .enable = true,
             .focused = CursorGameInstance::cursor_ui_ == Render};
    b.rect.x += pos_.x;
    b.rect.y += pos_.y;
    b.callback = callback;
    components_.push_back(b);
  };
  // 公开/私房切换：锁图标 + 开关轨道都可点（无贴图纯热区）。
  add_button(nullptr, kIcon, EventButtonToggle);
  add_button(nullptr, kPasswordBar, EventButtonToggle);
  add_button(u"Minigame.img/btEnter",
             match ? kButtonOkMatch : kButtonOkOmok, EventButtonOk);
  add_button(u"Minigame.img/btLeave",
             match ? kButtonCancelMatch : kButtonCancelOmok,
             EventButtonCancel);
  return components_;
}

bool MinigameSetupUiSystem::Render() {
  RenderBackgrnd();
  RenderInputs();
  RenderUI();
  return true;
}

void MinigameSetupUiSystem::EventButtonToggle() {
  use_password_ = !use_password_;
  if (use_password_) {
    password_input_.Active();
    std::erase(System::event_systems_, KeyboardInputSystem::Event);
  } else {
    password_input_.Close();
    if (std::ranges::find(System::event_systems_,
                          &KeyboardInputSystem::Event) ==
        System::event_systems_.end()) {
      System::event_systems_.push_back(KeyboardInputSystem::Event);
    }
  }
}

void MinigameSetupUiSystem::EventButtonOk() {
  // 确认：step0 进房/建房。chat=密码（空=公开房）；无房时自己成为房主。
  auto password = use_password_ ? password_input_.text : std::u16string{};
  fbs::ClientCharacterMinigameT m;
  m.step = 0;
  m.x = static_cast<uint8_t>(game_type_);
  m.chat = {password.begin(), password.end()};
  ClientRequest::SendToHost(m);
  Close();
}

void MinigameSetupUiSystem::EventButtonCancel() {
  Close();
}

bool MinigameSetupUiSystem::EventUI(SDL_Event* event) {
  return DispatchButtonClick(GetComponent(), true);
}

bool MinigameSetupUiSystem::Event(SDL_Event* event) {
  // modal 弹窗：所有事件在此处理，不透传给下层界面。
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT && CursorIn()) {
        // 顶部标题区拖动窗口。
        if (event->button.y - pos_.y < 20) {
          drag_ = {pos_.x - event->button.x, pos_.y - event->button.y};
        }
        password_input_.Event(event);
        if (password_input_.active &&
            std::ranges::find(System::event_systems_,
                              &KeyboardInputSystem::Event) ==
                System::event_systems_.end()) {
          System::event_systems_.push_back(KeyboardInputSystem::Event);
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorIn()) {
          EventUI(event);
        }
        drag_ = std::nullopt;
        if (!password_input_.active &&
            std::ranges::find(System::event_systems_,
                              &KeyboardInputSystem::Event) ==
                System::event_systems_.end()) {
          System::event_systems_.push_back(KeyboardInputSystem::Event);
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      if (drag_.has_value()) {
        auto& camera = CameraGameInstance::camera_;
        auto [w, h] = LoadWh();
        pos_ = {event->motion.x + drag_->x, event->motion.y + drag_->y};
        pos_.x = std::clamp(pos_.x, 0.0f, camera.w - w);
        pos_.y = std::clamp(pos_.y, 0.0f, camera.h - h);
        password_input_.r = SDL_Rect{
            static_cast<int>(pos_.x + kTextinput.x + 3),
            static_cast<int>(pos_.y + kTextinput.y),
            static_cast<int>(kTextinput.w - 6),
            static_cast<int>(kTextinput.h),
        };
      }
      password_input_.Event(event);
      break;
    }
    case SDL_EVENT_TEXT_INPUT:
    case SDL_EVENT_TEXT_EDITING: {
      password_input_.Event(event);
      break;
    }
    case SDL_EVENT_KEY_DOWN: {
      auto scan_code = event->key.scancode;
      if (scan_code == SDL_SCANCODE_ESCAPE) {
        Close();
      } else if (scan_code == SDL_SCANCODE_RETURN) {
        EventButtonOk();
      } else if (password_input_.active) {
        password_input_.Event(event);
        if (!password_input_.active &&
            std::ranges::find(System::event_systems_,
                              &KeyboardInputSystem::Event) ==
                System::event_systems_.end()) {
          System::event_systems_.push_back(KeyboardInputSystem::Event);
        }
      }
      break;
    }
    default: {
      break;
    }
  }
  return false;
}

}  // namespace sdlms
