#include "quest_alarm_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

SDL_FPoint quest_alarm_ui_system::load_wh() {
  float h = 0;
  auto quests = quest_game_instance::load_progress_quest();
  for (auto &q : quests) {
    h += q.check_item.size();
    h += q.check_mob.size();
    h += q.check_npc.size();
  }
  h = (h + 1) * 12;
  if (max) {
    return {223, h};
  }
  return {223, h};
}

void quest_alarm_ui_system::render_backgrnd() {
  if (max) {
    static auto backgrndmax = wz_resource::load_texture(
        wz_resource::ui->find(u"Quest.img/QuestAlarm/backgrndmax"));
    static auto backgrndcenter = wz_resource::load_texture(
        wz_resource::ui->find(u"Quest.img/QuestAlarm/backgrndcenter"));
    static auto backgrndbottom = wz_resource::load_texture(
        wz_resource::ui->find(u"Quest.img/QuestAlarm/backgrndbottom"));
    auto h = load_wh().y;
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
      {170, 4, 21, 12}, //
      {205, 4, 12, 12}, //
  };
  if (max) {
    nodes.push_back(wz_resource::ui->find(u"Basic.img/BtMin"));
    rects.push_back({192, 4, 12, 12});
  } else {
    nodes.push_back(wz_resource::ui->find(u"Basic.img/BtMax"));
    rects.push_back({192, 4, 12, 12});
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

void quest_alarm_ui_system::render_quests() {}

void quest_alarm_ui_system::render_title() {
  auto n = wz_resource::ms->get_root()->find(u"String.img/Quest/Helper");
  auto str = static_cast<wz::Property<std::u16string> *>(n)->get();
  freetype::load_size(12);
  freetype::load_color(0, 0, 0, 255);
  freetype::load_aligned(true);
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
      {170, 4, 21, 12}, //
      {205, 4, 12, 12}, //
  };
  std::vector<void (*)()> fns = {};
  if (max) {
    fns.push_back(event_button_min);
    r.push_back({192, 4, 12, 12});
  } else {
    fns.push_back(event_button_max);
    r.push_back({192, 4, 12, 12});
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

void quest_alarm_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

bool quest_alarm_ui_system::event(SDL_Event *event) {
    return true;
}