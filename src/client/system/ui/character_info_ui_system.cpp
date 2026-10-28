#include "character_info_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "scroll_ui_system.h"
#include "src/client/game/game_item.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "tooltip_ui_system.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <ranges>
#include <string>
#include <vector>
namespace sdlms {


SDL_FPoint CharacterInfoUiSystem::LoadWh() { return SDL_FPoint{320, 183}; }

const static SDL_FPoint item_lt{316, 0};

void CharacterInfoUiSystem::RenderScroll() {
  if (!item_) {
    return;
  }
  const SDL_FPoint lt{225 + item_lt.x, 31 + item_lt.y};
  const uint32_t length = 115;
  auto v = EquipGameInstance::LoadEquips(character_).size();
  auto v2 = EquipGameInstance::LoadDecos(character_).size();
  auto size = v + v2;
  auto CursorIn = CursorGameInstance::cursor_ui;
  bool top = CursorIn == Render && !CursorGameInstance::modal_overlay;
  ScrollUiSystem::RenderVscroll((int)pos_.x + lt.x, (int)pos_.y + lt.y,
                                   item_page_, size, length, top, 3);
  return;
}

void CharacterInfoUiSystem::RenderBackgrnd() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"CharacterInfo.img/RemoteInfo/backgrnd"));
  SDL_FRect pos_rect{
      static_cast<float>((int)pos_.x),
      static_cast<float>((int)pos_.y),
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);

  static auto backgrnd_detail = WzResource::LoadTexture(
      WzResource::ui_->find(u"CharacterInfo.img/RemoteInfo/backgrnd_detail"));
  pos_rect = {
      static_cast<float>((int)pos_.x + 8),
      static_cast<float>((int)pos_.y + 25),
      static_cast<float>(backgrnd_detail->w),
      static_cast<float>(backgrnd_detail->h),
  };
  SDL_RenderTexture(Window::renderer_, backgrnd_detail, nullptr, &pos_rect);

  if (item_) {
    static auto backgrnd_item = WzResource::LoadTexture(
        WzResource::ui_->find(u"CharacterInfo.img/ItemList/backgrnd"));
    pos_rect = {
        (int)pos_.x + item_lt.x,
        (int)pos_.y + item_lt.y,
        static_cast<float>(backgrnd_item->w),
        static_cast<float>(backgrnd_item->h),
    };
    SDL_RenderTexture(Window::renderer_, backgrnd_item, nullptr, &pos_rect);
  }
}

void CharacterInfoUiSystem::RenderButton() {
  std::vector<wz::Node *> buttons_nodes = {
      WzResource::ui_->find(u"CharacterInfo.img/RemoteInfo/button:party"),
      WzResource::ui_->find(u"CharacterInfo.img/RemoteInfo/button:trade"),
      WzResource::ui_->find(
          u"CharacterInfo.img/RemoteInfo/button:toggleItemlist"),
      WzResource::ui_->find(
          u"CharacterInfo.img/RemoteInfo/button:showCitizenship"),
      WzResource::ui_->find(u"CharacterInfo.img/RemoteInfo/button:upPop"),
      WzResource::ui_->find(u"CharacterInfo.img/RemoteInfo/button:downPop"),
      WzResource::ui_->find(u"CharacterInfo.img/RemoteInfo/button:showPet"),
      WzResource::ui_->find(u"Basic.img/BtClose"),
  };
  auto wh = LoadWh();
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{130, 133, 89, 16},     //
      SDL_FRect{223, 133, 89, 16},     //
      SDL_FRect{214, 26, 80, 16},      //
      SDL_FRect{130, 26, 80, 16},      //
      SDL_FRect{285, 94, 12, 12},      //
      SDL_FRect{298, 94, 12, 12},      //
      SDL_FRect{245, 160, 67, 16},     //
      SDL_FRect{wh.x - 20, 7, 12, 12}, //
  };
  if (item_) {
    buttons_nodes.push_back(
        WzResource::ui_->find(u"CharacterInfo.img/ItemList/button:hide"));
    buttons_rect.push_back(SDL_FRect{227 + item_lt.x, 7 + item_lt.y, 12, 12});
  }

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = Window::mouse_pos_;
    // 判断按钮是否被遮挡
    auto CursorIn = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && CursorIn == Render &&
        !CursorGameInstance::modal_overlay) {
      if (Window::mouse_state_ & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::LoadTexture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer_, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::LoadTexture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer_, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::LoadTexture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer_, normal, nullptr, &pos_rect);
    }
  }
}

