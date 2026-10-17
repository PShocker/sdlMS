#include "quick_slot_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game/game_input.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <flat_map>
#include <format>
#include <optional>
#include <string>

void quick_slot_ui_system::render_input_icon(const game_input &i, int x, int y,
                                             uint8_t alpha) {
  SDL_Texture *t;
  std::u16string id;
  if (i.type == "skill") {
    id = {i.val.begin(), i.val.end()};
    auto ski_info = skill_game_instance::load_ski_node(id);
    t = wz_resource::load_texture(ski_info->get_child(u"icon"));
  } else if (i.type == "item") {
    id = {i.val.begin(), i.val.end()};
    auto item_info = item_game_instance::load_item_info(id, 1);
    t = wz_resource::load_texture(item_info->get_child(u"icon"));
  } else if (i.type == "action") {
    const static std::flat_map<std::string, SDL_Texture *> actions = {
        {"pick", wz_resource::load_texture(
                     wz_resource::ui->find(u"KeyBindings.img/icon/50"))},
        {"sit", wz_resource::load_texture(
                    wz_resource::ui->find(u"KeyBindings.img/icon/51"))},
        {"attack", wz_resource::load_texture(
                       wz_resource::ui->find(u"KeyBindings.img/icon/52"))},
        {"jump", wz_resource::load_texture(
                     wz_resource::ui->find(u"KeyBindings.img/icon/53"))},
    };
    if (actions.contains(i.val)) {
      t = actions.at(i.val);
    }
  } else if (i.type == "ui") {
    const static std::flat_map<std::string, SDL_Texture *> uis = {
        {"equip", wz_resource::load_texture(
                          wz_resource::ui->find(u"KeyBindings.img/icon/0"))},
        {"package", wz_resource::load_texture(
                        wz_resource::ui->find(u"KeyBindings.img/icon/1"))},
        {"characterStat", wz_resource::load_texture(
                     wz_resource::ui->find(u"KeyBindings.img/icon/2"))},
        {"skill", wz_resource::load_texture(
                      wz_resource::ui->find(u"KeyBindings.img/icon/3"))},
        {"buddy", wz_resource::load_texture(
                      wz_resource::ui->find(u"KeyBindings.img/icon/4"))},
        {"worldMap", wz_resource::load_texture(
                         wz_resource::ui->find(u"KeyBindings.img/icon/5"))},
        {"mapleChat", wz_resource::load_texture(
                          wz_resource::ui->find(u"KeyBindings.img/icon/6"))},
        {"miniMap", wz_resource::load_texture(
                        wz_resource::ui->find(u"KeyBindings.img/icon/7"))},
        {"quest", wz_resource::load_texture(
                      wz_resource::ui->find(u"KeyBindings.img/icon/8"))},
        {"keyBind", wz_resource::load_texture(
                        wz_resource::ui->find(u"KeyBindings.img/icon/9"))},
        {"toAll", wz_resource::load_texture(
                      wz_resource::ui->find(u"KeyBindings.img/icon/10"))},
        {"whisper", wz_resource::load_texture(
                        wz_resource::ui->find(u"KeyBindings.img/icon/11"))},
        {"toParty", wz_resource::load_texture(
                        wz_resource::ui->find(u"KeyBindings.img/icon/12"))},
        {"toBuddy", wz_resource::load_texture(
                        wz_resource::ui->find(u"KeyBindings.img/icon/13"))},
        {"shortCut", wz_resource::load_texture(
                         wz_resource::ui->find(u"KeyBindings.img/icon/14"))},
        {"quickSlot", wz_resource::load_texture(
                          wz_resource::ui->find(u"KeyBindings.img/icon/15"))},
        {"chat+", wz_resource::load_texture(
                      wz_resource::ui->find(u"KeyBindings.img/icon/16"))},
        {"party", wz_resource::load_texture(
                      wz_resource::ui->find(u"KeyBindings.img/icon/19"))},
        {"cashShop", wz_resource::load_texture(
                         wz_resource::ui->find(u"KeyBindings.img/icon/22"))},
        {"changeChannel", wz_resource::load_texture(wz_resource::ui->find(
                              u"KeyBindings.img/icon/45"))},
        {"menu", wz_resource::load_texture(
                     wz_resource::ui->find(u"KeyBindings.img/icon/46"))},
        {"screenShot", wz_resource::load_texture(
                           wz_resource::ui->find(u"KeyBindings.img/icon/47"))},
        {"picMode", wz_resource::load_texture(
                        wz_resource::ui->find(u"KeyBindings.img/icon/48"))},

    };
    if (uis.contains(i.val)) {
      t = uis.at(i.val);
    }
  }
  if (t == nullptr) {
    return;
  }
  SDL_FRect pos_rect{
      static_cast<float>(x),
      static_cast<float>(y),
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  if (i.type == "item") {
    pos_rect.y += (32 - pos_rect.h) / 2;
  }
  SDL_SetTextureAlphaMod(t, alpha);
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  SDL_SetTextureAlphaMod(t, 255);
  if (i.type == "item") {
    auto num = package_game_instance::load_item_num(id);
    package_ui_system::render_number(num, x, y + 16);
  }
}

void quick_slot_ui_system::render_quickSlot() {
  const static std::flat_map<SDL_Scancode, SDL_Texture *> textures = {
      {SDL_SCANCODE_PAGEDOWN, wz_resource::load_texture(wz_resource::ui->find(
                                  u"QuickSlot.img/key/81"))},
      {SDL_SCANCODE_END, wz_resource::load_texture(
                             wz_resource::ui->find(u"QuickSlot.img/key/79"))},
      {SDL_SCANCODE_PAGEUP, wz_resource::load_texture(wz_resource::ui->find(
                                u"QuickSlot.img/key/73"))},
      {SDL_SCANCODE_HOME, wz_resource::load_texture(
                              wz_resource::ui->find(u"QuickSlot.img/key/71"))},
      {SDL_SCANCODE_INSERT, wz_resource::load_texture(wz_resource::ui->find(
                                u"QuickSlot.img/key/82"))},
      {SDL_SCANCODE_DELETE, wz_resource::load_texture(wz_resource::ui->find(
                                u"QuickSlot.img/key/83"))},
      {SDL_SCANCODE_LCTRL, wz_resource::load_texture(
                               wz_resource::ui->find(u"QuickSlot.img/key/29"))},
      {SDL_SCANCODE_LSHIFT, wz_resource::load_texture(wz_resource::ui->find(
                                u"QuickSlot.img/key/42"))},
  };
  switch (quickSlot) {
  case quick_slot::hide: {
    break;
  }
  case quick_slot::two: {
    static auto q = wz_resource::load_texture(
        wz_resource::ui->find(u"QuickSlot.img/layer:backgrnd"));
    auto screen_w = camera_game_instance::camera.w;
    auto screen_h = camera_game_instance::camera.h;
    auto base_x = (screen_w - 808) / 2;
    auto base_y = (screen_h - 73);
    SDL_FRect p{
        base_x + 654,
        base_y - 107,
        static_cast<float>(q->w),
        static_cast<float>(q->h),
    };
    SDL_RenderTexture(window::renderer, q, nullptr, &p);
    std::vector<SDL_Scancode> scode = {
        (SDL_SCANCODE_LSHIFT), (SDL_SCANCODE_INSERT),   (SDL_SCANCODE_HOME),
        (SDL_SCANCODE_PAGEUP), (SDL_SCANCODE_LCTRL),    (SDL_SCANCODE_DELETE),
        (SDL_SCANCODE_END),    (SDL_SCANCODE_PAGEDOWN),
    };
    std::vector<SDL_FPoint> r = {
        SDL_FPoint{p.x + 9, p.y + 10},
        SDL_FPoint{p.x + 44, p.y + 10},
        SDL_FPoint{p.x + 79, p.y + 10},
        SDL_FPoint{p.x + 114, p.y + 10},
        // 2row
        SDL_FPoint{p.x + 9, p.y + 44},
        SDL_FPoint{p.x + 44, p.y + 44},
        SDL_FPoint{p.x + 79, p.y + 44},
        SDL_FPoint{p.x + 114, p.y + 44},
    };
    for (int i = 0; i < r.size(); i++) {
      SDL_FRect pos_rect;
      if (keyboard_game_instance::data.contains(scode[i])) {
        const auto &input = keyboard_game_instance::data.at(scode[i]);
        render_input_icon(input, r[i].x - 2, r[i].y + 32);
      }
      auto texture = textures.at(scode[i]);
      pos_rect = {
          r[i].x,
          r[i].y + 34,
          static_cast<float>(texture->w),
          static_cast<float>(texture->h),
      };
      SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
    }

    break;
  }
  case quick_slot::three: {
    break;
  }
  }
}

bool quick_slot_ui_system::render() {
  render_quickSlot();
  return true;
}

SDL_FPoint quick_slot_ui_system::load_wh() {
  switch (quickSlot) {
  case quick_slot::hide:
    return {0, 0};
  case quick_slot::two:
    return {151, 78};
  case quick_slot::three:
    return {151, 114};
  }
  return {0, 0};
}

void quick_slot_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void quick_slot_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void quick_slot_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool quick_slot_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  base_x = base_x + 654;
  base_y = base_y - 107;
  SDL_FRect pos_rect{
      base_x,
      base_y,
      w,
      h,
  };
  if (quickSlot == quick_slot::two) {
    pos_rect.y += 36;
  }
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool quick_slot_ui_system::event_click_quickslot(SDL_Event *event) {
  if (cursor_game_instance::cursor_hand_net.has_value()) {
    return false;
  }
  auto screen_w = camera_game_instance::camera.w;
  auto screen_h = camera_game_instance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  switch (quickSlot) {
  case quick_slot::hide: {
    break;
  }
  case quick_slot::two: {
    SDL_FPoint p{
        base_x + 654,
        base_y - 107,
    };
    std::vector<SDL_Scancode> t = {
        SDL_SCANCODE_LSHIFT, SDL_SCANCODE_INSERT,   SDL_SCANCODE_HOME,
        SDL_SCANCODE_PAGEUP, SDL_SCANCODE_LCTRL,    SDL_SCANCODE_DELETE,
        SDL_SCANCODE_END,    SDL_SCANCODE_PAGEDOWN,
    };
    std::vector<SDL_FPoint> r = {
        SDL_FPoint{p.x + 9, p.y + 10},
        SDL_FPoint{p.x + 44, p.y + 10},
        SDL_FPoint{p.x + 79, p.y + 10},
        SDL_FPoint{p.x + 114, p.y + 10},
        // 2row
        SDL_FPoint{p.x + 9, p.y + 44},
        SDL_FPoint{p.x + 44, p.y + 44},
        SDL_FPoint{p.x + 79, p.y + 44},
        SDL_FPoint{p.x + 114, p.y + 44},
    };
    for (int i = 0; i < r.size(); i++) {
      SDL_FRect pos_rect{
          r[i].x,
          r[i].y + 32,
          static_cast<float>(32),
          static_cast<float>(32),
      };
      if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
        if (cursor_game_instance::cursor_hand.has_value()) {
          auto &cursor_hand = cursor_game_instance::cursor_hand.value();
          auto scan_code = t[i];
          std::string id;
          switch (cursor_hand.type) {
          case cursor_game_instance::keybind: {
            auto input =
                keyboard_game_instance::data.at((SDL_Scancode)cursor_hand.val);
            if (input.type == "skill") {
              keyboard_game_instance::add_input(
                  scan_code, keyboard_game_instance::skill, input.val);
            } else if (input.type == "item") {
            }
            break;
          }
          case cursor_game_instance::package: {
            auto &item = package_game_instance::data[cursor_hand.val]
                                                    [cursor_hand.sub_val];
            id = std::string{item->id.begin(), item->id.end()};
            if (id.starts_with("02")) {
              if (id.starts_with("0204") || id.starts_with("0206") ||
                  id.starts_with("0207")) {
                break;
              }
              keyboard_game_instance::add_input(
                  scan_code, keyboard_game_instance::item, id);
            }
            break;
          }
          case cursor_game_instance::skill: {
            id = std::format("{:07d}", cursor_hand.sub_val);
            keyboard_game_instance::add_input(
                scan_code, keyboard_game_instance::skill, id);
            break;
          }
          default: {
            break;
          }
          }
          cursor_game_instance::cursor_hand = std::nullopt;
          audio_game_instance::load_audio(u"UI.img/DragEnd", 0);
        } else {
          auto &key_data = keyboard_game_instance::data;
          auto scan_code = t[i];
          if (key_data.contains(scan_code)) {
            auto &input = key_data.at(scan_code);
            cursor_game_instance::cursor_hand = {
                .type = cursor_game_instance::keybind,
                .val = scan_code,
            };
          }
          audio_game_instance::load_audio(u"UI.img/DragStart", 0);
        }
        return true;
      }
    }

    break;
  }
  case quick_slot::three: {
    break;
  }
  }
  return false;
}

bool quick_slot_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        r = !event_click_quickslot(event);
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    break;
  }
  default: {
    break;
  }
  }

  return r;
}