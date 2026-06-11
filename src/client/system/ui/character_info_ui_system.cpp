#include "character_info_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "scroll_ui_system.h"
#include "src/client/game/game_equip.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
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

std::vector<game_equip> character_info_ui_system::load_equips() {
  std::vector<game_equip> v;
  if (character.cap.has_value()) {
    v.push_back(character.cap.value());
  }
  if (character.cape.has_value()) {
    v.push_back(character.cape.value());
  }
  if (character.coat.has_value()) {
    v.push_back(character.coat.value());
  }
  if (character.longcoat.has_value()) {
    v.push_back(character.longcoat.value());
  }
  if (character.weapon.has_value()) {
    v.push_back(character.weapon.value());
  }
  if (character.pant.has_value()) {
    v.push_back(character.pant.value());
  }
  if (character.shield.has_value()) {
    v.push_back(character.shield.value());
  }
  if (character.shoes.has_value()) {
    v.push_back(character.shoes.value());
  }
  if (character.ring0.has_value()) {
    v.push_back(character.ring0.value());
  }
  if (character.ring1.has_value()) {
    v.push_back(character.ring1.value());
  }
  if (character.ring2.has_value()) {
    v.push_back(character.ring2.value());
  }
  if (character.ring3.has_value()) {
    v.push_back(character.ring3.value());
  }
  return v;
}

SDL_FPoint character_info_ui_system::load_wh() { return SDL_FPoint{277, 183}; }

const static SDL_FPoint item_lt{269, 0};

void character_info_ui_system::render_scroll() {
  if (!item) {
    return;
  }
  const SDL_FPoint lt{219 + item_lt.x, 32 + item_lt.y};
  const uint32_t length = 115;
  auto size = load_equips().size() - 3;
  auto cursor_in = cursor_game_instance::cursor_ui;
  bool top = cursor_in == render;
  scroll_ui_system::render_vscroll((int)pos.x + lt.x, (int)pos.y + lt.y,
                                   item_page, size, length, top);
  return;
}

void character_info_ui_system::render_backgrnd() {
  static auto backgrnd_top = wz_resource::load_texture(
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/backgrnd_top"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(backgrnd_top->w),
                     static_cast<float>(backgrnd_top->h)};
  SDL_RenderTexture(window::renderer, backgrnd_top, nullptr, &pos_rect);

  static auto backgrnd_bottom = wz_resource::load_texture(
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/backgrnd_bottom"));
  pos_rect = {pos.x, pos.y + backgrnd_top->h,
              static_cast<float>(backgrnd_bottom->w),
              static_cast<float>(backgrnd_bottom->h)};
  SDL_RenderTexture(window::renderer, backgrnd_bottom, nullptr, &pos_rect);

  if (item) {
    static auto backgrnd_item = wz_resource::load_texture(
        wz_resource::ui->find(u"CharacterInfo.img/ItemList/backgrnd"));
    pos_rect = {pos.x + item_lt.x, pos.y + item_lt.y,
                static_cast<float>(backgrnd_item->w),
                static_cast<float>(backgrnd_item->h)};
    SDL_RenderTexture(window::renderer, backgrnd_item, nullptr, &pos_rect);
  }
}

void character_info_ui_system::render_button() {
  std::vector<wz::Node *> buttons_nodes = {
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:party"),
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:trade"),
      wz_resource::ui->find(
          u"CharacterInfo.img/RemoteInfo/button:toggleItemlist"),
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:wishlist"),
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:upPop"),
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:downPop"),
      wz_resource::ui->find(u"CharacterInfo.img/RemoteInfo/button:showPet"),
      wz_resource::ui->find(u"Basic.img/BtClose"),
  };
  auto wh = load_wh();
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{117, 25, 80, 18},      //
      SDL_FRect{117, 45, 80, 18},      //
      SDL_FRect{210, 25, 60, 18},      //
      SDL_FRect{210, 45, 60, 18},      //
      SDL_FRect{240, 115, 12, 12},     //
      SDL_FRect{253, 115, 12, 12},     //
      SDL_FRect{188, 158, 81, 18},     //
      SDL_FRect{wh.x - 20, 8, 12, 12}, //
  };
  if (item) {
    buttons_nodes.push_back(
        wz_resource::ui->find(u"CharacterInfo.img/ItemList/button:hide"));
    buttons_rect.push_back(SDL_FRect{220 + item_lt.x, 7 + item_lt.y, 15, 14});
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
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render) {
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
  character.pos.x = int(camera.x + pos.x + 57);
  character.pos.y = int(camera.y + pos.y + 122);
  character_render_system::render_character(character);
}

void character_info_ui_system::render_text() {
  freetype::load_aligned(true);
  freetype::load_size(12);
  auto charname = character.nametags[0].text;
  auto w = freetype::load_w(charname);
  freetype::load_color(255, 255, 255, 255);
  freetype::draw_line(charname, int(pos.x + 53 - w / 2), int(pos.y + 127));

  auto level = character.level;
  std::string level2 = std::to_string(level);
  std::u16string level3{level2.begin(), level2.end()};
  freetype::load_color(0, 0, 0, 255);
  freetype::draw_line(level3, int(pos.x + 165), int(pos.y + 77));

  // job
  auto job_node =
      wz_resource::string2->get_root()->find(u"JobName.img/" + character.job);
  auto job_name = static_cast<wz::Property<std::u16string> *>(job_node)->get();
  freetype::load_color(0, 0, 0, 255);
  freetype::draw_line(job_name, int(pos.x + 165), int(pos.y + 95));

  auto fame = character.fame;
  std::string fame2 = std::to_string(fame);
  std::u16string fame3{fame2.begin(), fame2.end()};
  freetype::load_color(0, 0, 0, 255);
  freetype::draw_line(fame3, int(pos.x + 165), int(pos.y + 113));

  freetype::load_aligned(false);
}

void character_info_ui_system::render_items() {
  if (!item) {
    return;
  }
  auto w = 16;
  auto h = 32;
  auto v = load_equips();
  for (uint8_t i = item_page; i < item_page + 3; i++) {
    if (i >= v.size()) {
      break;
    }
    static auto entry_back = wz_resource::load_texture(
        wz_resource::ui->find(u"CharacterInfo.img/ItemListEntry/backgrnd"));

    SDL_FRect pos_rect = {
        static_cast<float>(int(pos.x + item_lt.x + w)),
        static_cast<float>(int(pos.y + item_lt.y + h + 40 * (i - item_page))),
        static_cast<float>(entry_back->w), static_cast<float>(entry_back->h)};
    SDL_RenderTexture(window::renderer, entry_back, nullptr, &pos_rect);

    auto info = equip_game_instance::load_equip_info(v[i].id);
    auto texture = wz_resource::load_texture(info->get_child(u"icon"));
    pos_rect = {
        static_cast<float>(int(pos.x + item_lt.x + w + (35 - texture->w) / 2)),
        static_cast<float>(int(pos.y + item_lt.y + h + 40 * (i - item_page) +
                               (40 - texture->h) / 2)),
        static_cast<float>(texture->w), static_cast<float>(texture->h)};
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);

    auto name = equip_game_instance::load_equip_name(v[i].id);
    freetype::load_aligned(true);
    freetype::load_size(12);
    auto x = pos.x + item_lt.x + w + 42;
    auto y = pos.y + item_lt.y + h + 40 * (i - item_page) - 1;
    freetype::draw_line(name, x, y);

    auto equip_info = equip_game_instance::load_equip_info(v[i].id);
    auto req =
        static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLevel"))
            ->get();
    std::string req2 = std::to_string(req);
    std::u16string req3{req2.begin(), req2.end()};
    x += 40;
    y += 18;
    freetype::draw_line(req3, x, y);

    freetype::load_aligned(false);

    auto cursor_in = cursor_game_instance::cursor_ui;
    if (cursor_in != render) {
      continue;
    }
    auto &mouse_pos = window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      item_info = v[i];
    }
  }
}

