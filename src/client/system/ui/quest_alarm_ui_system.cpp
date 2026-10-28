#include "quest_alarm_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>
#include <format>
#include <string>

SDL_FPoint QuestAlarmUiSystem::load_wh() {
  float h = 0;
  for (auto id : quests) {
    auto q = QuestGameInstance::progress_quests.at(id);
    h += q.check_item.size() * 18;
    h += q.check_mob.size() * 18;
    h += 18;
  }
  if (max && h > 0) {
    return {223, h + 20};
  }
  return {223, 20};
}

void QuestAlarmUiSystem::render_backgrnd() {
  auto h = load_wh().y;
  if (max && h > 20) {
    h -= 20;
    static auto backgrndmax = WzResource::load_texture(
        WzResource::ui->find(u"Quest.img/QuestAlarm/backgrndmax"));
    static auto backgrndcenter = WzResource::load_texture(
        WzResource::ui->find(u"Quest.img/QuestAlarm/backgrndcenter"));
    static auto backgrndbottom = WzResource::load_texture(
        WzResource::ui->find(u"Quest.img/QuestAlarm/backgrndbottom"));
    SDL_FRect pos_rect{
        pos.x,
        pos.y,
        static_cast<float>(backgrndmax->w),
        static_cast<float>(backgrndmax->h),
    };
    SDL_RenderTexture(Window::renderer, backgrndmax, nullptr, &pos_rect);
    pos_rect.y += backgrndmax->h;
    pos_rect.h = h;
    SDL_RenderTextureTiled(Window::renderer, backgrndcenter, nullptr, 1,
                           &pos_rect);
    pos_rect.y += h;
    pos_rect.h = backgrndbottom->h;
    SDL_RenderTexture(Window::renderer, backgrndbottom, nullptr, &pos_rect);
  } else {
    static auto backgrndmin = WzResource::load_texture(
        WzResource::ui->find(u"Quest.img/QuestAlarm/backgrndmin"));
    SDL_FRect pos_rect{
        pos.x,
        pos.y,
        static_cast<float>(backgrndmin->w),
        static_cast<float>(backgrndmin->h),
    };
    SDL_RenderTexture(Window::renderer, backgrndmin, nullptr, &pos_rect);
  }
}

void QuestAlarmUiSystem::render_button() {
  // 构建按钮列表
  std::vector<wz::Node *> nodes = {
      WzResource::ui->find(u"Quest.img/QuestAlarm/BtAuto"),
      WzResource::ui->find(u"Basic.img/BtClose"), // hp
  };
  std::vector<SDL_FRect> rects = {
      {173, 4, 21, 12}, //
      {208, 4, 12, 12}, //
  };
  if (max) {
    nodes.push_back(WzResource::ui->find(u"Basic.img/BtMin"));
    rects.push_back({195, 4, 12, 12});
  } else {
    nodes.push_back(WzResource::ui->find(u"Basic.img/BtMax"));
    rects.push_back({195, 4, 12, 12});
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

    if (cursor_on_ui && !modal_blocked && mouse_in) {
      state = mouse_down ? u"pressed" : u"mouseOver";
    }
    auto texture = WzResource::load_texture(nodes[i]->find((state + u"/0")));
    SDL_RenderTexture(Window::renderer, texture, nullptr, &rect);
  }
}

static std::u16string close_quest;

