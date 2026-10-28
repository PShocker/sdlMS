#include "skill_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "scroll_ui_system.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
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

SDL_FPoint SkillUiSystem::load_wh() { return {197, 371}; }

bool SkillUiSystem::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void SkillUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

void SkillUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::render_systems, render_info);
  std::erase(System::event_systems, event);

  event_drag_end();
  event_vscr_end();
}

void SkillUiSystem::event_top() {
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

void SkillUiSystem::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void SkillUiSystem::event_drag_end() {
  drag = std::nullopt;
  return;
}

void SkillUiSystem::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = CameraGameInstance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void SkillUiSystem::render_backgrnd() {
  static auto back_texture =
      WzResource::load_texture(WzResource::ui->find(u"Skill.img/backgrnd"));
  SDL_FRect pos_rect = {
      pos.x,
      pos.y,
      (float)back_texture->w,
      (float)back_texture->h,
  };
  SDL_RenderTexture(Window::renderer, back_texture, nullptr, &pos_rect);
}

void SkillUiSystem::render_tab() {
  const static SDL_FPoint lt = {5, 24};
  const static SDL_FPoint rb = {186, 43};
  const static auto tab_node = WzResource::ui->find(u"Skill.img/tab:grade");
  auto jobs =
      JobSkillGameInstance::load_ski_tree(CharacterGameInstance::self.job);
  const static std::array selected_texture = {
      WzResource::load_texture(tab_node->find(u"selected/0")),
      WzResource::load_texture(tab_node->find(u"selected/1")),
      WzResource::load_texture(tab_node->find(u"selected/2")),
      WzResource::load_texture(tab_node->find(u"selected/3")),
      WzResource::load_texture(tab_node->find(u"selected/4")),
  };
  const static std::array normal_texture = {
      WzResource::load_texture(tab_node->find(u"normal/0")),
      WzResource::load_texture(tab_node->find(u"normal/1")),
      WzResource::load_texture(tab_node->find(u"normal/2")),
      WzResource::load_texture(tab_node->find(u"normal/3")),
      WzResource::load_texture(tab_node->find(u"normal/4")),
  };
  for (uint8_t i = 0; i < jobs.size(); i++) {
    SDL_Texture *t = active_tab == i ? selected_texture[i] : normal_texture[i];
    SDL_FPoint tab_pos = {static_cast<float>(lt.x + i * 33), lt.y};
    SDL_FRect pos_rect{static_cast<float>(int(pos.x + tab_pos.x)),
                       static_cast<float>(int(pos.y + tab_pos.y)),
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
  }
}

static std::u16string mouse_skill_id;

void SkillUiSystem::render_skill_entry() {
  mouse_skill_id = u"";
  auto entry = WzResource::load_texture(
      WzResource::ui->find(u"Skill.img/entry/skill1"));

  const SDL_FPoint lt{8, 99};
  const SDL_FPoint rb{184, 334};
  const SDL_FPoint pos_icon{2, 2};
  const uint8_t max_scroll_num = 6;
  const auto l =
      (rb.y - lt.y - max_scroll_num * entry->h) / (max_scroll_num - 1);
  auto self_job = CharacterGameInstance::self.job;
  auto ski_tree = JobSkillGameInstance::load_ski_tree(self_job);
  JobType jt = ski_tree.at(active_tab);

  auto &mouse_pos = Window::mouse_pos;
  // 判断按钮是否被遮挡
  auto cursor_in = CursorGameInstance::cursor_ui;
  uint8_t i = 0;
  auto nodes = JobSkillGameInstance::load_job_skis(jt);
  for (auto it = nodes.begin() + page; it != nodes.end(); ++it) {
    auto &k = it->first;
    auto &v = it->second;
    if (i >= max_scroll_num) {
      break;
    }
    auto skl_id = std::string{k.begin(), k.end()};
    auto skl_id2 = std::stoi(skl_id);

    SDL_Texture *ski_texture;
    auto ski_level = JobSkillGameInstance::load_ski_level(k);
    SDL_FRect pos_rect = {
        pos.x + lt.x + 2,
        pos.y + lt.y + i * entry->h + l * i + 2,
        static_cast<float>(32),
        static_cast<float>(32),
    };
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      mouse_skill_id = k;
    }
    if (ski_level > 0) {
      if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
          !CursorGameInstance::modal_overlay) {
        ski_texture =
            WzResource::load_texture(v[0]->get_child(u"iconMouseOver"));
      } else {
        ski_texture = WzResource::load_texture(v[0]->get_child(u"icon"));
      }
    } else {
      // disable
      ski_texture = WzResource::load_texture(v[0]->get_child(u"iconDisabled"));
      entry = WzResource::load_texture(
          WzResource::ui->find(u"Skill.img/entry/skill0"));
    }
    // render backgrnd
    pos_rect = {
        pos.x + lt.x,
        pos.y + lt.y + i * entry->h + l * i,
        static_cast<float>(entry->w),
        static_cast<float>(entry->h),
    };
    SDL_RenderTexture(Window::renderer, entry, nullptr, &pos_rect);

    pos_rect = {
        pos.x + lt.x + 2,
        pos.y + lt.y + i * entry->h + l * i + 2,
        static_cast<float>(ski_texture->w),
        static_cast<float>(ski_texture->h),
    };
    SDL_RenderTexture(Window::renderer, ski_texture, nullptr, &pos_rect);
    // render ski_name
    auto ski_name = SkillGameInstance::load_ski_name(k);
    Freetype::load_color(0, 0, 0, 255);
    Freetype::load_aligned(true);
    Freetype::load_size(12);
    Freetype::draw_line(ski_name.name, pos.x + lt.x + 40, pos_rect.y - 3);
    auto ski_level2 = std::to_string(ski_level);
    auto ski_level3 = std::u16string{ski_level2.begin(), ski_level2.end()};
    Freetype::draw_line(ski_level3, pos.x + lt.x + 40, pos_rect.y + 15);

    Freetype::load_aligned(false);
    i++;
  }
}

