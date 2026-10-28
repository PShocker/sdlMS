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

SDL_FPoint QuestAlarmUiSystem::LoadWh() {
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

void QuestAlarmUiSystem::RenderBackgrnd() {
  auto h = LoadWh().y;
  if (max && h > 20) {
    h -= 20;
    static auto backgrndmax = WzResource::LoadTexture(
        WzResource::ui->find(u"Quest.img/QuestAlarm/backgrndmax"));
    static auto backgrndcenter = WzResource::LoadTexture(
        WzResource::ui->find(u"Quest.img/QuestAlarm/backgrndcenter"));
    static auto backgrndbottom = WzResource::LoadTexture(
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
    static auto backgrndmin = WzResource::LoadTexture(
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

void QuestAlarmUiSystem::RenderButton() {
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
  bool cursor_on_ui = CursorGameInstance::cursor_ui == Render;
  bool modal_blocked = CursorGameInstance::modal_overlay;

  for (size_t i = 0; i < nodes.size(); ++i) {
    SDL_FRect rect = {
        rects[i].x + (int)pos.x,
        rects[i].y + (int)pos.y,
        rects[i].w,
        rects[i].h,
    };

    std::u16string State = u"normal";
    bool mouse_in = SDL_PointInRectFloat(&Window::mouse_pos, &rect);

    if (cursor_on_ui && !modal_blocked && mouse_in) {
      State = mouse_down ? u"pressed" : u"mouseOver";
    }
    auto texture = WzResource::LoadTexture(nodes[i]->find((State + u"/0")));
    SDL_RenderTexture(Window::renderer, texture, nullptr, &rect);
  }
}

static std::u16string close_quest;

void QuestAlarmUiSystem::RenderQuests() {
  close_quest = u"";
  int y = 20;
  for (const auto &id : quests) {
    auto node = QuestGameInstance::LoadQuestNode(id);
    node = node->find(u"QuestInfo/name");
    auto name = static_cast<wz::Property<std::u16string> *>(node)->get();
    if (name.size() > 25) {
      name = name.substr(0, 25) + u"...";
    }
    Freetype::LoadSize(12);
    Freetype::LoadBold(true);
    Freetype::LoadColor(0, 0, 0, 255);
    Freetype::LoadAligned(true);
    Freetype::DrawLine(name, pos.x + 5, pos.y + y);
    Freetype::LoadBold(false);
    static auto close_t = WzResource::LoadTexture(
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
      if (ItemGameInstance::CheckItem(k)) {
        str = ItemGameInstance::LoadItemText(k, u"name");
      } else {
        str = EquipGameInstance::LoadEquipName(k);
      }
      str = std::u16string{tmp.begin(), tmp.end()} + u" " + str;
      Freetype::DrawLine(str, pos.x + 5, pos.y + y);
    }
    for (const auto &[k, v] : quest.check_mob) {
      y += 18;
      uint32_t num = 0;
      if (quest.mob.contains(k)) {
        num = quest.mob.at(k).count;
      }
      auto count = v.count;
      auto tmp = std::format("{}/{}", num, count);
      std::u16string str = MobGameInstance::LoadMobName(k);
      str = std::u16string{tmp.begin(), tmp.end()} + u" " + str;
      Freetype::DrawLine(str, pos.x + 5, pos.y + y);
    }
    y += 18;
  }
}

void QuestAlarmUiSystem::RenderTitle() {
  auto n = WzResource::ms->get_root()->find(u"String.img/Quest/Helper");
  auto str = static_cast<wz::Property<std::u16string> *>(n)->get();
  Freetype::LoadSize(12);
  Freetype::LoadColor(0, 0, 0, 255);
  Freetype::LoadAligned(true);
  auto max_num = 5;
  auto quest_num = quests.size();
  std::string str2 = std::format("({}/{})", quest_num, max_num);
  str = str + std::u16string{str2.begin(), str2.end()};
  Freetype::DrawLine(str, pos.x + 5, pos.y);
}

bool QuestAlarmUiSystem::Render() {
  RenderBackgrnd();
  RenderTitle();
  RenderQuests();
  RenderButton();
  return true;
}

void QuestAlarmUiSystem::EventButtonAuto() { EventAutoQuests(); }

void QuestAlarmUiSystem::EventButtonMax() {}

void QuestAlarmUiSystem::EventButtonMin() {}

void QuestAlarmUiSystem::EventButtonClose() { Close(); }

bool QuestAlarmUiSystem::EventButton(SDL_Event *Event) {
  std::vector<SDL_FRect> r = {
      {173, 4, 21, 12}, //
      {208, 4, 12, 12}, //
  };
  std::vector<void (*)()> fns = {
      EventButtonAuto,
      EventButtonClose,
  };
  if (max) {
    fns.push_back(EventButtonMin);
    r.push_back({195, 4, 12, 12});
  } else {
    fns.push_back(EventButtonMax);
    r.push_back({195, 4, 12, 12});
  }
  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      fns[i]();
      AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }

  return false;
}

void QuestAlarmUiSystem::Open() {
  Close();
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::Render);
  if (it != System::render_systems.end()) {
    System::render_systems.insert(it, Render);
    System::event_systems.insert(System::event_systems.begin(), Event);
  }
}

void QuestAlarmUiSystem::Close() {
  std::erase(System::render_systems, Render);
  std::erase(System::event_systems, Event);

  quests.clear();

  EventDragEnd();
}

void QuestAlarmUiSystem::EventTop() {
  std::erase(System::render_systems, Render);
  std::erase(System::event_systems, Event);
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::Render);
  if (it != System::render_systems.end()) {
    System::render_systems.insert(it, Render);
    System::event_systems.insert(System::event_systems.begin(), Event);
  }
}

void QuestAlarmUiSystem::EventDragStart(SDL_Event *Event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {Event->button.x, Event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - Event->button.x, pos.y - Event->button.y};
  }
  return;
}

void QuestAlarmUiSystem::EventDragEnd() {
  drag = std::nullopt;
  return;
}

void QuestAlarmUiSystem::EventDragMove(SDL_Event *Event) {
  if (drag.has_value()) {
    pos = {Event->motion.x + drag->x, Event->motion.y + drag->y};
    auto &camera = CameraGameInstance::camera;
    auto [w, h] = LoadWh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void QuestAlarmUiSystem::EventQuestClose() {
  if (close_quest.empty()) {
    return;
  }
  quests.erase(close_quest);
}

bool QuestAlarmUiSystem::Event(SDL_Event *Event) {
  bool r = true;
  switch (Event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        EventTop();
        EventDragStart(Event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        EventQuestClose();
        r = !EventButton(Event);
      }
      EventDragEnd();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    EventDragMove(Event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

void QuestAlarmUiSystem::EventAutoQuests() {
  if (quests.empty()) {
    size_t count = 0;
    for (const auto &[key, _] : QuestGameInstance::progress_quests) {
      if (count++ >= 5)
        break;
      quests.emplace(key);
    }
  }
}

bool QuestAlarmUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void QuestAlarmUiSystem::AcceptQuest(const std::u16string &id) {
  auto it = std::ranges::find(System::render_systems, &Render);
  if (it == System::render_systems.end()) {
    return;
  }
  if (quests.size() > 5) {
    return;
  }
  quests.insert(id);
}

void QuestAlarmUiSystem::CompleteQuest(const std::u16string &id) {
  quests.erase(id);
}