void QuestAlarmUiSystem::render_quests() {
  close_quest = u"";
  int y = 20;
  for (const auto &id : quests) {
    auto node = QuestGameInstance::load_quest_node(id);
    node = node->find(u"QuestInfo/name");
    auto name = static_cast<wz::Property<std::u16string> *>(node)->get();
    if (name.size() > 25) {
      name = name.substr(0, 25) + u"...";
    }
    Freetype::load_size(12);
    Freetype::load_bold(true);
    Freetype::load_color(0, 0, 0, 255);
    Freetype::load_aligned(true);
    Freetype::draw_line(name, pos.x + 5, pos.y + y);
    Freetype::load_bold(false);
    static auto close_t = WzResource::load_texture(
        WzResource::ui->find(u"Quest.img/QuestAlarm/BtDelete/normal/0"));
    SDL_FRect pos_rect{
        static_cast<float>((int)pos.x + 208),
        static_cast<float>((int)pos.y + y + 6),
        static_cast<float>(close_t->w),
        static_cast<float>(close_t->h),
    };
    SDL_RenderTexture(Window::renderer, close_t, nullptr, &pos_rect);
    auto mouse_pos = Window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      close_quest = id;
    }
    const auto &quest = QuestGameInstance::progress_quests.at(id);
    for (const auto &[k, v] : quest.check_item) {
      y += 18;
      uint32_t num = 0;
      if (quest.item.contains(k)) {
        num = quest.item.at(k).count;
      }
      auto count = v.count;
      auto tmp = std::format("{}/{}", num, count);
      std::u16string str;
      if (ItemGameInstance::check_item(k)) {
        str = ItemGameInstance::load_item_text(k, u"name");
      } else {
        str = EquipGameInstance::load_equip_name(k);
      }
      str = std::u16string{tmp.begin(), tmp.end()} + u" " + str;
      Freetype::draw_line(str, pos.x + 5, pos.y + y);
    }
    for (const auto &[k, v] : quest.check_mob) {
      y += 18;
      uint32_t num = 0;
      if (quest.mob.contains(k)) {
        num = quest.mob.at(k).count;
      }
      auto count = v.count;
      auto tmp = std::format("{}/{}", num, count);
      std::u16string str = MobGameInstance::load_mob_name(k);
      str = std::u16string{tmp.begin(), tmp.end()} + u" " + str;
      Freetype::draw_line(str, pos.x + 5, pos.y + y);
    }
    y += 18;
  }
}

void QuestAlarmUiSystem::render_title() {
  auto n = WzResource::ms->get_root()->find(u"String.img/Quest/Helper");
  auto str = static_cast<wz::Property<std::u16string> *>(n)->get();
  Freetype::load_size(12);
  Freetype::load_color(0, 0, 0, 255);
  Freetype::load_aligned(true);
  auto max_num = 5;
  auto quest_num = quests.size();
  std::string str2 = std::format("({}/{})", quest_num, max_num);
  str = str + std::u16string{str2.begin(), str2.end()};
  Freetype::draw_line(str, pos.x + 5, pos.y);
}

bool QuestAlarmUiSystem::render() {
  render_backgrnd();
  render_title();
  render_quests();
  render_button();
  return true;
}

void QuestAlarmUiSystem::event_button_auto() { event_auto_quests(); }

void QuestAlarmUiSystem::event_button_max() {}

void QuestAlarmUiSystem::event_button_min() {}

void QuestAlarmUiSystem::event_button_close() { close(); }

bool QuestAlarmUiSystem::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r = {
      {173, 4, 21, 12}, //
      {208, 4, 12, 12}, //
  };
  std::vector<void (*)()> fns = {
      event_button_auto,
      event_button_close,
  };
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
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      fns[i]();
      AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }

  return false;
}

void QuestAlarmUiSystem::open() {
  close();
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

void QuestAlarmUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);

  quests.clear();

  event_drag_end();
}

void QuestAlarmUiSystem::event_top() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

void QuestAlarmUiSystem::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void QuestAlarmUiSystem::event_drag_end() {
  drag = std::nullopt;
  return;
}

void QuestAlarmUiSystem::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = CameraGameInstance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void QuestAlarmUiSystem::event_quest_close() {
  if (close_quest.empty()) {
    return;
  }
  quests.erase(close_quest);
}

bool QuestAlarmUiSystem::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
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

void QuestAlarmUiSystem::event_auto_quests() {
  if (quests.empty()) {
    size_t count = 0;
    for (const auto &[key, _] : QuestGameInstance::progress_quests) {
      if (count++ >= 5)
        break;
      quests.emplace(key);
    }
  }
}

bool QuestAlarmUiSystem::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void QuestAlarmUiSystem::accept_quest(const std::u16string &id) {
  auto it = std::ranges::find(System::render_systems, &render);
  if (it == System::render_systems.end()) {
    return;
  }
  if (quests.size() > 5) {
    return;
  }
  quests.insert(id);
}

void QuestAlarmUiSystem::complete_quest(const std::u16string &id) {
  quests.erase(id);
}