void SkillUiSystem::render_scroll() {
  const SDL_FPoint lt{174, 98};
  const uint32_t length = 236;
  auto self_job = CharacterGameInstance::self.job;
  auto ski_tree = JobSkillGameInstance::load_ski_tree(self_job);
  JobType jt = ski_tree.at(active_tab);
  auto skill_node = JobSkillGameInstance::load_job_skis(jt);
  auto size = skill_node.size();
  auto cursor_in = CursorGameInstance::cursor_ui;
  bool top = (cursor_in == render) && !CursorGameInstance::modal_overlay;
  ScrollUiSystem::render_vscroll((int)pos.x + lt.x, (int)pos.y + lt.y, page,
                                   size, length, top, 6);
  return;
}

bool SkillUiSystem::render_info() {
  if (!mouse_skill_id.empty() && !CursorGameInstance::modal_overlay &&
      CursorGameInstance::cursor_ui == render) {
    auto &mouse_pos = Window::mouse_pos;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    TooltipUiSystem::render_skill(mouse_skill_id, 1, show_pos.x, show_pos.y);
  }
  return true;
}

static std::u16string ski_button_id = u"";

void SkillUiSystem::render_button() {
  ski_button_id = u"";

  auto wh = load_wh();
  auto self_job = CharacterGameInstance::self.job;
  auto ski_tree = JobSkillGameInstance::load_ski_tree(self_job);
  JobType jt = ski_tree.at(active_tab);
  self_job = JobSkillGameInstance::load_job_id(jt);
  auto skill_node = JobSkillGameInstance::load_job_skis(jt);

  const int max_scroll = 6;
  const int entry_h = 35;
  const float l = (334 - 99 - max_scroll * entry_h) / (max_scroll - 1);

  // 构建按钮列表
  std::vector<wz::Node *> nodes = {WzResource::ui->find(u"Basic.img/BtClose")};
  std::vector<SDL_FRect> rects = {{wh.x - 20, 8, 12, 12}};

  const auto disable_ski = [](std::u16string id) {
    auto active = active_tab;
    const auto sps = JobSkillGameInstance::remain_point;
    if (sps.at(active) == 0) {
      return true;
    }
    // 判断前置技能
    auto ski_node = SkillGameInstance::load_ski_node(id);
    if (auto req = ski_node->get_child(u"req")) {
      for (auto [k, v] : *req) {
        auto lv = static_cast<wz::Property<int> *>(v[0])->get();
        auto req_lv = JobSkillGameInstance::load_ski_level(k);
        if (req_lv < lv) {
          return true;
        }
      }
    }
    auto ski_max_lvl = SkillGameInstance::load_ski_max_lvl(id);
    auto ski_lvl = JobSkillGameInstance::load_ski_level(id);
    if (ski_lvl >= ski_max_lvl) {
      return true;
    }
    return false;
  };

  std::vector<bool> disable = {
      false,
  };

  for (int i = 0; i < max_scroll && i < skill_node.size(); ++i) {
    nodes.push_back(WzResource::ui->find(u"Basic.img/BtUP"));
    rects.push_back({8 + 146, 99 + i * entry_h + l * i + 22, 12, 12});
    auto id = (skill_node.begin() + page + i)->first;
    disable.push_back(disable_ski(id));
  }

  // 渲染所有按钮
  bool mouse_down = Window::mouse_state & SDL_BUTTON_LMASK;
  bool cursor_on_ui = CursorGameInstance::cursor_ui == render;
  bool modal_blocked = CursorGameInstance::modal_overlay;

  for (size_t i = 0; i < nodes.size(); ++i) {
    SDL_FRect rect = {
        rects[i].x + (int)pos.x,
        rects[i].y + (int)pos.y,
        rects[i].w,
        rects[i].h,
    };

    std::u16string state = u"normal";
    bool mouse_in = SDL_PointInRectFloat(&Window::mouse_pos, &rect);

    if (disable[i]) {
      state = u"disabled";
    } else if (cursor_on_ui && !modal_blocked && mouse_in) {
      state = mouse_down ? u"pressed" : u"mouseOver";
      if (i >= 1) {
        ski_button_id = (skill_node.begin() + page + (i - 1))->first;
      }
    }
    auto texture = WzResource::load_texture(nodes[i]->find((state + u"/0")));
    SDL_RenderTexture(Window::renderer, texture, nullptr, &rect);
  }
}

