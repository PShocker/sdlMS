#include "skill_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "scroll_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
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
  auto ski_tree = job_skill_game_instance::load_ski_tree(self_job);
  job_type jt = ski_tree.at(active_tab);
  // 根据active_tab获取技能组
  auto skill_node = job_skill_game_instance::load_job_skis(jt);

  auto &mouse_pos = window::mouse_pos;
  uint8_t i = 0;
  const auto ski_w = 32;
  const auto ski_h = 32;

  const auto entry_h = 35;
  const auto l =
      (rb.y - lt.y - max_scroll_num * entry_h) / (max_scroll_num - 1);
  for (auto [k, v] : skill_node) {
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
      return (skill_node.begin() + i + page)->first;
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

    event_motion(nullptr);
  }
}

void skill_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::render_systems, render_info);
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

    event_motion(nullptr);
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
  auto jobs =
      job_skill_game_instance::load_ski_tree(character_game_instance::self.job);
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
  for (uint8_t i = 0; i < jobs.size(); i++) {
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
  auto ski_tree = job_skill_game_instance::load_ski_tree(self_job);
  job_type jt = ski_tree.at(active_tab);

  auto &mouse_pos = window::mouse_pos;
  // 判断按钮是否被遮挡
  auto cursor_in = cursor_game_instance::cursor_ui;
  uint8_t i = 0;
  auto nodes = job_skill_game_instance::load_job_skis(jt);
  for (auto it = nodes.begin() + page; it != nodes.end(); ++it) {
    auto &k = it->first;
    auto &v = it->second;
    if (i >= max_scroll_num) {
      break;
    }
    // render backgrnd
    SDL_FRect pos_rect{
        pos.x + lt.x,
        pos.y + lt.y + i * entry->h + l * i,
        static_cast<float>(entry->w),
        static_cast<float>(entry->h),
    };
    SDL_RenderTexture(window::renderer, entry, nullptr, &pos_rect);

    auto skl_id = std::string{k.begin(), k.end()};
    auto skl_id2 = std::stoi(skl_id);

    SDL_Texture *ski_texture;
    auto ski_level = job_skill_game_instance::load_ski_level(k);
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
  auto self_job = character_game_instance::self.job;
  auto ski_tree = job_skill_game_instance::load_ski_tree(self_job);
  job_type jt = ski_tree.at(active_tab);
  auto skill_node = job_skill_game_instance::load_job_skis(jt);
  auto size = skill_node.size();
  auto cursor_in = cursor_game_instance::cursor_ui;
  bool top = (cursor_in == render) && !cursor_game_instance::modal_overlay;
  scroll_ui_system::render_vscroll((int)pos.x + lt.x, (int)pos.y + lt.y, page,
                                   size, length, top, 6);
  return;
}

bool skill_ui_system::render_info() {
  auto mouse_ski = load_mouse_ski();
  if (mouse_ski.has_value() && !cursor_game_instance::modal_overlay) {
    auto &mouse_pos = window::mouse_pos;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    auto ski_id = mouse_ski.value();
    tooltip_ui_system::render_skill(ski_id, 1, show_pos.x, show_pos.y);
  }
  return true;
}

static std::u16string ski_button_id = u"";

void skill_ui_system::render_button() {
  ski_button_id = u"";

  auto wh = load_wh();
  auto self_job = character_game_instance::self.job;
  auto ski_tree = job_skill_game_instance::load_ski_tree(self_job);
  job_type jt = ski_tree.at(active_tab);
  self_job = job_skill_game_instance::load_job_id(jt);
  auto skill_node = job_skill_game_instance::load_job_skis(jt);

  const int max_scroll = 6;
  const int entry_h = 35;
  const float l = (334 - 99 - max_scroll * entry_h) / (max_scroll - 1);

  // 构建按钮列表
  std::vector<wz::Node *> nodes = {wz_resource::ui->find(u"Basic.img/BtClose")};
  std::vector<SDL_FRect> rects = {{wh.x - 20, 8, 12, 12}};

  const auto disable_ski = [](std::u16string id) {
    auto active = active_tab;
    const auto sps = job_skill_game_instance::remain_point;
    if (sps.at(active) == 0) {
      return true;
    }
    // 判断前置技能
    auto ski_node = skill_game_instance::load_ski_node(id);
    if (auto req = ski_node->get_child(u"req")) {
      for (auto [k, v] : *req) {
        auto lvl = static_cast<wz::Property<int> *>(v[0])->get();
        auto req_lvl = job_skill_game_instance::load_ski_level(k);
        if (req_lvl < lvl) {
          return true;
        }
      }
    }
    auto ski_max_lvl = skill_game_instance::load_ski_max_lvl(id);
    auto ski_lvl = job_skill_game_instance::load_ski_level(id);
    if (ski_lvl >= ski_max_lvl) {
      return true;
    }
    return false;
  };

  std::vector<bool> disable = {
      false,
  };

  for (int i = 0; i < max_scroll && i < skill_node.size(); ++i) {
    nodes.push_back(wz_resource::ui->find(u"Basic.img/BtUP"));
    rects.push_back({8 + 146, 99 + i * entry_h + l * i + 22, 12, 12});
    auto id = (skill_node.begin() + page + i)->first;
    disable.push_back(disable_ski(id));
  }

  // 渲染所有按钮
  bool mouse_down = window::mouse_state & SDL_BUTTON_LMASK;
  bool cursor_on_ui = cursor_game_instance::cursor_ui == render;
  bool modal_blocked = cursor_game_instance::modal_overlay;

  for (size_t i = 0; i < nodes.size(); ++i) {
    SDL_FRect rect = {
        rects[i].x + (int)pos.x,
        rects[i].y + (int)pos.y,
        rects[i].w,
        rects[i].h,
    };

    std::u16string state = u"normal";
    bool mouse_in = SDL_PointInRectFloat(&window::mouse_pos, &rect);

    if (disable[i]) {
      state = u"disabled";
    } else if (cursor_on_ui && !modal_blocked && mouse_in) {
      state = mouse_down ? u"pressed" : u"mouseOver";
      if (i >= 1) {
        ski_button_id = (skill_node.begin() + page + (i - 1))->first;
      }
    }
    auto texture = wz_resource::load_texture(nodes[i]->find((state + u"/0")));
    SDL_RenderTexture(window::renderer, texture, nullptr, &rect);
  }
}

void skill_ui_system::render_book() {
  auto self_job = character_game_instance::self.job;
  auto ski_tree = job_skill_game_instance::load_ski_tree(self_job);
  job_type jt = ski_tree.at(active_tab);
  auto job_str = job_skill_game_instance::load_job_id(jt);

  auto icon_node = wz_resource::skill->find(job_str + u".img/info/icon");
  auto t = wz_resource::load_texture(icon_node);
  SDL_FRect pos_rect{
      pos.x + 15,
      pos.y + 57,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);

  auto book_node =
      wz_resource::string->find(u"Skill.img/" + job_str + u"/bookName");
  auto book_name =
      static_cast<wz::Property<std::u16string> *>(book_node)->get();
  freetype::load_aligned(true);
  freetype::load_size(12);
  freetype::load_color(255, 255, 255, 255);
  freetype::draw_line(book_name, pos_rect.x + 35, pos_rect.y + 6);
}

void skill_ui_system::render_point() {
  auto point = job_skill_game_instance::remain_point[active_tab];
  freetype::load_aligned(true);
  freetype::load_size(12);
  freetype::load_color(0, 0, 0, 255);
  auto str = std::to_string(point);
  std::u16string str2{str.begin(), str.end()};
  auto w = freetype::load_w(str2);
  freetype::draw_line(str2, pos.x + 104 - w, pos.y + 347);
}

bool skill_ui_system::render() {
  render_backgrnd();
  render_skill_entry();
  render_button();
  render_scroll();
  render_tab();
  render_book();
  render_point();
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

void skill_ui_system::event_button_ski_up() {
  if (ski_button_id == u"") {
    return;
  }
  auto ski_id =
      std::stoi(std::string(ski_button_id.begin(), ski_button_id.end()));
  job_skill_game_instance::skill_point[ski_id] += 1;
  job_skill_game_instance::remain_point[active_tab] -= 1;
  return;
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
  auto index = load_mouse_ski();
  if (index.has_value()) {
    if (event->button.clicks >= 1) {
      auto id = index.value();
      if (!skill_game_instance::load_ski_active(id)) {
        return false;
      }
      if (job_skill_game_instance::load_ski_level(id) == 0) {
        return false;
      }
      auto val = std::string{id.begin(), id.end()};
      auto sub_val = std::stoi(val);

      if (cursor_game_instance::cursor_hand.has_value()) {
        if (cursor_game_instance::cursor_hand->type ==
            cursor_game_instance::skill) {
          if (cursor_game_instance::cursor_hand->sub_val == sub_val) {
            // run_skill
            auto &c = character_game_instance::self;
            character_logic_system::run_skill(c, id);
          }
        }
        cursor_game_instance::cursor_hand = std::nullopt;
        return false;
      } else {
        cursor_game_instance::cursor_hand = {
            .type = cursor_game_instance::skill,
            .val = active_tab,
            .sub_val = static_cast<uint32_t>(sub_val),
        };
      }
    }
    return true;
  }
  return false;
}

bool skill_ui_system::event_click_tab(SDL_Event *event) {
  const static SDL_FPoint lt = {5, 24};
  const static SDL_FPoint rb = {186, 43};
  auto jobs =
      job_skill_game_instance::load_ski_tree(character_game_instance::self.job);
  auto &mouse_pos = window::mouse_pos;
  for (uint8_t i = 0; i < jobs.size(); i++) {
    SDL_FPoint tab_pos = {static_cast<float>(lt.x + i * 33), lt.y};
    SDL_FRect pos_rect{
        static_cast<float>(int(pos.x + tab_pos.x)),
        static_cast<float>(int(pos.y + tab_pos.y)),
        33,
        19,
    };
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      active_tab = i;
      page = 0;
      return true;
    }
  }
  return false;
}

void skill_ui_system::event_vscr(SDL_Event *event) {
  const SDL_FPoint lt{174, 98};
  const uint32_t length = 236;
  auto self_job = character_game_instance::self.job;
  auto ski_tree = job_skill_game_instance::load_ski_tree(self_job);
  job_type jt = ski_tree.at(active_tab);
  // 根据active_tab获取技能组
  auto skill_node = job_skill_game_instance::load_job_skis(jt);
  int size = skill_node.size() - 6;
  auto cursor_in = cursor_game_instance::cursor_ui;
  bool top = cursor_in == render;
  size = std::max(0, size);
  auto val = scroll_ui_system::click_vscroll(
      (int)pos.x + lt.x, (int)pos.y + lt.y, page, size, length, top);
  page = val;
}

void skill_ui_system::event_motion(SDL_Event *event) {
  auto &sys = system::render_systems;
  std::erase(sys, render_info);
  auto it = std::ranges::find(sys, &cursor_render_system::render);
  if (it != sys.end()) {
    sys.insert(it, render_info);
  }
}

void skill_ui_system::event_vscr_start(SDL_Event *event) {
  const SDL_FPoint lt{174, 98};
  const uint32_t length = 236;
  if (vscr_motion == false) {
    vscr_motion =
        scroll_ui_system::click_thumb(pos.x + lt.x, pos.y + lt.y, length);
  }
}

void skill_ui_system::event_vscr_end() { vscr_motion = {}; }

void skill_ui_system::event_vscr_move(SDL_Event *event) {
  auto mouse_state = window::mouse_state;
  const SDL_FPoint lt{174, 98};
  const uint32_t length = 236;
  if (vscr_motion) {
    event->button.x = pos.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos.y + lt.y, pos.y + lt.y + length);
    event_vscr(event);
  }
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
        event_button_ski_up();
        event_vscr(event);
        event_click_ski(event);
        event_click_tab(event);
        r = event_button(event);
      }
      event_drag_end();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_motion(event);
    event_drag_move(event);
    break;
  }
  case SDL_EVENT_MOUSE_WHEEL: {
    auto dy = event->wheel.integer_y;
    if (dy > 0) {
      // up
      if (page > 0) {
        page -= 1;
      }
    } else {
      // down
      auto self_job = character_game_instance::self.job;
      auto ski_tree = job_skill_game_instance::load_ski_tree(self_job);
      job_type jt = ski_tree.at(active_tab);
      // 根据active_tab获取技能组
      auto skill_node = job_skill_game_instance::load_job_skis(jt);
      if ((int)skill_node.size() - 6 < 0) {
        break;
      }
      if (page < skill_node.size() - 6) {
        page += 1;
      }
    }
    break;
  }
  default: {
    break;
  }
  }

  return r;
}