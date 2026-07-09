#include "skill_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "scroll_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "tooltip_ui_system.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

std::optional<std::u16string> skill_ui_system::load_mouse_ski() {
  auto cursor_in = cursor_game_instance::cursor_ui;
  if (cursor_in != render) {
    return std::nullopt;
  }
  const SDL_FPoint lt{8, 99};
  const SDL_FPoint rb{184, 334};
  const SDL_FPoint pos_icon{2, 2};
  const uint8_t max_scroll_num = 6;
  auto self_job = character_game_instance::self.job;
  // 根据active_tab获取技能组
  auto skill_node = wz_resource::skill->find(self_job + u".img");

  skill_node = skill_node->get_child(u"skill");
  auto &mouse_pos = window::mouse_pos;

  uint8_t i = 0;
  const auto ski_w = 32;
  const auto ski_h = 32;

  const auto entry_h = 35;
  const auto l =
      (rb.y - lt.y - max_scroll_num * entry_h) / (max_scroll_num - 1);
  for (auto [k, v] : *skill_node->get_children()) {
    if (i >= max_scroll_num) {
      break;
    }
    SDL_FRect pos_rect{
        pos.x + lt.x + 2,
        pos.y + lt.y + i * entry_h + l * i + 2,
        static_cast<float>(ski_w),
        static_cast<float>(ski_h),
    };
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      return k;
    }
    i++;
  }
  return std::nullopt;
}

SDL_FPoint skill_ui_system::load_wh() { return {197, 371}; }

uint8_t skill_ui_system::load_skill_num() {
  auto self_job = character_game_instance::self.job;
  auto skill_node = wz_resource::skill->find(self_job + u".img/skill");
  return skill_node->children_count();
}

bool skill_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void skill_ui_system::open() {
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

void skill_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  event_drag_end();
}

void skill_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void skill_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void skill_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void skill_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void skill_ui_system::render_backgrnd() {
  static auto back_texture =
      wz_resource::load_texture(wz_resource::ui->find(u"Skill.img/backgrnd"));
  SDL_FRect pos_rect = {
      pos.x,
      pos.y,
      (float)back_texture->w,
      (float)back_texture->h,
  };
  SDL_RenderTexture(window::renderer, back_texture, nullptr, &pos_rect);
}

void skill_ui_system::render_tab() {
  const static SDL_FPoint lt = {5, 24};
  const static SDL_FPoint rb = {186, 43};
  const static auto tab_node = wz_resource::ui->find(u"Skill.img/tab:grade");
  const static std::array selected_texture = {
      wz_resource::load_texture(tab_node->find(u"selected/0")),
      wz_resource::load_texture(tab_node->find(u"selected/1")),
      wz_resource::load_texture(tab_node->find(u"selected/2")),
      wz_resource::load_texture(tab_node->find(u"selected/3")),
      wz_resource::load_texture(tab_node->find(u"selected/4")),
  };
  const static std::array normal_texture = {
      wz_resource::load_texture(tab_node->find(u"normal/0")),
      wz_resource::load_texture(tab_node->find(u"normal/1")),
      wz_resource::load_texture(tab_node->find(u"normal/2")),
      wz_resource::load_texture(tab_node->find(u"normal/3")),
      wz_resource::load_texture(tab_node->find(u"normal/4")),
  };
  for (uint8_t i = 0; i < selected_texture.size(); i++) {
    SDL_Texture *t = active_tab == i ? selected_texture[i] : normal_texture[i];
    SDL_FPoint tab_pos = {static_cast<float>(lt.x + i * 33), lt.y};
    SDL_FRect pos_rect{static_cast<float>(int(pos.x + tab_pos.x)),
                       static_cast<float>(int(pos.y + tab_pos.y)),
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  }
}

void skill_ui_system::render_skill_entry() {

  static auto entry = wz_resource::load_texture(
      wz_resource::ui->find(u"Skill.img/entry/skill1"));

  const SDL_FPoint lt{8, 99};
  const SDL_FPoint rb{184, 334};
  const SDL_FPoint pos_icon{2, 2};
  const uint8_t max_scroll_num = 6;
  const auto l =
      (rb.y - lt.y - max_scroll_num * entry->h) / (max_scroll_num - 1);
  auto self_job = character_game_instance::self.job;
  // 根据active_tab获取技能组
  auto skill_group = std::pow(10, active_tab);
  auto skill_node = wz_resource::skill->find(self_job + u".img");

  auto book_texture = wz_resource::load_texture(skill_node->find(u"info/icon"));
  SDL_FRect pos_rect{pos.x + 15, pos.y + 57,
                     static_cast<float>(book_texture->w),
                     static_cast<float>(book_texture->h)};
  SDL_RenderTexture(window::renderer, book_texture, nullptr, &pos_rect);

  skill_node = skill_node->get_child(u"skill");
  auto &mouse_pos = window::mouse_pos;
  // 判断按钮是否被遮挡
  auto cursor_in = cursor_game_instance::cursor_ui;
  uint8_t i = 0;
  for (auto [k, v] : *skill_node->get_children()) {
    if (i >= max_scroll_num) {
      break;
    }
    // render backgrnd

    // SDL_RenderTexture(window::renderer, line, nullptr, &pos_rect);

    SDL_FRect pos_rect{pos.x + lt.x, pos.y + lt.y + i * entry->h + l * i,
                       static_cast<float>(entry->w),
                       static_cast<float>(entry->h)};
    SDL_RenderTexture(window::renderer, entry, nullptr, &pos_rect);

    auto skl_id = std::string{k.begin(), k.end()};
    auto skl_id2 = std::stoi(skl_id);

    SDL_Texture *ski_texture;
    auto ski_level = job_skill_game_instance::load_skill_level(k);
    if (ski_level > 0) {
      if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
          !cursor_game_instance::modal_overlay) {
        ski_texture =
            wz_resource::load_texture(v[0]->get_child(u"iconMouseOver"));
      } else {
        ski_texture = wz_resource::load_texture(v[0]->get_child(u"icon"));
      }
    } else {
      // disable
      ski_texture = wz_resource::load_texture(v[0]->get_child(u"iconDisabled"));
    }
    pos_rect = {pos.x + lt.x + 2, pos.y + lt.y + i * entry->h + l * i + 2,
                static_cast<float>(ski_texture->w),
                static_cast<float>(ski_texture->h)};
    SDL_RenderTexture(window::renderer, ski_texture, nullptr, &pos_rect);
    // render ski_name
    auto ski_name = skill_game_instance::load_ski_name(k);
    freetype::load_color(0, 0, 0, 255);
    freetype::load_aligned(true);
    freetype::load_size(12);
    freetype::draw_line(ski_name.name, pos.x + lt.x + 40, pos_rect.y - 3);
    auto ski_level2 = std::to_string(ski_level);
    auto ski_level3 = std::u16string{ski_level2.begin(), ski_level2.end()};
    freetype::draw_line(ski_level3, pos.x + lt.x + 40, pos_rect.y + 15);

    freetype::load_aligned(false);
    i++;
  }
}

