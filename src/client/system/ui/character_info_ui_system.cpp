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

SDL_FPoint character_info_ui_system::load_wh() { return SDL_FPoint{320, 183}; }

const static SDL_FPoint item_lt{316, 0};

void character_info_ui_system::render_scroll() {
  if (!item) {
    return;
  }
  const SDL_FPoint lt{225 + item_lt.x, 31 + item_lt.y};
  const uint32_t length = 115;
  auto v = equip_game_instance::load_equips(character).size();
  auto v2 = equip_game_instance::load_decos(character).size();
  auto size = v + v2;
  auto cursor_in = cursor_game_instance::cursor_ui;
  bool top = cursor_in == render && !cursor_game_instance::modal_overlay;
  scroll_ui_system::render_vscroll((int)pos.x + lt.x, (int)pos.y + lt.y,
                                   item_page, size, length, top, 3);
  return;
}

void character_info_ui_system::render_backgrnd() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/backgrnd"));
  SDL_FRect pos_rect{
      static_cast<float>((int)pos.x),
      static_cast<float>((int)pos.y),
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);

  static auto backgrnd_detail = wz_resource::load_texture(
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/backgrnd_detail"));
  pos_rect = {
      static_cast<float>((int)pos.x + 8),
      static_cast<float>((int)pos.y + 25),
      static_cast<float>(backgrnd_detail->w),
      static_cast<float>(backgrnd_detail->h),
  };
  SDL_RenderTexture(window::renderer, backgrnd_detail, nullptr, &pos_rect);

  if (item) {
    static auto backgrnd_item = wz_resource::load_texture(
        wz_resource::ui->find(u"CharacterInfo.img/ItemList/backgrnd"));
    pos_rect = {
        (int)pos.x + item_lt.x,
        (int)pos.y + item_lt.y,
        static_cast<float>(backgrnd_item->w),
        static_cast<float>(backgrnd_item->h),
    };
    SDL_RenderTexture(window::renderer, backgrnd_item, nullptr, &pos_rect);
  }
}

void character_info_ui_system::render_button() {
  std::vector<wz::Node *> buttons_nodes = {
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:party"),
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:trade"),
      wz_resource::ui->find(
          u"CharacterInfo.img/RemoteInfo/button:toggleItemlist"),
      wz_resource::ui->find(
          u"CharacterInfo.img/RemoteInfo/button:showCitizenship"),
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:upPop"),
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:downPop"),
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:showPet"),
      wz_resource::ui->find(u"Basic.img/BtClose"),
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
        wz_resource::ui->find(u"CharacterInfo.img/ItemList/button:hide"));
    buttons_rect.push_back(SDL_FRect{227 + item_lt.x, 7 + item_lt.y, 12, 12});
  }

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = cursor_game_instance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
        !cursor_game_instance::modal_overlay) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = wz_resource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = wz_resource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = wz_resource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void character_info_ui_system::render_character() {
  auto &camera = camera_game_instance::camera;
  character.pos.x = int(camera.x + pos.x + 68);
  character.pos.y = int(camera.y + pos.y + 119);
  character_render_system::render_character(character);
}

void character_info_ui_system::render_text() {
  freetype::load_aligned(true);
  freetype::load_size(12);
  const auto label_offset = -43;
  // name
  auto charname = character.name;
  auto w = freetype::load_w(charname);
  freetype::load_color(255, 255, 255, 255);
  auto x = int(pos.x + 67 - w / 2);
  freetype::draw_line(charname, x, int(pos.y + 127));

  freetype::load_color(0, 0, 0, 255);
  // level
  auto level = character.level;
  std::string level2 = std::to_string(level);
  std::u16string level3{level2.begin(), level2.end()};
  x = int(pos.x + 208);
  freetype::draw_line(level3, x, int(pos.y + 53));
  x += label_offset;
  std::u16string str = u"LEVEL";
  w = freetype::load_w(str);
  freetype::load_color(255, 255, 255, 255);
  freetype::draw_line(str, x - w / 2, int(pos.y + 53));

  freetype::load_color(0, 0, 0, 255);
  // job
  auto job_node =
      wz_resource::ms->get_root()->find(u"String.img/Job/" + character.job);
  auto job_name = static_cast<wz::Property<std::u16string> *>(job_node)->get();
  freetype::draw_line(job_name, int(pos.x + 208), int(pos.y + 72));
  str = u"JOB";
  w = freetype::load_w(str);
  freetype::load_color(255, 255, 255, 255);
  freetype::draw_line(str, x - w / 2, int(pos.y + 72));

  freetype::load_color(0, 0, 0, 255);
  auto fame = character.fame;
  std::string fame2 = std::to_string(fame);
  std::u16string fame3{fame2.begin(), fame2.end()};
  freetype::draw_line(fame3, int(pos.x + 208), int(pos.y + 91));
  str = u"FAME";
  w = freetype::load_w(str);
  freetype::load_color(255, 255, 255, 255);
  freetype::draw_line(str, x - w / 2, int(pos.y + 91));
}