void CharacterInfoUiSystem::RenderCharacter() {
  auto &camera = CameraGameInstance::camera_;
  character_.pos.x = int(camera.x + pos_.x + 68);
  character_.pos.y = int(camera.y + pos_.y + 119);
  CharacterRenderSystem::RenderCharacter(character_);
}

void CharacterInfoUiSystem::RenderText() {
  Freetype::LoadAligned(true);
  Freetype::LoadSize(12);
  const auto label_offset = -43;
  // name
  auto charname = character_.name;
  auto w = Freetype::LoadW(charname);
  Freetype::LoadColor(255, 255, 255, 255);
  auto x = int(pos_.x + 67 - w / 2);
  Freetype::DrawLine(charname, x, int(pos_.y + 127));

  Freetype::LoadColor(0, 0, 0, 255);
  // level
  auto level = character_.level;
  std::string level2 = std::to_string(level);
  std::u16string level3{level2.begin(), level2.end()};
  x = int(pos_.x + 208);
  Freetype::DrawLine(level3, x, int(pos_.y + 53));
  x += label_offset;
  std::u16string str = u"LEVEL";
  w = Freetype::LoadW(str);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(str, x - w / 2, int(pos_.y + 53));

  Freetype::LoadColor(0, 0, 0, 255);
  // job
  auto job_node =
      WzResource::ms_->get_root()->find(u"String.img/Job/" + character_.job);
  auto job_name = static_cast<wz::Property<std::u16string> *>(job_node)->get();
  Freetype::DrawLine(job_name, int(pos_.x + 208), int(pos_.y + 72));
  str = u"JOB";
  w = Freetype::LoadW(str);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(str, x - w / 2, int(pos_.y + 72));

  Freetype::LoadColor(0, 0, 0, 255);
  auto fame = character_.fame;
  std::string fame2 = std::to_string(fame);
  std::u16string fame3{fame2.begin(), fame2.end()};
  Freetype::DrawLine(fame3, int(pos_.x + 208), int(pos_.y + 91));
  str = u"FAME";
  w = Freetype::LoadW(str);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(str, x - w / 2, int(pos_.y + 91));
}