void character_info_ui_system::render_items_info() {
  if (item_info.has_value()) {
    auto &mouse_pos = window::mouse_pos;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    tooltip_ui_system::render_equip(item_info.value(), show_pos.x, show_pos.y);
  }
  item_info = std::nullopt;
}

bool character_info_ui_system::render() {
  render_backgrnd();
  render_scroll();
  render_button();
  render_character();
  render_text();
  render_items();
  render_items_info();
  return true;
}

void character_info_ui_system::event_click_scroll() {
  if (!item) {
    return;
  }
  const SDL_FPoint lt{219 + item_lt.x, 32 + item_lt.y};
  const uint32_t length = 115;
  auto size = load_equips().size() - 3;
  auto cursor_in = cursor_game_instance::cursor_ui;
  bool top = cursor_in == render;
  auto val = scroll_ui_system::click_vscroll(
      (int)pos.x + lt.x, (int)pos.y + lt.y, item_page, size, length, top);
  item_page = val;
}

void character_info_ui_system::open() {
  auto wh = load_wh();
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;

  item = false;
  pet = false;

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.begin(), event);
}

void character_info_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void character_info_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.begin(), event);
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
  SDL_FRect pos_rect{pos.x, pos.y, 277, 183};
  r = SDL_PointInRectFloat(&mouse, &pos_rect);
  if (r) {
    return r;
  }
  if (item) {
    pos_rect = {pos.x + item_lt.x, pos.y + item_lt.y, 242, 164};
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

void character_info_ui_system::event_button_wishlist() {}

void character_info_ui_system::event_button_up_fame() {}

void character_info_ui_system::event_button_dowm_fame() {}

void character_info_ui_system::event_button_show_pet() {}

void character_info_ui_system::event_close() { close(); }

bool character_info_ui_system::event_button(SDL_Event *event) {
  std::vector<void (*)()> fns;
  auto wh = load_wh();
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{117, 25, 80, 18},      //
      SDL_FRect{117, 45, 80, 18},      //
      SDL_FRect{210, 25, 60, 18},      //
      SDL_FRect{210, 45, 60, 18},      //
      SDL_FRect{240, 115, 12, 12},     //
      SDL_FRect{253, 115, 12, 12},     //
      SDL_FRect{188, 158, 81, 18},     //
      SDL_FRect{wh.x - 20, 8, 12, 12}, //
  };
  fns = {
      event_button_party,    event_button_trade,   event_button_toggleitem,
      event_button_wishlist, event_button_up_fame, event_button_dowm_fame,
      event_button_show_pet, event_close,
  };

  if (item) {
    buttons_rect.push_back(SDL_FRect{220 + item_lt.x, 7 + item_lt.y, 15, 14});
    fns.push_back(event_button_toggleitem_hide);
  }

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
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
        event_click_scroll();
        r = !event_button(event);
      }
      event_drag_end();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_drag_move(event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}