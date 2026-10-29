#include "src/client/system/ui/popup_tip_ui_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/popup_tip_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <string>

namespace sdlms {

void PopupTipUiSystem::RenderBackgrnd(GamePopupTip& t) {
  SDL_Texture* te;
  SDL_Texture* i = nullptr;
  SDL_FPoint icon_pos;
  switch (t.type) {
    case PopupTipEnums::kTrade: {
      static auto texture = WzResource::LoadTexture(
          WzResource::ui_->find(u"StatusBar.img/submenu/backgrnd/1"));
      break;
    }
    case PopupTipEnums::kParty: {
      static auto texture = WzResource::LoadTexture(
          WzResource::ui_->find(u"StatusBar.img/submenu/backgrnd/1"));
      break;
    }
    case PopupTipEnums::kQuest: {
      static auto texture = WzResource::LoadTexture(
          WzResource::ui_->find(u"FadeYesNo.img/FadeYesNo/backgrnd3"));
      static auto icon = WzResource::LoadTexture(
          WzResource::ui_->find(u"FadeYesNo.img/FadeYesNo/icon6"));
      te = texture;
      i = icon;
      icon_pos = {-10, -12};
      break;
    }
  }
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto x = pos_.x + (screen_w - 808) / 2;
  auto y = pos_.y + (screen_h - 73);

  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(te->w),
      static_cast<float>(te->h),
  };
  SDL_SetTextureAlphaMod(te, t.alpha);
  SDL_RenderTexture(Window::renderer_, te, nullptr, &pos_rect);

  pos_rect.x -= icon_pos.x;
  pos_rect.y -= icon_pos.y;
  pos_rect.w = i->w;
  pos_rect.h = i->h;
  SDL_SetTextureAlphaMod(i, t.alpha);
  SDL_RenderTexture(Window::renderer_, i, nullptr, &pos_rect);
}

void PopupTipUiSystem::RenderText(GamePopupTip& t) {
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto x = pos_.x + (screen_w - 808) / 2;
  auto y = pos_.y + (screen_h - 73);

  switch (t.type) {
    case PopupTipEnums::kQuest: {
      auto quest_id = std::any_cast<std::u16string>(t.data);
      auto node = QuestGameInstance::LoadQuestNode(quest_id);
      node = node->find(u"QuestInfo/name");
      auto name = static_cast<wz::Property<std::u16string>*>(node)->get();
      if (name.size() > 17) {
        name = name.substr(0, 17) + u"...";
      }
      Freetype::LoadSize(12);
      Freetype::LoadColor(255, 255, 255, t.alpha);
      Freetype::LoadAligned(true);
      Freetype::DrawLine(name, x + 28, y + 3);

      name = u"Quest Complete!";
      Freetype::DrawLine(name, x + 28, y + 16);
      break;
    }
    case PopupTipEnums::kTrade:
    case PopupTipEnums::kParty: {
      break;
    }
  }
}

void PopupTipUiSystem::RenderButton(GamePopupTip& t) {
  // 构建按钮列表
  std::vector<wz::Node*> nodes = {};
  std::vector<SDL_FRect> rects = {};
  switch (t.type) {
    case PopupTipEnums::kTrade: {
      nodes = {};
      rects = {};
      break;
    }
    case PopupTipEnums::kParty: {
      nodes = {};
      rects = {};
      break;
    }
    case PopupTipEnums::kQuest: {
      break;
    }
  }
  // 渲染所有按钮
  bool mouse_down = Window::mouse_state_ & SDL_BUTTON_LMASK;
  bool cursor_on_ui = CursorGameInstance::cursor_ui_ == Render;
  bool modal_blocked = CursorGameInstance::modal_overlay_;

  for (size_t i = 0; i < nodes.size(); ++i) {
    SDL_FRect rect = {
        rects[i].x + (int)pos_.x,
        rects[i].y + (int)pos_.y,
        rects[i].w,
        rects[i].h,
    };

    std::u16string state = u"normal";
    if (cursor_on_ui && !modal_blocked &&
        SDL_PointInRectFloat(&Window::mouse_pos_, &rect)) {
      state = mouse_down ? u"pressed" : u"mouseOver";
    }
    auto texture = WzResource::LoadTexture(nodes[i]->find((state + u"/0")));
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &rect);
  }
}

void PopupTipUiSystem::Destroy() {
  auto& data = PopupTipGameInstance::data_;
  const auto now = Window::dt_now_;
  std::erase_if(data, [now](const GamePopupTip& t) {
    return t.destroy < now && t.alpha == 0.0f;
  });
}

bool PopupTipUiSystem::Render() {
  auto& data = PopupTipGameInstance::data_;
  const auto now = Window::dt_now_;
  const auto dt = Window::delta_time_;

  for (auto& t : data) {
    const float dir = (t.destroy < now) ? -1.0f : 1.0f;
    t.alpha = std::clamp(t.alpha + dir * dt, 0.0f, 255.0f);
    RenderBackgrnd(t);
    RenderText(t);
    RenderButton(t);
  }

  Destroy();

  return true;
}

void PopupTipUiSystem::EventButtonParty(GamePopupTip& t) {
  fbs::ClientCharacterPartyT ccp;
  ccp.step = 1;
  ClientRequest::SendToHost(ccp);
  return;
}

void PopupTipUiSystem::EventButtonTrade(GamePopupTip& t) {}

void PopupTipUiSystem::EventButtonOk(GamePopupTip& t) {
  switch (t.type) {
    case PopupTipEnums::kTrade: {
      EventButtonTrade(t);
      break;
    }
    case PopupTipEnums::kParty: {
      EventButtonParty(t);
      break;
    }
    case PopupTipEnums::kQuest: {
      break;
    }
  }
}

void PopupTipUiSystem::EventButtonCancel(GamePopupTip& t) {
  auto& data = PopupTipGameInstance::data_;
  std::erase_if(data, [t](const auto& tip) { return &t == &tip; });
}

bool PopupTipUiSystem::EventUI(SDL_Event* event) {
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);

  std::vector<SDL_FRect> buttons_rect;
  std::vector<void (*)(GamePopupTip&)> fns = {
      EventButtonOk,
      EventButtonCancel,
  };

  for (int i = 0; i < PopupTipGameInstance::data_.size(); i++) {
    auto& t = PopupTipGameInstance::data_[i];
    switch (t.type) {
      case PopupTipEnums::kTrade: {
        break;
      }
      case PopupTipEnums::kParty: {
        break;
      }
      case PopupTipEnums::kQuest: {
        break;
      }
    }
    for (size_t i = 0; i < buttons_rect.size(); ++i) {
      auto pos_rect = buttons_rect[i];
      pos_rect.x += base_x;
      pos_rect.y += base_y;
      if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
        fns[i](t);
        return true;
      }
    }
  }
  return false;
}

bool PopupTipUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = EventUI(event);
        }
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
