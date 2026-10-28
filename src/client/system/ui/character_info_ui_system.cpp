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

SDL_FPoint CharacterInfoUiSystem::LoadWh() { return SDL_FPoint{320, 183}; }

const static SDL_FPoint item_lt{316, 0};

void CharacterInfoUiSystem::RenderScroll() {
  if (!item) {
    return;
  }
  const SDL_FPoint lt{225 + item_lt.x, 31 + item_lt.y};
  const uint32_t length = 115;
  auto v = EquipGameInstance::LoadEquips(character).size();
  auto v2 = EquipGameInstance::LoadDecos(character).size();
  auto size = v + v2;
  auto CursorIn = CursorGameInstance::cursor_ui;
  bool top = CursorIn == Render && !CursorGameInstance::modal_overlay;
  ScrollUiSystem::RenderVscroll((int)pos.x + lt.x, (int)pos.y + lt.y,
                                   item_page, size, length, top, 3);
  return;
}

void CharacterInfoUiSystem::RenderBackgrnd() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui->find(u"CharacterInfo.img/RemoteInfo/backgrnd"));
  SDL_FRect pos_rect{
      static_cast<float>((int)pos.x),
      static_cast<float>((int)pos.y),
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);

  static auto backgrnd_detail = WzResource::LoadTexture(
      WzResource::ui->find(u"CharacterInfo.img/RemoteInfo/backgrnd_detail"));
  pos_rect = {
      static_cast<float>((int)pos.x + 8),
      static_cast<float>((int)pos.y + 25),
      static_cast<float>(backgrnd_detail->w),
      static_cast<float>(backgrnd_detail->h),
  };
  SDL_RenderTexture(Window::renderer, backgrnd_detail, nullptr, &pos_rect);

  if (item) {
    static auto backgrnd_item = WzResource::LoadTexture(
        WzResource::ui->find(u"CharacterInfo.img/ItemList/backgrnd"));
    pos_rect = {
        (int)pos.x + item_lt.x,
        (int)pos.y + item_lt.y,
        static_cast<float>(backgrnd_item->w),
        static_cast<float>(backgrnd_item->h),
    };
    SDL_RenderTexture(Window::renderer, backgrnd_item, nullptr, &pos_rect);
  }
}

