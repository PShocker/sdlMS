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

namespace {

// FadeYesNo 确认框（173x44）位于状态栏上方居中，坐标沿用 095 CUIFadeYesNo。
// 交易邀请与五子棋请求（悔棋/求和）共用。
SDL_FPoint FadeYesBoxPos() {
  auto& camera = CameraGameInstance::camera_;
  return {static_cast<float>((camera.w - 173) / 2),
          static_cast<float>(camera.h - 73 - 33 - 44)};
}

SDL_FRect FadeYesOkRect(SDL_FPoint p) {
  return {p.x + 136, p.y + 7, 12, 12};
}

SDL_FRect FadeYesCancelRect(SDL_FPoint p) {
  return {p.x + 136, p.y + 20, 12, 12};
}

}  // namespace


void PopupTipUiSystem::RenderBackgrnd(GamePopupTip& t) {
  SDL_Texture* te = nullptr;

  SDL_Texture* i = nullptr;

  SDL_FPoint icon_pos{0, 0};

  switch (t.type) {
    case PopupTipEnums::kTrade:
    case PopupTipEnums::kMinigame:
    case PopupTipEnums::kParty: {
      auto p = FadeYesBoxPos();
      static auto bg = WzResource::LoadTexture(
          WzResource::ui_->find(u"FadeYesNo.img/FadeYesNo/backgrnd3"));
      SDL_FRect r{p.x, p.y, static_cast<float>(bg->w),
                  static_cast<float>(bg->h)};
      SDL_SetTextureAlphaMod(bg, static_cast<uint8_t>(t.alpha));
      SDL_RenderTexture(Window::renderer_, bg, nullptr, &r);
      return;
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


  if (i != nullptr) {
    pos_rect.x -= icon_pos.x;

    pos_rect.y -= icon_pos.y;

    pos_rect.w = i->w;

    pos_rect.h = i->h;

    SDL_SetTextureAlphaMod(i, t.alpha);

    SDL_RenderTexture(Window::renderer_, i, nullptr, &pos_rect);
  }
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


      name = WzResource::LoadMSString(u"questComplete", u"Quest Complete!");

      Freetype::DrawLine(name, x + 28, y + 16);

      break;
    }

    case PopupTipEnums::kTrade: {
      auto p = FadeYesBoxPos();
      auto* player = std::any_cast<fbs::PlayerT>(&t.data);
      if (player != nullptr && player->character != nullptr) {
        std::u16string name{player->character->name.begin(),
                            player->character->name.end()};
        Freetype::LoadSize(11);
        Freetype::LoadColor(255, 255, 255, t.alpha);
        Freetype::LoadAligned(true);
        Freetype::DrawLine(name + u" wants to trade?", p.x + 14, p.y + 16);
      }
      break;
    }

    // 五子棋请求（095 OnRetreatRequest / OnTieRequest）。
    case PopupTipEnums::kMinigame: {
      auto p = FadeYesBoxPos();
      auto* req = std::any_cast<GameOmokRequestTip>(&t.data);
      if (req != nullptr && req->player.character != nullptr) {
        std::u16string name{req->player.character->name.begin(),
                            req->player.character->name.end()};
        auto text = name + (req->kind == 6 ? u" asks for an undo."
                                           : u" asks for a draw.");
        Freetype::LoadSize(11);
        Freetype::LoadColor(255, 255, 255, t.alpha);
        Freetype::LoadAligned(true);
        Freetype::DrawLine(text, p.x + 14, p.y + 16);
      }
      break;
    }
    // 组队邀请（095 CUIFadeYesNo CreatePartyInvite）。
    case PopupTipEnums::kParty: {
      auto p = FadeYesBoxPos();
      auto* player = std::any_cast<fbs::PlayerT>(&t.data);
      if (player != nullptr && player->character != nullptr) {
        std::u16string name{player->character->name.begin(),
                            player->character->name.end()};
        Freetype::LoadSize(11);
        Freetype::LoadColor(255, 255, 255, t.alpha);
        Freetype::LoadAligned(true);
        Freetype::DrawLine(name + u" invites you to the party.", p.x + 14,
                           p.y + 16);
      }
      break;
    }
  }
}