void character_info_ui_system::render_items() {
  if (!item) {
    return;
  }
  const auto line_h = 38;
  auto w = 9;
  auto h = 31;
  auto v = equip_game_instance::load_equips(character);
  auto v2 = equip_game_instance::load_decos(character);
  for (uint8_t i = item_page; i < item_page + 3; i++) {
    if (i >= v.size() + v2.size()) {
      break;
    }
    std::u16string itm_id;
    if (i < v.size()) {
      itm_id = v[i].id;
    } else {
      itm_id = v2[i - v.size()].id;
    }

    static auto entry_back = wz_resource::load_texture(
        wz_resource::ui->find(u"CharacterInfo.img/ItemListEntry/backgrnd"));

    SDL_FRect pos_rect = {
        static_cast<float>(int(pos.x + item_lt.x + w)),
        static_cast<float>(
            int(pos.y + item_lt.y + h + line_h * (i - item_page))),
        static_cast<float>(entry_back->w),
        static_cast<float>(entry_back->h),
    };
    SDL_RenderTexture(window::renderer, entry_back, nullptr, &pos_rect);

    auto info = equip_game_instance::load_equip_info(itm_id);
    auto texture = wz_resource::load_texture(info->get_child(u"icon"));
    pos_rect = {
        static_cast<float>(int(pos.x + item_lt.x + w + (35 - texture->w) / 2)),
        static_cast<float>(int(pos.y + item_lt.y + h +
                               line_h * (i - item_page) +
                               (line_h - texture->h) / 2)),
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);

    freetype::load_color(0, 0, 0, 255);

    auto name = equip_game_instance::load_equip_name(itm_id);
    if (name.size() > 22) {
      name = name.substr(0, 22) + u"...";
    }
    freetype::load_aligned(true);
    freetype::load_size(12);
    auto x = pos.x + item_lt.x + w + 43;
    auto y = pos.y + item_lt.y + h + line_h * (i - item_page) - 1;
    freetype::draw_line(name, x, y);

    x = pos.x + item_lt.x + w + 43;
    y = pos.y + item_lt.y + h + line_h * (i - item_page) + 17;
    freetype::draw_line(u"Level", x, y);

    auto equip_info = equip_game_instance::load_equip_info(itm_id);
    auto req =
        static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLevel"))
            ->get();
    std::string req2 = std::to_string(req);
    std::u16string req3{req2.begin(), req2.end()};
    x = pos.x + item_lt.x + w + 79;
    y = pos.y + item_lt.y + h + line_h * (i - item_page) + 17;
    freetype::draw_line(req3, x, y);

    auto cursor_in = cursor_game_instance::cursor_ui;
    if (cursor_in != render) {
      continue;
    }
    auto &mouse_pos = window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      if (i < v.size()) {
        item_info = v[i];
      } else {
        item_info = v2[i - v.size()];
      }
    }
  }
}

bool character_info_ui_system::render_items_info() {
  if (item_info.has_value()) {
    auto &mouse_pos = window::mouse_pos;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    if (std::holds_alternative<game_equip_item>(item_info.value())) {
      // 是装备类型
      auto &equip = std::get<game_equip_item>(item_info.value());
      tooltip_ui_system::render_equip(equip, show_pos.x, show_pos.y);
    } else {
      auto &equip = std::get<game_deco_item>(item_info.value());
      tooltip_ui_system::render_deco(equip, show_pos.x, show_pos.y);
    }
  }
  item_info = std::nullopt;
  return true;
}