void SkillUiSystem::render_book() {
  auto self_job = CharacterGameInstance::self.job;
  auto ski_tree = JobSkillGameInstance::load_ski_tree(self_job);
  JobType jt = ski_tree.at(active_tab);
  auto job_str = JobSkillGameInstance::load_job_id(jt);

  auto icon_node = WzResource::skill->find(job_str + u".img/info/icon");
  auto t = WzResource::load_texture(icon_node);
  SDL_FRect pos_rect{
      pos.x + 15,
      pos.y + 57,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);

  auto book_node =
      WzResource::string->find(u"Skill.img/" + job_str + u"/bookName");
  auto book_name =
      static_cast<wz::Property<std::u16string> *>(book_node)->get();
  Freetype::load_aligned(true);
  Freetype::load_size(12);
  Freetype::load_color(255, 255, 255, 255);
  Freetype::draw_line(book_name, pos_rect.x + 35, pos_rect.y + 6);
}

void SkillUiSystem::render_point() {
  auto point = JobSkillGameInstance::remain_point[active_tab];
  Freetype::load_aligned(true);
  Freetype::load_size(12);
  Freetype::load_color(0, 0, 0, 255);
  auto str = std::to_string(point);
  std::u16string str2{str.begin(), str.end()};
  auto w = Freetype::load_w(str2);
  Freetype::draw_line(str2, pos.x + 104 - w, pos.y + 347);
}

bool SkillUiSystem::render() {
  render_backgrnd();
  render_skill_entry();
  render_button();
  render_scroll();
  render_tab();
  render_book();
  render_point();
  return true;
}