void CharacterInfoUiSystem::RenderItems() {
  if (!item_) {
    return;
  }
  const auto line_h = 38;
  auto w = 9;
  auto h = 31;
  auto v = EquipGameInstance::LoadEquips(character_);
  auto v2 = EquipGameInstance::LoadDecos(character_);
  for (uint8_t i = item_page_; i < item_page_ + 3; i++) {
    if (i >= v.size() + v2.size()) {
      break;
    }
    std::u16string itm_id;
    bool cash = false;
    if (i < v.size()) {
      itm_id = v[i].id;
    } else {
      itm_id = v2[i - v.size()].id;
      cash = true;
    }

    static auto entry_back = WzResource::LoadTexture(
        WzResource::ui_->find(u"CharacterInfo.img/ItemListEntry/backgrnd"));

    SDL_FRect pos_rect = {
        static_cast<float>(int(pos_.x + item_lt.x + w)),
        static_cast<float>(
            int(pos_.y + item_lt.y + h + line_h * (i - item_page_))),
        static_cast<float>(entry_back->w),
        static_cast<float>(entry_back->h),
    };
    SDL_RenderTexture(Window::renderer_, entry_back, nullptr, &pos_rect);

    auto info = EquipGameInstance::LoadEquipInfo(itm_id);
    auto texture = WzResource::LoadTexture(info->get_child(u"icon"));
    pos_rect = {
        static_cast<float>(int(pos_.x + item_lt.x + w + (35 - texture->w) / 2)),
        static_cast<float>(int(pos_.y + item_lt.y + h +
                               line_h * (i - item_page_) +
                               (line_h - texture->h) / 2)),
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);

    if (cash) {
      auto cash_icon = WzResource::LoadTexture(
          WzResource::ms_->get_root()->find(u"UI.img/CashItem"));
      SDL_FRect rect = {
          static_cast<float>(int(19 + pos_.x + item_lt.x + w)),
          static_cast<float>(
              int(19 + pos_.y + item_lt.y + h + line_h * (i - item_page_))),
          static_cast<float>(cash_icon->w),
          static_cast<float>(cash_icon->h),
      };
      SDL_RenderTexture(Window::renderer_, cash_icon, nullptr, &rect);
    }

    Freetype::LoadColor(0, 0, 0, 255);

    auto name = EquipGameInstance::LoadEquipName(itm_id);
    if (name.size() > 22) {
      name = name.substr(0, 22) + u"...";
    }
    Freetype::LoadAligned(true);
    Freetype::LoadSize(12);
    auto x = pos_.x + item_lt.x + w + 43;
    auto y = pos_.y + item_lt.y + h + line_h * (i - item_page_) - 1;
    Freetype::DrawLine(name, x, y);

    x = pos_.x + item_lt.x + w + 43;
    y = pos_.y + item_lt.y + h + line_h * (i - item_page_) + 17;
    Freetype::DrawLine(u"Level", x, y);

    auto equip_info = EquipGameInstance::LoadEquipInfo(itm_id);
    auto req =
        static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLevel"))
            ->get();
    std::string req2 = std::to_string(req);
    std::u16string req3{req2.begin(), req2.end()};
    x = pos_.x + item_lt.x + w + 79;
    y = pos_.y + item_lt.y + h + line_h * (i - item_page_) + 17;
    Freetype::DrawLine(req3, x, y);

    auto CursorIn = CursorGameInstance::cursor_ui;
    if (CursorIn != Render) {
      continue;
    }
    auto &mouse_pos = Window::mouse_pos_;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      if (i < v.size()) {
        item_info_ = v[i];
      } else {
        item_info_ = v2[i - v.size()];
      }
    }
  }
}

bool CharacterInfoUiSystem::RenderItemsInfo() {
  if (item_info_.has_value()) {
    auto &mouse_pos = Window::mouse_pos_;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    if (std::holds_alternative<GameEquipItem>(item_info_.value())) {
      // 是装备类型
      auto &equip = std::get<GameEquipItem>(item_info_.value());
      TooltipUiSystem::RenderEquip(equip, show_pos.x, show_pos.y);
    } else {
      auto &equip = std::get<GameDecoItem>(item_info_.value());
      TooltipUiSystem::RenderDeco(equip, show_pos.x, show_pos.y);
    }
  }
  item_info_ = std::nullopt;
  return true;
}

bool CharacterInfoUiSystem::Render() {
  RenderBackgrnd();
  RenderScroll();
  RenderButton();
  RenderCharacter();
  RenderText();
  RenderItems();
  return true;
}

