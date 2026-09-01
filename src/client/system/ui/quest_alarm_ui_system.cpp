#include "quest_alarm_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <format>
#include <string>

SDL_FPoint quest_alarm_ui_system::load_wh() {
  float h = 0;
  for (auto id : quests) {
    auto q = quest_game_instance::progress_quests.at(id);
    h += q.check_item.size() * 18;
    h += q.check_mob.size() * 18;
    h += q.check_npc.size() * 18;
    h += 12;
  }
  if (max) {
    return {223, h + 30};
  }
  return {223, 20};
}

void quest_alarm_ui_system::render_backgrnd() {
  if (max) {
    static auto backgrndmax = wz_resource::load_texture(
        wz_resource::ui->find(u"Quest.img/QuestAlarm/backgrndmax"));
    static auto backgrndcenter = wz_resource::load_texture(
        wz_resource::ui->find(u"Quest.img/QuestAlarm/backgrndcenter"));
    static auto backgrndbottom = wz_resource::load_texture(
        wz_resource::ui->find(u"Quest.img/QuestAlarm/backgrndbottom"));
    auto h = load_wh().y - 30;
    SDL_FRect pos_rect{
        pos.x,
        pos.y,
        static_cast<float>(backgrndmax->w),
        static_cast<float>(backgrndmax->h),
    };
    SDL_RenderTexture(window::renderer, backgrndmax, nullptr, &pos_rect);
    pos_rect.y += backgrndmax->h;
    pos_rect.h = h;
    SDL_RenderTextureTiled(window::renderer, backgrndcenter, nullptr, 1,
                           &pos_rect);
    pos_rect.y += h;
    pos_rect.h = backgrndbottom->h;
    SDL_RenderTexture(window::renderer, backgrndbottom, nullptr, &pos_rect);
  } else {
    static auto backgrndmin = wz_resource::load_texture(
        wz_resource::ui->find(u"Quest.img/QuestAlarm/backgrndmin"));
    SDL_FRect pos_rect{
        pos.x,
        pos.y,
        static_cast<float>(backgrndmin->w),
        static_cast<float>(backgrndmin->h),
    };
    SDL_RenderTexture(window::renderer, backgrndmin, nullptr, &pos_rect);
  }
}

void quest_alarm_ui_system::render_button() {
  // 构建按钮列表
  std::vector<wz::Node *> nodes = {
      wz_resource::ui->find(u"Quest.img/QuestAlarm/BtAuto"),
      wz_resource::ui->find(u"Basic.img/BtClose"), // hp
  };
  std::vector<SDL_FRect> rects = {
      {173, 4, 21, 12}, //
      {208, 4, 12, 12}, //
  };
  if (max) {
    nodes.push_back(wz_resource::ui->find(u"Basic.img/BtMin"));
    rects.push_back({195, 4, 12, 12});
  } else {
    nodes.push_back(wz_resource::ui->find(u"Basic.img/BtMax"));
    rects.push_back({195, 4, 12, 12});
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

    if (cursor_on_ui && !modal_blocked && mouse_in) {
      state = mouse_down ? u"pressed" : u"mouseOver";
    }
    auto texture = wz_resource::load_texture(nodes[i]->find((state + u"/0")));
    SDL_RenderTexture(window::renderer, texture, nullptr, &rect);
  }
}

static std::u16string close_quest;

void quest_alarm_ui_system::render_quests() {
  close_quest = u"";
  int i = 0;
  int y = 20;
  for (const auto &id : quests) {
    auto node = quest_game_instance::load_quest_node(id);
    node = node->find(u"QuestInfo/name");
    auto name = static_cast<wz::Property<std::u16string> *>(node)->get();
    freetype::load_size(12);
    freetype::load_bold(true);
    freetype::load_color(0, 0, 0, 255);
    freetype::load_aligned(true);
    freetype::draw_line(name, pos.x + 5, pos.y + y);
    freetype::load_bold(false);
    static auto close_t = wz_resource::load_texture(
        wz_resource::ui->find(u"Quest.img/QuestAlarm/BtDelete/normal/0"));
    SDL_FRect pos_rect{
        static_cast<float>((int)pos.x + 208),
        static_cast<float>((int)pos.y + y + 6),
        static_cast<float>(close_t->w),
        static_cast<float>(close_t->h),
    };
    SDL_RenderTexture(window::renderer, close_t, nullptr, &pos_rect);
    auto mouse_pos = window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      close_quest = id;
    }
    const auto &quest = quest_game_instance::progress_quests.at(id);
    for (const auto &[k, v] : quest.check_item) {
      y += 18;
      auto num = quest.item.at(k).count;
      auto count = v.count;
      auto tmp = std::format("{}/{}", num, count);
      std::u16string str;
      if (item_game_instance::check_item(k)) {
        str = item_game_instance::load_item_text(k, u"name");
      } else {
        str = equip_game_instance::load_equip_name(k);
      }
      str = std::u16string{tmp.begin(), tmp.end()} + u" " + str;
      freetype::draw_line(str, pos.x + 5, pos.y + y);
    }
    for (const auto &[k, v] : quest.check_mob) {
      y += 18;
      freetype::draw_line(u"123456", pos.x + 5, pos.y + y);
    }
    for (const auto &[k, v] : quest.check_npc) {
      y += 18;
      freetype::draw_line(u"123456", pos.x + 5, pos.y + y);
    }
    i++;
  }
}

void quest_alarm_ui_system::render_title() {
  auto n = wz_resource::ms->get_root()->find(u"String.img/Quest/Helper");
  auto str = static_cast<wz::Property<std::u16string> *>(n)->get();
  freetype::load_size(12);
  freetype::load_color(0, 0, 0, 255);
  freetype::load_aligned(true);
  auto max_num = 5;
  auto quest_num = quests.size();
  std::string str2 = std::format("({}/{})", quest_num, max_num);
  str = str + std::u16string{str2.begin(), str2.end()};
  freetype::draw_line(str, pos.x + 5, pos.y);
}

bool quest_alarm_ui_system::render() {
  render_backgrnd();
  render_title();
  render_quests();
  render_button();
  return true;
}

void quest_alarm_ui_system::event_button_auto() {}

void quest_alarm_ui_system::event_button_max() {}

void quest_alarm_ui_system::event_button_min() {}

void quest_alarm_ui_system::event_button_close() {}

bool quest_alarm_ui_system::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r = {
      {173, 4, 21, 12}, //
      {208, 4, 12, 12}, //
  };
  std::vector<void (*)()> fns = {};
  if (max) {
    fns.push_back(event_button_min);
    r.push_back({195, 4, 12, 12});
  } else {
    fns.push_back(event_button_max);
    r.push_back({195, 4, 12, 12});
  }
  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      audio_game_instance::load_audio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }

  return false;
}

void quest_alarm_ui_system::open() {
  close();
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    event_auto_quests();

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void quest_alarm_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  event_drag_end();
}

void quest_alarm_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void quest_alarm_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void quest_alarm_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void quest_alarm_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void quest_alarm_ui_system::event_quest_close() {
  if (close_quest.empty()) {
    return;
  }
  quests.erase(close_quest);
}

bool quest_alarm_ui_system::event(SDL_Event *event) {
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
        event_quest_close();
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

void quest_alarm_ui_system::event_auto_quests() {
  if (quests.empty()) {
    size_t count = 0;
    for (const auto &[key, _] : quest_game_instance::progress_quests) {
      if (count++ >= 5)
        break;
      quests.emplace(key);
    }
  }
}

bool quest_alarm_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}