void PopupTipUiSystem::RenderButton(GamePopupTip& t) {
  // 构建按钮列表

  std::vector<wz::Node*> nodes = {};

  std::vector<SDL_FRect> rects = {};

  switch (t.type) {
    case PopupTipEnums::kTrade:
    case PopupTipEnums::kMinigame:
    case PopupTipEnums::kParty: {
      auto p = FadeYesBoxPos();
      auto draw_btn = [&](const char16_t* base, SDL_FRect r) {
        bool hover = SDL_PointInRectFloat(&Window::mouse_pos_, &r);
        std::u16string path =
            std::u16string(base) + (hover ? u"/mouseOver/0" : u"/normal/0");
        auto* node = WzResource::ui_->find(path);
        if (node == nullptr) {
          return;
        }
        auto* tex = WzResource::LoadTexture(node);
        SDL_SetTextureAlphaMod(tex, static_cast<uint8_t>(t.alpha));
        SDL_FRect rr{r.x, r.y, static_cast<float>(tex->w),
                     static_cast<float>(tex->h)};
        SDL_RenderTexture(Window::renderer_, tex, nullptr, &rr);
      };
      draw_btn(u"FadeYesNo.img/FadeYesNo/BtOK", FadeYesOkRect(p));
      draw_btn(u"FadeYesNo.img/FadeYesNo/BtCancel", FadeYesCancelRect(p));
      return;
    }

    case PopupTipEnums::kQuest: {
      break;
    }
  }

  // 渲染所有按�?
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


void PopupTipUiSystem::EventButtonParty(GamePopupTip& t, uint8_t accept) {
  // 接受/拒绝组队邀请（095 CP 145 step 3；拒绝本地忽略，这里回执邀请人）。
  auto* player = std::any_cast<fbs::PlayerT>(&t.data);
  if (player != nullptr && player->client_id != 0) {
    fbs::ClientCharacterPartyT cp;
    cp.step = accept ? 1 : 2;
    cp.to_id = player->client_id;
    ClientRequest::SendToHost(cp);
  }
  std::erase_if(PopupTipGameInstance::data_,
                [&t](const auto& tip) { return &t == &tip; });
}


void PopupTipUiSystem::EventButtonTrade(GamePopupTip& t) {
  auto* player = std::any_cast<fbs::PlayerT>(&t.data);
  if (player != nullptr && player->client_id != 0) {
    fbs::ClientCharacterTradeT ct;
    ct.to_id = player->client_id;
    ct.step = 1;  // 接受交易
    ClientRequest::SendToHost(ct);
  }
  std::erase_if(PopupTipGameInstance::data_,
                [&t](const auto& tip) { return &t == &tip; });
}


// 五子棋请求答复：OK 同意（095 OnRetreatResult / OnTieResult）。
void PopupTipUiSystem::EventButtonOmok(GamePopupTip& t, uint8_t accept) {
  auto* req = std::any_cast<GameOmokRequestTip>(&t.data);
  if (req != nullptr && req->player.client_id != 0) {
    fbs::ClientCharacterMinigameT m;
    m.step = static_cast<uint8_t>(req->kind + 1);  // 7=悔棋答复 / 9=求和答复
    m.x = accept;
    ClientRequest::SendToHost(m);
  }
  std::erase_if(PopupTipGameInstance::data_,
                [&t](const auto& tip) { return &t == &tip; });
}

void PopupTipUiSystem::EventButtonOk(GamePopupTip& t) {
  switch (t.type) {
    case PopupTipEnums::kTrade: {
      EventButtonTrade(t);

      break;
    }

    case PopupTipEnums::kMinigame: {
      EventButtonOmok(t, 1);

      break;
    }

    case PopupTipEnums::kParty: {
      EventButtonParty(t, 1);

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
        auto p = FadeYesBoxPos();
        SDL_FRect ok = FadeYesOkRect(p);
        SDL_FRect cancel = FadeYesCancelRect(p);
        if (SDL_PointInRectFloat(&Window::mouse_pos_, &ok)) {
          EventButtonTrade(t);  // 接受
          return true;
        }
        if (SDL_PointInRectFloat(&Window::mouse_pos_, &cancel)) {
          EventButtonCancel(t);  // 拒绝
          return true;
        }
        break;
      }
      case PopupTipEnums::kMinigame: {
        auto p = FadeYesBoxPos();
        SDL_FRect ok = FadeYesOkRect(p);
        SDL_FRect cancel = FadeYesCancelRect(p);
        if (SDL_PointInRectFloat(&Window::mouse_pos_, &ok)) {
          EventButtonOmok(t, 1);  // 同意
          return true;
        }
        if (SDL_PointInRectFloat(&Window::mouse_pos_, &cancel)) {
          EventButtonOmok(t, 0);  // 拒绝
          return true;
        }
        break;
      }
      case PopupTipEnums::kParty: {
        auto p = FadeYesBoxPos();
        SDL_FRect ok = FadeYesOkRect(p);
        SDL_FRect cancel = FadeYesCancelRect(p);
        if (SDL_PointInRectFloat(&Window::mouse_pos_, &ok)) {
          EventButtonParty(t, 1);  // 接受
          return true;
        }
        if (SDL_PointInRectFloat(&Window::mouse_pos_, &cancel)) {
          EventButtonParty(t, 0);  // 拒绝
          return true;
        }
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
        r = !EventUI(event);
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