bool CharacterInfoUiSystem::EventVscr(SDL_Event *Event) {
  if (!item_) {
    return false;
  }
  const SDL_FPoint lt{219 + item_lt.x, 32 + item_lt.y};
  const uint32_t length = 115;
  auto v = EquipGameInstance::LoadEquips(character_).size();
  auto v2 = EquipGameInstance::LoadDecos(character_).size();
  int size = v + v2 - 3;
  auto CursorIn = CursorGameInstance::cursor_ui;
  bool top = CursorIn == Render;
  size = std::max(0, size);
  auto mouse_pos = SDL_FPoint{Event->button.x, Event->button.y};
  auto val =
      ScrollUiSystem::ClickVscroll((int)pos_.x + lt.x, (int)pos_.y + lt.y,
                                      item_page_, size, length, top, mouse_pos);
  item_page_ = val;
  return true;
}

void CharacterInfoUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto &camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    item_ = false;
    pet_ = false;

    CharacterLogicSystem::RunStandAction(character_);
    CharacterLogicSystem::RunFaceAction(character_, u"default");
    character_.flip = 0;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);

    EventMotion(nullptr);
  }
}

void CharacterInfoUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  std::erase(System::render_systems_, RenderItemsInfo);
}

void CharacterInfoUiSystem::EventTop() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);

    EventMotion(nullptr);
  }
}

void CharacterInfoUiSystem::EventDragStart(SDL_Event *Event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos_.x, pos_.y, wh.x, 20};
  SDL_FPoint mouse_pos = {Event->button.x, Event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - Event->button.x, pos_.y - Event->button.y};
  }
  return;
}

void CharacterInfoUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
  return;
}

void CharacterInfoUiSystem::EventDragMove(SDL_Event *Event) {
  if (drag_.has_value()) {
    pos_ = {Event->motion.x + drag_->x, Event->motion.y + drag_->y};
    auto &camera = CameraGameInstance::camera_;
    auto [w, h] = LoadWh();
    pos_.x = std::clamp(pos_.x, (float)0, camera.w - w);
    pos_.y = std::clamp(pos_.y, (float)0, camera.h - h);
  }
  return;
}

void CharacterInfoUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

bool CharacterInfoUiSystem::CursorIn() {
  bool r = false;
  auto &mouse = Window::mouse_pos_;
  auto [w, h] = LoadWh();
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  r = SDL_PointInRectFloat(&mouse, &pos_rect);
  if (r) {
    return r;
  }
  if (item_) {
    pos_rect = {pos_.x + item_lt.x, pos_.y + item_lt.y, 246, 160};
    r = SDL_PointInRectFloat(&mouse, &pos_rect);
    if (r) {
      return r;
    }
  }
  if (pet_) {
  }
  return r;
}

void CharacterInfoUiSystem::EventButtonParty() {}

void CharacterInfoUiSystem::EventButtonTrade() {}

void CharacterInfoUiSystem::EventButtonToggleitem() { item_ = true; }

void CharacterInfoUiSystem::EventButtonToggleitemHide() { item_ = false; }

void CharacterInfoUiSystem::EventButtonCitizenship() {}

void CharacterInfoUiSystem::EventButtonUpFame() {}

void CharacterInfoUiSystem::EventButtonDowmFame() {}

void CharacterInfoUiSystem::EventButtonShowPet() {}

void CharacterInfoUiSystem::EventClose() { Close(); }

bool CharacterInfoUiSystem::EventButton(SDL_Event *Event) {
  std::vector<void (*)()> fns;
  auto wh = LoadWh();
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{130, 133, 89, 16},     //
      SDL_FRect{223, 133, 89, 16},     //
      SDL_FRect{214, 26, 80, 16},      //
      SDL_FRect{130, 26, 80, 16},      //
      SDL_FRect{285, 94, 12, 12},      //
      SDL_FRect{298, 94, 12, 12},      //
      SDL_FRect{245, 160, 67, 16},     //
      SDL_FRect{wh.x - 20, 7, 12, 12}, //
  };
  fns = {
      EventButtonParty,       EventButtonTrade,   EventButtonToggleitem,
      EventButtonCitizenship, EventButtonUpFame, EventButtonDowmFame,
      EventButtonShowPet,    EventClose,
  };
  if (item_) {
    buttons_rect.push_back(SDL_FRect{227 + item_lt.x, 7 + item_lt.y, 12, 12});
    fns.push_back(EventButtonToggleitemHide);
  }

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos_.x;
    pos_rect.y += (int)pos_.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      fns[i]();
      AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }

  return false;
}