void skill_ui_system::render_scroll() {
  const SDL_FPoint lt{174, 98};
  const uint32_t length = 236;
  auto size = 6;
  auto cursor_in = cursor_game_instance::cursor_ui;
  bool top = (cursor_in == render) && !cursor_game_instance::modal_overlay;
  scroll_ui_system::render_vscroll((int)pos.x + lt.x, (int)pos.y + lt.y, page,
                                   size, length, top);
  return;
}

void skill_ui_system::render_info() {
  auto mouse_ski = load_mouse_ski();
  if (mouse_ski.has_value() && !cursor_game_instance::modal_overlay) {
    auto &mouse_pos = window::mouse_pos;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    auto ski_id = mouse_ski.value();
    tooltip_ui_system::render_skill(ski_id, 1, show_pos.x, show_pos.y);
  }
}

void skill_ui_system::render_button() {
  std::vector<wz::Node *> buttons_nodes = {
      wz_resource::ui->find(u"Basic.img/BtClose"),
  };
  auto wh = load_wh();
  std::vector<SDL_FRect> buttons_rect = {
      {wh.x - 20, 8, 12, 12}, //
  };

  auto self_job = character_game_instance::self.job;
  auto skill_node = wz_resource::skill->find(self_job + u".img");
  const SDL_FPoint lt{8, 99};
  const SDL_FPoint rb{184, 334};
  const SDL_FPoint pos_icon{2, 2};
  const uint8_t max_scroll_num = 6;
  const auto ski_w = 32;
  const auto ski_h = 32;

  const auto entry_h = 35;
  const auto l =
      (rb.y - lt.y - max_scroll_num * entry_h) / (max_scroll_num - 1);

  skill_node = skill_node->get_child(u"skill");
  int i = 0;
  for (auto [k, v] : *skill_node->get_children()) {
    if (i >= max_scroll_num) {
      break;
    }
    SDL_FRect pos_rect{
        lt.x + 146,
        lt.y + i * entry_h + l * i + 22,
        12,
        12,
    };
    buttons_nodes.push_back(wz_resource::ui->find(u"Basic.img/BtUP"));
    buttons_rect.push_back(pos_rect);
    i++;
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

void skill_ui_system::render_book() {
  auto self_job = character_game_instance::self.job;
  auto icon_node = wz_resource::skill->find(self_job + u".img/info/icon");
  auto t = wz_resource::load_texture(icon_node);
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);

  auto book_node =
      wz_resource::string->find(u"Skill.img/" + self_job + u"/bookName");
  auto book_name =
      static_cast<wz::Property<std::u16string> *>(book_node)->get();
  freetype::load_aligned(true);
  freetype::load_size(13);
  freetype::load_color(0, 0, 0, 255);
  freetype::draw_line(book_name, pos.x, pos.y);
  freetype::load_aligned(false);
}

bool skill_ui_system::render() {
  render_backgrnd();
  render_skill_entry();
  render_button();
  render_scroll();
  render_tab();
  render_info();
  return true;
}

void skill_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

void skill_ui_system::event_close() { close(); }

bool skill_ui_system::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto wh = load_wh();
  r = {
      SDL_FRect{wh.x - 20, 7, 12, 12}, //
  };
  fns = {
      event_close,
  };

  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

bool skill_ui_system::event_click_ski(SDL_Event *event) {
  if (cursor_game_instance::cursor_hand.has_value()) {
    return false;
  }
  auto index = load_mouse_ski();
  if (index.has_value()) {
    if (event->button.clicks >= 2) {

    } else {
      auto val = std::string{index.value().begin(), index.value().end()};
      auto sub_val = std::stoi(val);
      cursor_game_instance::cursor_hand = {
          .type = cursor_game_instance::skill,
          .val = active_tab,
          .sub_val = static_cast<uint32_t>(sub_val),
      };
    }
    return true;
  }
  return false;
}

bool skill_ui_system::event(SDL_Event *event) {
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
        event_click_ski(event);
        r = event_button(event);
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