bool character_info_ui_system::render() {
  render_backgrnd();
  render_scroll();
  render_button();
  render_character();
  render_text();
  render_items();
  return true;
}

bool character_info_ui_system::event_vscr(SDL_Event *event) {
  if (!item) {
    return false;
  }
  const SDL_FPoint lt{219 + item_lt.x, 32 + item_lt.y};
  const uint32_t length = 115;
  auto v = equip_game_instance::load_equips(character).size();
  auto v2 = equip_game_instance::load_decos(character).size();
  int size = v + v2 - 3;
  auto cursor_in = cursor_game_instance::cursor_ui;
  bool top = cursor_in == render;
  size = std::max(0, size);
  auto mouse_pos = SDL_FPoint{event->button.x, event->button.y};
  auto val =
      scroll_ui_system::click_vscroll((int)pos.x + lt.x, (int)pos.y + lt.y,
                                      item_page, size, length, top, mouse_pos);
  item_page = val;
  return true;
}

void character_info_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    item = false;
    pet = false;

    character_logic_system::run_stand_action(character);
    character_logic_system::run_face_action(character, u"default");
    character.flip = 0;

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

void character_info_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  std::erase(system::render_systems, render_items_info);
}

void character_info_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

void character_info_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void character_info_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void character_info_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void character_info_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool character_info_ui_system::cursor_in() {
  bool r = false;
  auto &mouse = window::mouse_pos;
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

void character_info_ui_system::event_button_party() {}

void character_info_ui_system::event_button_trade() {}

void character_info_ui_system::event_button_toggleitem() { item = true; }

void character_info_ui_system::event_button_toggleitem_hide() { item = false; }

void character_info_ui_system::event_button_citizenship() {}

void character_info_ui_system::event_button_up_fame() {}

void character_info_ui_system::event_button_dowm_fame() {}

void character_info_ui_system::event_button_show_pet() {}

void character_info_ui_system::event_close() { close(); }

bool character_info_ui_system::event_button(SDL_Event *event) {
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
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      audio_game_instance::load_audio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }

  return false;
}

void character_info_ui_system::event_vscr_start(SDL_Event *event) {
  const SDL_FPoint lt{219 + item_lt.x, 32 + item_lt.y};
  const uint32_t length = 115;
  if (vscr_motion == false) {
    vscr_motion =
        scroll_ui_system::click_thumb(pos.x + lt.x, pos.y + lt.y, length);
  }
}

void character_info_ui_system::event_vscr_end() { vscr_motion = false; }

void character_info_ui_system::event_vscr_move(SDL_Event *event) {
  auto mouse_state = window::mouse_state;
  if (vscr_motion) {
    const SDL_FPoint lt{219 + item_lt.x, 32 + item_lt.y};
    const uint32_t length = 115;

    event->button.x = pos.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos.y + lt.y, pos.y + lt.y + length);
    event_vscr(event);
  }
}

void character_info_ui_system::event_motion_bt_audio(SDL_Event *event) {
  if (cursor_game_instance::cursor_ui != render) {
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
  auto &mouse_pos = window::mouse_pos;
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
    audio_game_instance::load_audio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

bool character_info_ui_system::event(SDL_Event *event) {
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
      if (cursor_game_instance::cursor_ui == render) {
        event_top();
        event_drag_start(event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
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
    auto dy = event->wheel.integer_y;
    if (dy > 0) {
      // up
      if (item_page > 0) {
        item_page -= 1;
      }
    } else {
      // down
      auto v = equip_game_instance::load_equips(character).size();
      auto v2 = equip_game_instance::load_decos(character).size();
      auto size = v + v2;
      if (size - 3 < 0) {
        break;
      }
      if (item_page < size - 3) {
        item_page += 1;
      }
    }
    return false;
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

void character_info_ui_system::event_motion(SDL_Event *event) {
  auto &sys = system::render_systems;
  std::erase(sys, render_items_info);
  auto it = std::ranges::find(sys, &cursor_render_system::render);
  if (it != sys.end()) {
    sys.insert(it, render_items_info);
  }
}