void CharacterInfoUiSystem::EventVscrStart(SDL_Event *Event) {
  const SDL_FPoint lt{219 + item_lt.x, 32 + item_lt.y};
  const uint32_t length = 115;
  if (vscr_motion_ == false) {
    vscr_motion_ =
        ScrollUiSystem::ClickThumb(pos_.x + lt.x, pos_.y + lt.y, length);
  }
}

void CharacterInfoUiSystem::EventVscrEnd() { vscr_motion_ = false; }

void CharacterInfoUiSystem::EventVscrMove(SDL_Event *Event) {
  auto mouse_state = Window::mouse_state_;
  if (vscr_motion_) {
    const SDL_FPoint lt{219 + item_lt.x, 32 + item_lt.y};
    const uint32_t length = 115;

    Event->button.x = pos_.x + lt.x;
    Event->button.y =
        std::clamp(Event->button.y, pos_.y + lt.y, pos_.y + lt.y + length);
    EventVscr(Event);
  }
}

void CharacterInfoUiSystem::EventMotionBtAudio(SDL_Event *Event) {
  if (CursorGameInstance::cursor_ui != Render) {
    return;
  }
  auto wh = LoadWh();
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{130, 133, 89, 16},     //
      SDL_FRect{223, 133, 89, 16},     //
      SDL_FRect{214, 26, 80, 16},      //
      SDL_FRect{130, 26, 80, 16},      //
      SDL_FRect{285, 94, 12, 12},      //
      SDL_FRect{298, 94, 12, 12},      //
      SDL_FRect{245, 160, 67, 16},     //
      SDL_FRect{wh.x - 20, 7, 12, 12}, //
  };
  if (item_) {
    buttons_rect.push_back(SDL_FRect{227 + item_lt.x, 7 + item_lt.y, 12, 12});
  }
  static int8_t bt_audio = -1;
  auto &mouse_pos = Window::mouse_pos_;
  int8_t bt_in = -1;
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos_.x;
    pos_rect.y += (int)pos_.y;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      bt_in = i;
      break;
    }
  }
  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

void CharacterInfoUiSystem::EventWheel(SDL_Event *Event) {
  if (CursorGameInstance::cursor_ui != Render) {
    return;
  }
  auto dy = Event->wheel.integer_y;
  if (dy > 0) {
    // up
    if (item_page_ > 0) {
      item_page_--;
    }
  } else {
    auto v = EquipGameInstance::LoadEquips(character_).size();
    auto v2 = EquipGameInstance::LoadDecos(character_).size();
    int size = v + v2 - 3;
    size = std::max(0, size);
    if (item_page_ < size) {
      item_page_++;
    }
  }
}

bool CharacterInfoUiSystem::Event(SDL_Event *Event) {
  bool r = true;
  switch (Event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = Event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      EventClose();
      return false;
      break;
    }
    default: {
      break;
    }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        EventTop();
        EventDragStart(Event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        EventVscr(Event);
        r = !EventButton(Event);
      }
      EventDragEnd();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    EventMotion(Event);
    EventMotionBtAudio(Event);
    EventDragMove(Event);
    EventVscrMove(Event);
    break;
  }
  case SDL_EVENT_MOUSE_WHEEL: {
    EventWheel(Event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

void CharacterInfoUiSystem::EventMotion(SDL_Event *Event) {
  auto &sys = System::render_systems_;
  std::erase(sys, RenderItemsInfo);
  auto it = std::ranges::find(sys, &CursorRenderSystem::Render);
  if (it != sys.end()) {
    sys.insert(it, RenderItemsInfo);
  }
}

}  // namespace sdlms
