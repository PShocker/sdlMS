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

SDL_FPoint CharacterInfoUiSystem::load_wh() { return SDL_FPoint{320, 183}; }

const static SDL_FPoint item_lt{316, 0};

void CharacterInfoUiSystem::render_scroll() {
  if (!item) {
    return;
  }
  const SDL_FPoint lt{225 + item_lt.x, 31 + item_lt.y};
  const uint32_t length = 115;
  auto v = EquipGameInstance::load_equips(character).size();
  auto v2 = EquipGameInstance::load_decos(character).size();
  auto size = v + v2;
  auto cursor_in = CursorGameInstance::cursor_ui;
  bool top = cursor_in == render && !CursorGameInstance::modal_overlay;
  ScrollUiSystem::render_vscroll((int)pos.x + lt.x, (int)pos.y + lt.y,
                                   item_page, size, length, top, 3);
  return;
}

void CharacterInfoUiSystem::render_backgrnd() {
  static auto backgrnd = WzResource::load_texture(
      WzResource::ui->find(u"CharacterInfo.img/RemoteInfo/backgrnd"));
  SDL_FRect pos_rect{
      static_cast<float>((int)pos.x),
      static_cast<float>((int)pos.y),
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);

  static auto backgrnd_detail = WzResource::load_texture(
      WzResource::ui->find(u"CharacterInfo.img/RemoteInfo/backgrnd_detail"));
  pos_rect = {
      static_cast<float>((int)pos.x + 8),
      static_cast<float>((int)pos.y + 25),
      static_cast<float>(backgrnd_detail->w),
      static_cast<float>(backgrnd_detail->h),
  };
  SDL_RenderTexture(Window::renderer, backgrnd_detail, nullptr, &pos_rect);

  if (item) {
    static auto backgrnd_item = WzResource::load_texture(
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

void CharacterInfoUiSystem::render_button() {
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
  auto wh = load_wh();
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
    auto cursor_in = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
        !CursorGameInstance::modal_overlay) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void CharacterInfoUiSystem::render_character() {
  auto &camera = CameraGameInstance::camera;
  character.pos.x = int(camera.x + pos.x + 68);
  character.pos.y = int(camera.y + pos.y + 119);
  CharacterRenderSystem::render_character(character);
}

void CharacterInfoUiSystem::render_text() {
  Freetype::load_aligned(true);
  Freetype::load_size(12);
  const auto label_offset = -43;
  // name
  auto charname = character.name;
  auto w = Freetype::load_w(charname);
  Freetype::load_color(255, 255, 255, 255);
  auto x = int(pos.x + 67 - w / 2);
  Freetype::draw_line(charname, x, int(pos.y + 127));

  Freetype::load_color(0, 0, 0, 255);
  // level
  auto level = character.level;
  std::string level2 = std::to_string(level);
  std::u16string level3{level2.begin(), level2.end()};
  x = int(pos.x + 208);
  Freetype::draw_line(level3, x, int(pos.y + 53));
  x += label_offset;
  std::u16string str = u"LEVEL";
  w = Freetype::load_w(str);
  Freetype::load_color(255, 255, 255, 255);
  Freetype::draw_line(str, x - w / 2, int(pos.y + 53));

  Freetype::load_color(0, 0, 0, 255);
  // job
  auto job_node =
      WzResource::ms->get_root()->find(u"String.img/Job/" + character.job);
  auto job_name = static_cast<wz::Property<std::u16string> *>(job_node)->get();
  Freetype::draw_line(job_name, int(pos.x + 208), int(pos.y + 72));
  str = u"JOB";
  w = Freetype::load_w(str);
  Freetype::load_color(255, 255, 255, 255);
  Freetype::draw_line(str, x - w / 2, int(pos.y + 72));

  Freetype::load_color(0, 0, 0, 255);
  auto fame = character.fame;
  std::string fame2 = std::to_string(fame);
  std::u16string fame3{fame2.begin(), fame2.end()};
  Freetype::draw_line(fame3, int(pos.x + 208), int(pos.y + 91));
  str = u"FAME";
  w = Freetype::load_w(str);
  Freetype::load_color(255, 255, 255, 255);
  Freetype::draw_line(str, x - w / 2, int(pos.y + 91));
}

void CharacterInfoUiSystem::render_items() {
  if (!item) {
    return;
  }
  const auto line_h = 38;
  auto w = 9;
  auto h = 31;
  auto v = EquipGameInstance::load_equips(character);
  auto v2 = EquipGameInstance::load_decos(character);
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

    static auto entry_back = WzResource::load_texture(
        WzResource::ui->find(u"CharacterInfo.img/ItemListEntry/backgrnd"));

    SDL_FRect pos_rect = {
        static_cast<float>(int(pos.x + item_lt.x + w)),
        static_cast<float>(
            int(pos.y + item_lt.y + h + line_h * (i - item_page))),
        static_cast<float>(entry_back->w),
        static_cast<float>(entry_back->h),
    };
    SDL_RenderTexture(Window::renderer, entry_back, nullptr, &pos_rect);

    auto info = EquipGameInstance::load_equip_info(itm_id);
    auto texture = WzResource::load_texture(info->get_child(u"icon"));
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
      auto cash_icon = WzResource::load_texture(
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

    Freetype::load_color(0, 0, 0, 255);

    auto name = EquipGameInstance::load_equip_name(itm_id);
    if (name.size() > 22) {
      name = name.substr(0, 22) + u"...";
    }
    Freetype::load_aligned(true);
    Freetype::load_size(12);
    auto x = pos.x + item_lt.x + w + 43;
    auto y = pos.y + item_lt.y + h + line_h * (i - item_page) - 1;
    Freetype::draw_line(name, x, y);

    x = pos.x + item_lt.x + w + 43;
    y = pos.y + item_lt.y + h + line_h * (i - item_page) + 17;
    Freetype::draw_line(u"Level", x, y);

    auto equip_info = EquipGameInstance::load_equip_info(itm_id);
    auto req =
        static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLevel"))
            ->get();
    std::string req2 = std::to_string(req);
    std::u16string req3{req2.begin(), req2.end()};
    x = pos.x + item_lt.x + w + 79;
    y = pos.y + item_lt.y + h + line_h * (i - item_page) + 17;
    Freetype::draw_line(req3, x, y);

    auto cursor_in = CursorGameInstance::cursor_ui;
    if (cursor_in != render) {
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

bool CharacterInfoUiSystem::render_items_info() {
  if (item_info.has_value()) {
    auto &mouse_pos = Window::mouse_pos;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    if (std::holds_alternative<GameEquipItem>(item_info.value())) {
      // 是装备类型
      auto &equip = std::get<GameEquipItem>(item_info.value());
      TooltipUiSystem::render_equip(equip, show_pos.x, show_pos.y);
    } else {
      auto &equip = std::get<GameDecoItem>(item_info.value());
      TooltipUiSystem::render_deco(equip, show_pos.x, show_pos.y);
    }
  }
  item_info = std::nullopt;
  return true;
}

bool CharacterInfoUiSystem::render() {
  render_backgrnd();
  render_scroll();
  render_button();
  render_character();
  render_text();
  render_items();
  return true;
}

bool CharacterInfoUiSystem::event_vscr(SDL_Event *event) {
  if (!item) {
    return false;
  }
  const SDL_FPoint lt{219 + item_lt.x, 32 + item_lt.y};
  const uint32_t length = 115;
  auto v = EquipGameInstance::load_equips(character).size();
  auto v2 = EquipGameInstance::load_decos(character).size();
  int size = v + v2 - 3;
  auto cursor_in = CursorGameInstance::cursor_ui;
  bool top = cursor_in == render;
  size = std::max(0, size);
  auto mouse_pos = SDL_FPoint{event->button.x, event->button.y};
  auto val =
      ScrollUiSystem::click_vscroll((int)pos.x + lt.x, (int)pos.y + lt.y,
                                      item_page, size, length, top, mouse_pos);
  item_page = val;
  return true;
}

void CharacterInfoUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    item = false;
    pet = false;

    CharacterLogicSystem::run_stand_action(character);
    CharacterLogicSystem::run_face_action(character, u"default");
    character.flip = 0;

    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

void CharacterInfoUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
  std::erase(System::render_systems, render_items_info);
}

void CharacterInfoUiSystem::event_top() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

void CharacterInfoUiSystem::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void CharacterInfoUiSystem::event_drag_end() {
  drag = std::nullopt;
  return;
}

void CharacterInfoUiSystem::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = CameraGameInstance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void CharacterInfoUiSystem::toggle() {
  auto fn = &render;
  if (std::ranges::contains(System::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool CharacterInfoUiSystem::cursor_in() {
  bool r = false;
  auto &mouse = Window::mouse_pos;
  auto [w, h] = load_wh();
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

void CharacterInfoUiSystem::event_button_party() {}

void CharacterInfoUiSystem::event_button_trade() {}

void CharacterInfoUiSystem::event_button_toggleitem() { item = true; }

void CharacterInfoUiSystem::event_button_toggleitem_hide() { item = false; }

void CharacterInfoUiSystem::event_button_citizenship() {}

void CharacterInfoUiSystem::event_button_up_fame() {}

void CharacterInfoUiSystem::event_button_dowm_fame() {}

void CharacterInfoUiSystem::event_button_show_pet() {}

void CharacterInfoUiSystem::event_close() { close(); }

bool CharacterInfoUiSystem::event_button(SDL_Event *event) {
  std::vector<void (*)()> fns;
  auto wh = load_wh();
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
      event_button_party,       event_button_trade,   event_button_toggleitem,
      event_button_citizenship, event_button_up_fame, event_button_dowm_fame,
      event_button_show_pet,    event_close,
  };
  if (item) {
    buttons_rect.push_back(SDL_FRect{227 + item_lt.x, 7 + item_lt.y, 12, 12});
    fns.push_back(event_button_toggleitem_hide);
  }

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      fns[i]();
      AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }

  return false;
}

void CharacterInfoUiSystem::event_vscr_start(SDL_Event *event) {
  const SDL_FPoint lt{219 + item_lt.x, 32 + item_lt.y};
  const uint32_t length = 115;
  if (vscr_motion == false) {
    vscr_motion =
        ScrollUiSystem::click_thumb(pos.x + lt.x, pos.y + lt.y, length);
  }
}

void CharacterInfoUiSystem::event_vscr_end() { vscr_motion = false; }

void CharacterInfoUiSystem::event_vscr_move(SDL_Event *event) {
  auto mouse_state = Window::mouse_state;
  if (vscr_motion) {
    const SDL_FPoint lt{219 + item_lt.x, 32 + item_lt.y};
    const uint32_t length = 115;

    event->button.x = pos.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos.y + lt.y, pos.y + lt.y + length);
    event_vscr(event);
  }
}

void CharacterInfoUiSystem::event_motion_bt_audio(SDL_Event *event) {
  if (CursorGameInstance::cursor_ui != render) {
    return;
  }
  auto wh = load_wh();
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
    AudioGameInstance::load_audio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

void CharacterInfoUiSystem::event_wheel(SDL_Event *event) {
  if (CursorGameInstance::cursor_ui != render) {
    return;
  }
  auto dy = event->wheel.integer_y;
  if (dy > 0) {
    // up
    if (item_page > 0) {
      item_page--;
    }
  } else {
    auto v = EquipGameInstance::load_equips(character).size();
    auto v2 = EquipGameInstance::load_decos(character).size();
    int size = v + v2 - 3;
    size = std::max(0, size);
    if (item_page < size) {
      item_page++;
    }
  }
}

bool CharacterInfoUiSystem::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      event_close();
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
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
        event_top();
        event_drag_start(event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
        event_vscr(event);
        r = !event_button(event);
      }
      event_drag_end();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_motion(event);
    event_motion_bt_audio(event);
    event_drag_move(event);
    event_vscr_move(event);
    break;
  }
  case SDL_EVENT_MOUSE_WHEEL: {
    event_wheel(event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

void CharacterInfoUiSystem::event_motion(SDL_Event *event) {
  auto &sys = System::render_systems;
  std::erase(sys, render_items_info);
  auto it = std::ranges::find(sys, &CursorRenderSystem::render);
  if (it != sys.end()) {
    sys.insert(it, render_items_info);
  }
}