void SkillUiSystem::toggle() {
  AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
  auto fn = &render;
  if (std::ranges::contains(System::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

void SkillUiSystem::event_button_ski_up() {
  if (ski_button_id == u"") {
    return;
  }
  auto ski_id =
      std::stoi(std::string(ski_button_id.begin(), ski_button_id.end()));
  JobSkillGameInstance::skill_point[ski_id] += 1;
  JobSkillGameInstance::remain_point[active_tab] -= 1;
  CharacterStatGameInstance::update();
  return;
}

void SkillUiSystem::event_close() { close(); }

bool SkillUiSystem::event_button(SDL_Event *event) {
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
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

bool SkillUiSystem::event_click_ski(SDL_Event *event) {
  if (CursorGameInstance::cursor_hand_net.has_value()) {
    return false;
  }
  if (!mouse_skill_id.empty()) {
    if (event->button.clicks >= 1) {
      auto id = mouse_skill_id;
      if (!SkillGameInstance::load_ski_active(id)) {
        return false;
      }
      if (JobSkillGameInstance::load_ski_level(id) == 0) {
        return false;
      }
      auto val = std::string{id.begin(), id.end()};
      auto sub_val = std::stoi(val);

      if (CursorGameInstance::cursor_hand.has_value()) {
        if (CursorGameInstance::cursor_hand->type ==
            CursorGameInstance::skill) {
          if (CursorGameInstance::cursor_hand->sub_val == sub_val) {
            // run_skill
            auto &c = CharacterGameInstance::self;
            CharacterLogicSystem::run_skill(c, id);
          }
        }
        CursorGameInstance::cursor_hand = std::nullopt;
        AudioGameInstance::load_audio(u"UI.img/DragEnd", 0);
        return false;
      } else {
        AudioGameInstance::load_audio(u"UI.img/DragStart", 0);
        CursorGameInstance::cursor_hand = {
            .type = CursorGameInstance::skill,
            .val = active_tab,
            .sub_val = static_cast<uint32_t>(sub_val),
        };
      }
    }
    return true;
  }
  return false;
}

bool SkillUiSystem::event_click_tab(SDL_Event *event) {
  const static SDL_FPoint lt = {5, 24};
  const static SDL_FPoint rb = {186, 43};
  auto jobs =
      JobSkillGameInstance::load_ski_tree(CharacterGameInstance::self.job);
  auto &mouse_pos = Window::mouse_pos;
  for (uint8_t i = 0; i < jobs.size(); i++) {
    SDL_FPoint tab_pos = {static_cast<float>(lt.x + i * 33), lt.y};
    SDL_FRect pos_rect{
        static_cast<float>(int(pos.x + tab_pos.x)),
        static_cast<float>(int(pos.y + tab_pos.y)),
        33,
        19,
    };
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
      active_tab = i;
      page = 0;
      return true;
    }
  }
  return false;
}

void SkillUiSystem::event_vscr(SDL_Event *event) {
  const SDL_FPoint lt{174, 98};
  const uint32_t length = 236;
  auto self_job = CharacterGameInstance::self.job;
  auto ski_tree = JobSkillGameInstance::load_ski_tree(self_job);
  JobType jt = ski_tree.at(active_tab);
  // 根据active_tab获取技能组
  auto skill_node = JobSkillGameInstance::load_job_skis(jt);
  int size = skill_node.size() - 6;
  auto cursor_in = CursorGameInstance::cursor_ui;
  bool top = cursor_in == render;
  size = std::max(0, size);
  auto val = ScrollUiSystem::click_vscroll(
      (int)pos.x + lt.x, (int)pos.y + lt.y, page, size, length, top);
  page = val;
}

void SkillUiSystem::event_motion(SDL_Event *event) {
  auto &sys = System::render_systems;
  std::erase(sys, render_info);
  auto it = std::ranges::find(sys, &CursorRenderSystem::render);
  if (it != sys.end()) {
    sys.insert(it, render_info);
  }
}

void SkillUiSystem::event_vscr_start(SDL_Event *event) {
  const SDL_FPoint lt{174, 98};
  const uint32_t length = 236;
  if (vscr_motion == false) {
    vscr_motion =
        ScrollUiSystem::click_thumb(pos.x + lt.x, pos.y + lt.y, length);
  }
}

void SkillUiSystem::event_vscr_end() { vscr_motion = {}; }

void SkillUiSystem::event_vscr_move(SDL_Event *event) {
  auto mouse_state = Window::mouse_state;
  const SDL_FPoint lt{174, 98};
  const uint32_t length = 236;
  if (vscr_motion) {
    event->button.x = pos.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos.y + lt.y, pos.y + lt.y + length);
    event_vscr(event);
  }
}

void SkillUiSystem::event_wheel(SDL_Event *event) {
  if (CursorGameInstance::cursor_ui != render) {
    return;
  }
  auto dy = event->wheel.integer_y;
  if (dy > 0) {
    // up
    if (page > 0) {
      page -= 1;
    }
  } else {
    // down
    auto self_job = CharacterGameInstance::self.job;
    auto ski_tree = JobSkillGameInstance::load_ski_tree(self_job);
    JobType jt = ski_tree.at(active_tab);
    // 根据active_tab获取技能组
    auto skill_node = JobSkillGameInstance::load_job_skis(jt);
    int size = skill_node.size() - 6;
    size = std::max(0, size);
    if (page < size) {
      page++;
    }
  }
}

void SkillUiSystem::event_motion_bt_audio(SDL_Event *event) {
  if (CursorGameInstance::cursor_ui != render) {
    return;
  }
  static std::u16string ski_button_i;
  if (!ski_button_id.empty() && ski_button_i.empty()) {
    AudioGameInstance::load_audio(u"UI.img/BtMouseOver", 0);
    ski_button_i = ski_button_id;
    return;
  }
  ski_button_i = ski_button_id;

  auto wh = load_wh();
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{wh.x - 20, 7, 12, 12}, //
  };
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

bool SkillUiSystem::event(SDL_Event *event) {
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
    event_motion_bt_audio(event);
    event_motion(event);
    event_drag_move(event);
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