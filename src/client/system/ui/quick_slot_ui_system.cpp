#include "quick_slot_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <flat_map>
#include <format>


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
        wz_resource::ui->find(u"QuickSlot.img/backgrnd"));
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

SDL_FPoint quick_slot_ui_system::load_wh() { return {286, 146}; }

void quick_slot_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

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
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}


bool quick_slot_ui_system::event_click_quickslot(SDL_Event *event) {
  if (cursor_game_instance::cursor_hand_net.has_value()) {
    return false;
  }
  switch (quickSlot) {
  case quick_slot::hide: {
    break;
  }
  case quick_slot::two: {
    static auto q = wz_resource::load_texture(
        wz_resource::ui->find(u"QuickSlot.img/backgrnd"));
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
          r[i].y + 34,
          static_cast<float>(32),
          static_cast<float>(32),
      };
      if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
        if (cursor_game_instance::cursor_hand.has_value()) {
          auto &cursor_hand = cursor_game_instance::cursor_hand.value();
          auto scan_code = t[i];
          switch (cursor_hand.type) {
          case cursor_game_instance::equipment: {
            break;
          }
          case cursor_game_instance::package: {
            break;
          }
          case cursor_game_instance::skill: {
            auto ski_id = std::format("{:07d}", cursor_hand.sub_val);
            auto &key_data = keyboard_game_instance::data;
            key_data[scan_code] = {
                .type = "skill",
                .val = ski_id,
            };
            break;
          }
          case cursor_game_instance::keybind: {
            break;
          }
          }
        } else {
          auto &key_data = keyboard_game_instance::data;
          auto scan_code = t[i];
          if (key_data.contains(scan_code)) {
            auto &input = key_data.at(scan_code);
            if (input.type == "skill") {
              auto ski_id = std::stoi(input.val);
              cursor_game_instance::cursor_hand = {
                  .type = cursor_game_instance::skill,
                  .val = static_cast<uint32_t>(ski_id),
              };
              return true;
            }
          }
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
  bool r = false;
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