void CharacterInfoUiSystem::RenderButton() {
  std::vector<wz::Node *> buttons_nodes = {
      WzResource::ui->find(u"CharacterInfo.img/RemoteInfo/button:party"),
      WzResource::ui->find(u"CharacterInfo.img/RemoteInfo/button:trade"),
      WzResource::ui->find(
          u"CharacterInfo.img/RemoteInfo/button:toggleItemlist"),
      WzResource::ui->find(
          u"CharacterInfo.img/RemoteInfo/button:showCitizenship"),
      WzResource::ui->find(u"CharacterInfo.img/RemoteInfo/button:upPop"),
      WzResource::ui->find(u"CharacterInfo.img/RemoteInfo/button:downPop"),
      WzResource::ui->find(u"CharacterInfo.img/RemoteInfo/button:showPet"),
      WzResource::ui->find(u"Basic.img/BtClose"),
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
  if (item) {
    buttons_nodes.push_back(
        WzResource::ui->find(u"CharacterInfo.img/ItemList/button:hide"));
    buttons_rect.push_back(SDL_FRect{227 + item_lt.x, 7 + item_lt.y, 12, 12});
  }

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = Window::mouse_pos;
    // 判断按钮是否被遮挡
    auto CursorIn = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && CursorIn == Render &&
        !CursorGameInstance::modal_overlay) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::LoadTexture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::LoadTexture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::LoadTexture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void CharacterInfoUiSystem::RenderCharacter() {
  auto &camera = CameraGameInstance::camera;
  character.pos.x = int(camera.x + pos.x + 68);
  character.pos.y = int(camera.y + pos.y + 119);
  CharacterRenderSystem::RenderCharacter(character);
}

void CharacterInfoUiSystem::RenderText() {
  Freetype::LoadAligned(true);
  Freetype::LoadSize(12);
  const auto label_offset = -43;
  // name
  auto charname = character.name;
  auto w = Freetype::LoadW(charname);
  Freetype::LoadColor(255, 255, 255, 255);
  auto x = int(pos.x + 67 - w / 2);
  Freetype::DrawLine(charname, x, int(pos.y + 127));

  Freetype::LoadColor(0, 0, 0, 255);
  // level
  auto level = character.level;
  std::string level2 = std::to_string(level);
  std::u16string level3{level2.begin(), level2.end()};
  x = int(pos.x + 208);
  Freetype::DrawLine(level3, x, int(pos.y + 53));
  x += label_offset;
  std::u16string str = u"LEVEL";
  w = Freetype::LoadW(str);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(str, x - w / 2, int(pos.y + 53));

  Freetype::LoadColor(0, 0, 0, 255);
  // job
  auto job_node =
      WzResource::ms->get_root()->find(u"String.img/Job/" + character.job);
  auto job_name = static_cast<wz::Property<std::u16string> *>(job_node)->get();
  Freetype::DrawLine(job_name, int(pos.x + 208), int(pos.y + 72));
  str = u"JOB";
  w = Freetype::LoadW(str);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(str, x - w / 2, int(pos.y + 72));

  Freetype::LoadColor(0, 0, 0, 255);
  auto fame = character.fame;
  std::string fame2 = std::to_string(fame);
  std::u16string fame3{fame2.begin(), fame2.end()};
  Freetype::DrawLine(fame3, int(pos.x + 208), int(pos.y + 91));
  str = u"FAME";
  w = Freetype::LoadW(str);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(str, x - w / 2, int(pos.y + 91));
}

void CharacterInfoUiSystem::RenderItems() {
  if (!item) {
    return;
  }
  const auto line_h = 38;
  auto w = 9;
  auto h = 31;
  auto v = EquipGameInstance::LoadEquips(character);
  auto v2 = EquipGameInstance::LoadDecos(character);
  for (uint8_t i = item_page; i < item_page + 3; i++) {
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
        WzResource::ui->find(u"CharacterInfo.img/ItemListEntry/backgrnd"));

    SDL_FRect pos_rect = {
        static_cast<float>(int(pos.x + item_lt.x + w)),
        static_cast<float>(
            int(pos.y + item_lt.y + h + line_h * (i - item_page))),
        static_cast<float>(entry_back->w),
        static_cast<float>(entry_back->h),
    };
    SDL_RenderTexture(Window::renderer, entry_back, nullptr, &pos_rect);

    auto info = EquipGameInstance::LoadEquipInfo(itm_id);
    auto texture = WzResource::LoadTexture(info->get_child(u"icon"));
    pos_rect = {
        static_cast<float>(int(pos.x + item_lt.x + w + (35 - texture->w) / 2)),
        static_cast<float>(int(pos.y + item_lt.y + h +
                               line_h * (i - item_page) +
                               (line_h - texture->h) / 2)),
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);

    if (cash) {
      auto cash_icon = WzResource::LoadTexture(
          WzResource::ms->get_root()->find(u"UI.img/CashItem"));
      SDL_FRect rect = {
          static_cast<float>(int(19 + pos.x + item_lt.x + w)),
          static_cast<float>(
              int(19 + pos.y + item_lt.y + h + line_h * (i - item_page))),
          static_cast<float>(cash_icon->w),
          static_cast<float>(cash_icon->h),
      };
      SDL_RenderTexture(Window::renderer, cash_icon, nullptr, &rect);
    }

    Freetype::LoadColor(0, 0, 0, 255);

    auto name = EquipGameInstance::LoadEquipName(itm_id);
    if (name.size() > 22) {
      name = name.substr(0, 22) + u"...";
    }
    Freetype::LoadAligned(true);
    Freetype::LoadSize(12);
    auto x = pos.x + item_lt.x + w + 43;
    auto y = pos.y + item_lt.y + h + line_h * (i - item_page) - 1;
    Freetype::DrawLine(name, x, y);

    x = pos.x + item_lt.x + w + 43;
    y = pos.y + item_lt.y + h + line_h * (i - item_page) + 17;
    Freetype::DrawLine(u"Level", x, y);

    auto equip_info = EquipGameInstance::LoadEquipInfo(itm_id);
    auto req =
        static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLevel"))
            ->get();
    std::string req2 = std::to_string(req);
    std::u16string req3{req2.begin(), req2.end()};
    x = pos.x + item_lt.x + w + 79;
    y = pos.y + item_lt.y + h + line_h * (i - item_page) + 17;
    Freetype::DrawLine(req3, x, y);

    auto CursorIn = CursorGameInstance::cursor_ui;
    if (CursorIn != Render) {
      continue;
    }
    auto &mouse_pos = Window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      if (i < v.size()) {
        item_info = v[i];
      } else {
        item_info = v2[i - v.size()];
      }
    }
  }
}

bool CharacterInfoUiSystem::RenderItemsInfo() {
  if (item_info.has_value()) {
    auto &mouse_pos = Window::mouse_pos;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    if (std::holds_alternative<GameEquipItem>(item_info.value())) {
      // 是装备类型
      auto &equip = std::get<GameEquipItem>(item_info.value());
      TooltipUiSystem::RenderEquip(equip, show_pos.x, show_pos.y);
    } else {
      auto &equip = std::get<GameDecoItem>(item_info.value());
      TooltipUiSystem::RenderDeco(equip, show_pos.x, show_pos.y);
    }
  }
  item_info = std::nullopt;
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
  if (!item) {
    return false;
  }
  const SDL_FPoint lt{219 + item_lt.x, 32 + item_lt.y};
  const uint32_t length = 115;
  auto v = EquipGameInstance::LoadEquips(character).size();
  auto v2 = EquipGameInstance::LoadDecos(character).size();
  int size = v + v2 - 3;
  auto CursorIn = CursorGameInstance::cursor_ui;
  bool top = CursorIn == Render;
  size = std::max(0, size);
  auto mouse_pos = SDL_FPoint{Event->button.x, Event->button.y};
  auto val =
      ScrollUiSystem::ClickVscroll((int)pos.x + lt.x, (int)pos.y + lt.y,
                                      item_page, size, length, top, mouse_pos);
  item_page = val;
  return true;
}

void CharacterInfoUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::Render);
  if (it != System::render_systems.end()) {
    auto wh = LoadWh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    item = false;
    pet = false;

    CharacterLogicSystem::RunStandAction(character);
    CharacterLogicSystem::RunFaceAction(character, u"default");
    character.flip = 0;

    System::render_systems.insert(it, Render);
    System::event_systems.insert(System::event_systems.begin(), Event);

    EventMotion(nullptr);
  }
}

void CharacterInfoUiSystem::Close() {
  std::erase(System::render_systems, Render);
  std::erase(System::event_systems, Event);
  std::erase(System::render_systems, RenderItemsInfo);
}

void CharacterInfoUiSystem::EventTop() {
  std::erase(System::render_systems, Render);
  std::erase(System::event_systems, Event);
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::Render);
  if (it != System::render_systems.end()) {
    System::render_systems.insert(it, Render);
    System::event_systems.insert(System::event_systems.begin(), Event);

    EventMotion(nullptr);
  }
}

void CharacterInfoUiSystem::EventDragStart(SDL_Event *Event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {Event->button.x, Event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - Event->button.x, pos.y - Event->button.y};
  }
  return;
}

void CharacterInfoUiSystem::EventDragEnd() {
  drag = std::nullopt;
  return;
}

void CharacterInfoUiSystem::EventDragMove(SDL_Event *Event) {
  if (drag.has_value()) {
    pos = {Event->motion.x + drag->x, Event->motion.y + drag->y};
    auto &camera = CameraGameInstance::camera;
    auto [w, h] = LoadWh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void CharacterInfoUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems, fn)) {
    Close();
  } else {
    Open();
  }
}

bool CharacterInfoUiSystem::CursorIn() {
  bool r = false;
  auto &mouse = Window::mouse_pos;
  auto [w, h] = LoadWh();
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  r = SDL_PointInRectFloat(&mouse, &pos_rect);
  if (r) {
    return r;
  }
  if (item) {
    pos_rect = {pos.x + item_lt.x, pos.y + item_lt.y, 246, 160};
    r = SDL_PointInRectFloat(&mouse, &pos_rect);
    if (r) {
      return r;
    }
  }
  if (pet) {
  }
  return r;
}

void CharacterInfoUiSystem::EventButtonParty() {}

void CharacterInfoUiSystem::EventButtonTrade() {}

void CharacterInfoUiSystem::EventButtonToggleitem() { item = true; }

void CharacterInfoUiSystem::EventButtonToggleitemHide() { item = false; }

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
  if (item) {
    buttons_rect.push_back(SDL_FRect{227 + item_lt.x, 7 + item_lt.y, 12, 12});
    fns.push_back(EventButtonToggleitemHide);
  }

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
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
  if (vscr_motion == false) {
    vscr_motion =
        ScrollUiSystem::ClickThumb(pos.x + lt.x, pos.y + lt.y, length);
  }
}

void CharacterInfoUiSystem::EventVscrEnd() { vscr_motion = false; }

void CharacterInfoUiSystem::EventVscrMove(SDL_Event *Event) {
  auto mouse_state = Window::mouse_state;
  if (vscr_motion) {
    const SDL_FPoint lt{219 + item_lt.x, 32 + item_lt.y};
    const uint32_t length = 115;

    Event->button.x = pos.x + lt.x;
    Event->button.y =
        std::clamp(Event->button.y, pos.y + lt.y, pos.y + lt.y + length);
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
  if (item) {
    buttons_rect.push_back(SDL_FRect{227 + item_lt.x, 7 + item_lt.y, 12, 12});
  }
  static int8_t bt_audio = -1;
  auto &mouse_pos = Window::mouse_pos;
  int8_t bt_in = -1;
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
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
    if (item_page > 0) {
      item_page--;
    }
  } else {
    auto v = EquipGameInstance::LoadEquips(character).size();
    auto v2 = EquipGameInstance::LoadDecos(character).size();
    int size = v + v2 - 3;
    size = std::max(0, size);
    if (item_page < size) {
      item_page++;
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
  auto &sys = System::render_systems;
  std::erase(sys, RenderItemsInfo);
  auto it = std::ranges::find(sys, &CursorRenderSystem::Render);
  if (it != sys.end()) {
    sys.insert(it, RenderItemsInfo);
  }
}