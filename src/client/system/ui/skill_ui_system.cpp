#include "skill_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>

SDL_FPoint skill_ui_system::load_wh() { return {197, 371}; }

uint8_t skill_ui_system::load_skill_num() {
  auto self_job = job_skill_game_instance::self_job;
  auto skill_node =
      wz_resource::skill->find(std::to_string(self_job) + ".img/skill");
  return skill_node->children_count();
}

bool skill_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void skill_ui_system::open() {

  auto wh = load_wh();
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void skill_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  event_drag_end();
}

void skill_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void skill_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 18};
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
  static auto back_node = wz_resource::ui->find(u"Skill.img/backgrnd");
  static auto back_texture = wz_resource::load_texture(back_node);
  SDL_FRect pos_rect = {pos.x, pos.y, (float)back_texture->w,
                        (float)back_texture->h};
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
    SDL_FRect pos_rect{pos.x + tab_pos.x, pos.y + tab_pos.y,
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  }
}

void skill_ui_system::render_skill_entry() {
  const SDL_FPoint lt{8, 99};
  const SDL_FPoint rb{184, 334};
  const SDL_FPoint pos_icon{2, 2};
  const uint8_t max_scroll_num = 6;
  auto self_job = job_skill_game_instance::self_job;
  // 根据active_tab获取技能组
  auto skill_group = std::pow(10, active_tab);
  auto skill_node =
      wz_resource::skill->find(std::to_string(self_job) + ".img/skill");

  auto &mouse_pos = window::mouse_pos;
  // 判断按钮是否被遮挡
  auto cursor_in = cursor_game_instance::cursor_ui;
  const auto &self_skill_point = job_skill_game_instance::self_skill_point;
  uint8_t i = 0;
  for (auto [k, v] : *skill_node->get_children()) {
    if (i >= max_scroll_num) {
      break;
    }
    // render backgrnd
    static auto line =
        wz_resource::load_texture(wz_resource::ui->find(u"Skill.img/line"));
    static auto backgrnd = wz_resource::load_texture(
        wz_resource::ui->find(u"Skill.img/entry/skill1"));
    SDL_FRect pos_rect{pos.x + lt.x, pos.y + lt.y + i * backgrnd->h - 1,
                       static_cast<float>(line->w),
                       static_cast<float>(line->h)};
    SDL_RenderTexture(window::renderer, line, nullptr, &pos_rect);

    pos_rect = {pos.x + lt.x, pos.y + lt.y + i * backgrnd->h,
                static_cast<float>(backgrnd->w),
                static_cast<float>(backgrnd->h)};
    SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);

    auto skl_id = std::string{k.begin(), k.end()};
    auto skl_id2 = std::stoi(skl_id);

    if (self_skill_point.contains(skl_id2)) {
      if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render) {
      } else {
      }
    } else {
      // disable
    }
    i++;
  }
}

void skill_ui_system::render_scroll() {
  static auto prev0 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VSr4/enabled/prev0"));
  static auto prev1 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VSr4/enabled/prev1"));
  static auto prev2 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VSr4/enabled/prev2"));
  static auto next0 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VSr4/enabled/next0"));
  static auto next1 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VSr4/enabled/next1"));
  static auto next2 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VSr4/enabled/next2"));
  static auto base0 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VSr4/enabled/base"));
  static auto thumb0 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VSr4/enabled/thumb0"));
  static auto thumb1 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VSr4/enabled/thumb1"));
  static auto thumb2 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VSr4/enabled/thumb2"));
  static auto prev = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VSr4/disabled/prev"));
  static auto next = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VSr4/disabled/next"));
  static auto base = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VSr4/disabled/base"));
  const SDL_FPoint lt{174, 98};
  const uint32_t length = 236;
  const uint8_t max_scroll_num = 6;

  if (max_scroll_num >= load_skill_num()) {
    // disable
    SDL_FRect pos_rect{pos.x + lt.x, pos.y + lt.y, static_cast<float>(prev->w),
                       static_cast<float>(prev->h)};
    SDL_RenderTexture(window::renderer, prev, nullptr, &pos_rect);

    pos_rect = {pos.x + lt.x, pos.y + lt.y + length - next->h,
                static_cast<float>(next->w), static_cast<float>(next->h)};
    SDL_RenderTexture(window::renderer, next, nullptr, &pos_rect);

    pos_rect = {pos.x + lt.x, pos.y + lt.y + prev->h,
                static_cast<float>(base->w),
                static_cast<float>(length - prev->h - next->h)};
    SDL_RenderTextureTiled(window::renderer, next, nullptr, 1, &pos_rect);
  } else {
    // 判断按钮是否被遮挡
    auto cursor_in = cursor_game_instance::cursor_ui;

    SDL_FRect pos_rect{pos.x + lt.x, pos.y + lt.y, static_cast<float>(prev->w),
                       static_cast<float>(prev->h)};
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect) &&
        cursor_in == render) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {

      } else {
      }
    }
    SDL_RenderTexture(window::renderer, prev, nullptr, &pos_rect);
  }
  return;
}

bool skill_ui_system::render() {
  render_backgrnd();
  render_skill_entry();
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

bool skill_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
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