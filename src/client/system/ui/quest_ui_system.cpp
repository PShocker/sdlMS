#include "src/client/system/ui/quest_ui_system.h"

#include "src/client/system/ui/component/button.h"
#include "src/client/game/game_npc.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/game_instance/trap_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/npc_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/quest_alarm_ui_system.h"
#include "src/client/system/ui/scroll_ui_system.h"
#include "src/client/system/ui/tooltip_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

int QuestUiSystem::LoadVscrNum0() {
  std::flat_map<int8_t, std::vector<GameQuest>> quests;
  switch (active_tab_) {
    case 0:
      quests = QuestGameInstance::LoadUiAvaliableQuest();
      break;
    case 1:
      quests = QuestGameInstance::LoadUiProgressQuest();
      break;
    case 2:
      quests = QuestGameInstance::LoadUiCompleteQuest();
      break;
    default: {
      break;
    }
  }
  int count = 0;
  for (auto [k, v] : quests) {
    count++;
    if (!disable_fold_.contains(k)) {
      count += v.size();
    }
  }
  return count;
}

int QuestUiSystem::LoadVscrNum1() {
  int h = 0;
  auto info = LoadDetailStr();
  Freetype::LoadSize(12);
  h += Freetype::LoadRh(info, 265, 1.3, std::nullopt);
  if (QuestGameInstance::progress_quests_.contains(detail_quest_)) {
    h += LoadDetailSummaryBackgrndH();
  } else {
    h += 18;
  }
  auto lh = Freetype::LoadLh();
  return h / lh;
}

int QuestUiSystem::LoadDetailSummaryBackgrndH() {
  if (detail_quest_.empty()) {
    return 0;
  }
  auto q = QuestGameInstance::progress_quests_.at(detail_quest_);
  int h = 0;
  h += q.check_item.size() * 40;
  h += q.check_mob.size() * 25;
  if (h == 0) {
    return 0;
  }
  h += 15;
  return h;
}

void QuestUiSystem::RenderBackgrnd() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIWindow.img/Quest/backgrnd"));
  static auto backgrnd2 = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIWindow.img/Quest/backgrnd2"));
  static auto backgrnd3 = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIWindow.img/Quest/backgrnd3"));
  static auto backgrnd4 = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIWindow.img/Quest/backgrnd4"));
  static auto backgrnd5 = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIWindow.img/Quest/backgrnd5"));
  static auto notice0 = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIWindow.img/Quest/notice0"));
  static auto notice1 = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIWindow.img/Quest/notice1"));
  static auto notice2 = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIWindow.img/Quest/notice2"));
  static auto notice3 = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIWindow.img/Quest/notice3"));
  SDL_FRect pos_rect{
      pos_.x,
      pos_.y,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);
}

void QuestUiSystem::RenderButton() {
  components_.clear();
  std::vector<wz::Node*> buttons_nodes = {
      WzResource::ui_->find(u"Basic.img/BtClose"),
      WzResource::ui_->find(u"Quest.img/Quest/list/BtDetail"),
  };
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{227, 6, 12, 12},    //
      SDL_FRect{183, 374, 57, 17},  //
  };
  if (detail_) {
    buttons_nodes.push_back(WzResource::ui_->find(u"Basic.img/BtHide"));
    buttons_rect.push_back(SDL_FRect{530, 6, 13, 12});
  }

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x += pos_.x;
    rect.y += pos_.y;
    rect.x = (int)rect.x;
    rect.y = (int)rect.y;
    Button button{.node = buttons_nodes[i],
                  .rect = rect,
                  .enable = true,
                  .focused = CursorGameInstance::cursor_ui_ == Render &&
                             !CursorGameInstance::modal_overlay_};
    components_.push_back(button);
    button.Render();
  }
}

std::vector<Button>& QuestUiSystem::GetComponent() {
  return components_;
}


static std::optional<int> fold;
static std::optional<std::u16string> d_quest;

void QuestUiSystem::RenderAreaName(int i, int y) {
  const SDL_FPoint lt{10, 50};
  auto t = WzResource::LoadTexture(
      WzResource::ui_->find(u"Quest.img/Quest/icon/icon0"));
  int px = pos_.x + lt.x;
  int py = pos_.y + y + lt.y;

  SDL_FRect pos_rect{
      static_cast<float>(px - 2),
      static_cast<float>(py - 2),
      static_cast<float>(215),
      static_cast<float>(18),
  };
  SDL_SetRenderDrawColor(Window::renderer_, 154, 176, 203, 255);
  SDL_RenderFillRect(Window::renderer_, &pos_rect);

  std::u16string state = u"normal";

  pos_rect = {
      static_cast<float>(px),
      static_cast<float>(py),
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  bool mouse_in = SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect);
  // 渲染所有按钮
  bool mouse_down = Window::mouse_state_ & SDL_BUTTON_LMASK;
  bool cursor_on_ui = CursorGameInstance::cursor_ui_ == Render;
  bool modal_blocked = CursorGameInstance::modal_overlay_;
  if (cursor_on_ui && !modal_blocked && mouse_in) {
    state = mouse_down ? u"pressed" : u"mouseOver";
  }
  auto node = WzResource::ui_->find(u"Basic.img/BtMin");
  if (disable_fold_.contains(i)) {
    node = WzResource::ui_->find(u"Basic.img/BtMax");
  }
  auto texture = WzResource::LoadTexture(node->find((state + u"/0")));
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  if (mouse_in) {
    fold = i;
  }

  auto name = QuestGameInstance::LoadAreaName(i);
  Freetype::LoadSize(12);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(name, px + 17, py - 3);
}

static std::u16string quest_name;
static std::u16string checkbox_quest_name;

void QuestUiSystem::RenderQuest(GameQuest& q, int y) {
  const SDL_FPoint lt{10, 50};
  int px = pos_.x + lt.x;
  int py = pos_.y + y + lt.y;

  int str_len = 0;
  int str_left = 0;
  switch (active_tab_) {
    case 0: {
      str_left = 17;
      str_len = 26;
      static auto t = WzResource::LoadTexture(
          WzResource::ui_->find(u"Quest.img/Quest/icon/icon0"));
      SDL_FRect pos_rect{
          static_cast<float>(px),
          static_cast<float>(py),
          static_cast<float>(t->w),
          static_cast<float>(t->h),
      };
      SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
      break;
    }
    case 1: {
      str_left = 30;
      str_len = 26;
      auto t = WzResource::LoadTexture(
          WzResource::ui_->find(u"Basic.img/CheckBox/0/0"));
      if (QuestAlarmUiSystem::quests_.contains(q.quest_id)) {
        t = WzResource::LoadTexture(
            WzResource::ui_->find(u"Basic.img/CheckBox/0/1"));
      }
      SDL_FRect pos_rect{
          static_cast<float>(px),
          static_cast<float>(py),
          static_cast<float>(t->w),
          static_cast<float>(t->h),
      };
      SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
      const auto& mouse_pos = Window::mouse_pos_;
      if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
        checkbox_quest_name = q.quest_id;
      }
      std::vector<int> delays = {
          120, 120, 120, 120, 120, 120, 120, 4000,
      };
      auto AnimateData = TrapGameInstance::LoadAnimateIndex(delays);
      auto render_index = std::to_string(AnimateData.index);
      auto icon = WzResource::LoadTexture(
          WzResource::ui_->find(u"Quest.img/Quest/icon/icon2")
              ->get_child(render_index));
      pos_rect.x += t->w;
      pos_rect.w = icon->w;
      pos_rect.h = icon->h;
      SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
      break;
    }
    case 2: {
      break;
    }
  }
  auto node = QuestGameInstance::LoadQuestNode(q.quest_id);
  node = node->find(u"QuestInfo/name");
  auto name = static_cast<wz::Property<std::u16string>*>(node)->get();
  SDL_FRect pos_rect;
  pos_rect = {
      static_cast<float>(px + str_left - 2),
      static_cast<float>(py - 2),
      static_cast<float>(215 - str_left),
      static_cast<float>(18),
  };
  auto& mouse_pos = Window::mouse_pos_;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    quest_name = name;
  }
  if (detail_quest_ == q.quest_id) {
    SDL_SetRenderDrawColor(Window::renderer_, 51, 100, 148, 255);
    SDL_RenderFillRect(Window::renderer_, &pos_rect);
    Freetype::LoadColor(255, 255, 255, 255);
  } else {
    Freetype::LoadColor(0, 0, 0, 255);
  }
  if (name.size() > str_len) {
    name = name.substr(0, str_len) + u"...";
  }
  Freetype::LoadSize(12);
  Freetype::LoadAligned(true);
  Freetype::DrawLine(name, px + str_left, py - 3);

  pos_rect = {
      static_cast<float>(px - 2),
      static_cast<float>(py - 2),
      static_cast<float>(215),
      static_cast<float>(22),
  };
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    d_quest = q.quest_id;
  }
}

void QuestUiSystem::RenderQuests() {
  const int MAX_DISPLAY_ITEMS = 16;
  std::flat_map<int8_t, std::vector<GameQuest>> quests;
  switch (active_tab_) {
    case 0:
      quests = QuestGameInstance::LoadUiAvaliableQuest();
      break;
    case 1:
      quests = QuestGameInstance::LoadUiProgressQuest();
      break;
    case 2:
      break;
    default:
      return;  // 或处理无效tab
  }
  int i = -pages_[0];
  fold = std::nullopt;
  d_quest = std::nullopt;
  quest_name = u"";
  checkbox_quest_name = u"";
  for (const auto& [k, v] : quests) {
    if (i >= MAX_DISPLAY_ITEMS)
      break;
    if (i >= 0) {
      RenderAreaName(k, i * 20);
    }
    i++;
    if (disable_fold_.contains(k)) {
      continue;
    }
    for (auto& q : v) {
      if (i >= MAX_DISPLAY_ITEMS)
        break;
      if (i >= 0) {
        RenderQuest(q, i * 20);
      }
      i++;
    }
  }
  return;
}

void QuestUiSystem::RenderDetailAvaliable() {
  if (QuestGameInstance::progress_quests_.contains(detail_quest_) ||
      QuestGameInstance::complete_quests_.contains(detail_quest_)) {
    return;
  }
  auto node = QuestGameInstance::LoadQuestNode(detail_quest_);
  node = node->find(u"Check/0/npc");
  Freetype::LoadSize(12);
  if (node != nullptr) {
    auto npc_i = static_cast<wz::Property<int>*>(node)->get();
    auto tmp = std::format("{:07d}", npc_i);
    std::u16string npc_id{tmp.begin(), tmp.end()};
    auto npc_name = NpcGameInstance::LoadNpcText(npc_id, u"name");
    node = WzResource::ms_->get_root()->find(u"String.img/Quest/talkNPC");
    auto npc_str = static_cast<wz::Property<std::u16string>*>(node)->get();
    npc_str = npc_str + npc_name;
    Freetype::LoadBold(true);
    Freetype::LoadColor(0, 0, 193, 255);
    Freetype::DrawLine(npc_str, pos_.x + 260, pos_.y + 126);
    Freetype::LoadBold(false);
  }
  auto info = LoadDetailStr();
  Freetype::LoadColor(0, 0, 0, 255);
  Freetype::DrawRstr(info, pos_.x + 260, pos_.y + 148, 265, 1.3, std::nullopt);
}

void QuestUiSystem::RenderDetailProgress() {
  if (!QuestGameInstance::progress_quests_.contains(detail_quest_)) {
    return;
  }
  auto left = 260;
  auto top = 125;
  SDL_Rect clip_rect = {
      static_cast<int>(pos_.x + left),
      static_cast<int>(pos_.y + top),
      270,
      240,
  };
  // 启用裁剪
  SDL_SetRenderClipRect(Window::renderer_, &clip_rect);
  RenderDetailSummary();
  auto info = LoadDetailStr();
  int h = 140;
  auto backgrnd_h = LoadDetailSummaryBackgrndH();
  if (backgrnd_h == 0) {
    h = 130;
  } else {
    h = h + backgrnd_h;
  }
  Freetype::LoadSize(12);
  Freetype::LoadColor(0, 0, 0, 255);
  Freetype::DrawRstr(info, pos_.x + 260, pos_.y + h - pages_[1] * 18, 265, 1.3,
                     std::nullopt);
  SDL_SetRenderClipRect(Window::renderer_, NULL);
}

void QuestUiSystem::RenderDetailSummaryBackgrnd() {
  static auto t = WzResource::LoadTexture(
      WzResource::ui_->find(u"Quest.img/Quest/quest_info/summary_pattern/t"));
  static auto c = WzResource::LoadTexture(
      WzResource::ui_->find(u"Quest.img/Quest/quest_info/summary_pattern/c"));
  static auto b = WzResource::LoadTexture(
      WzResource::ui_->find(u"Quest.img/Quest/quest_info/summary_pattern/b"));
  auto left = 260;
  auto top = 125 - pages_[1] * 18;
  auto h = LoadDetailSummaryBackgrndH();
  SDL_FRect pos_rect{
      static_cast<float>((int)pos_.x + left),
      static_cast<float>((int)pos_.y + top),
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
  pos_rect.y += t->h;
  pos_rect.h = h;
  SDL_RenderTextureTiled(Window::renderer_, c, nullptr, 1.0, &pos_rect);
  pos_rect.y += pos_rect.h;
  pos_rect.h = b->h;
  SDL_RenderTexture(Window::renderer_, b, nullptr, &pos_rect);
}

void QuestUiSystem::RenderDetailSummary() {
  if (!QuestGameInstance::progress_quests_.contains(detail_quest_)) {
    return;
  }
  auto h = LoadDetailSummaryBackgrndH();
  if (h > 0) {
    RenderDetailSummaryBackgrnd();
    auto left = 260;
    auto top = 125 - pages_[1] * 18;
    static auto t2 = WzResource::LoadTexture(WzResource::ui_->find(
        u"Quest.img/Quest/quest_info/summary_icon/summary"));
    SDL_FRect pos_rect{
        static_cast<float>((int)pos_.x + left + 5),
        static_cast<float>((int)pos_.y + top + 5),
        static_cast<float>(t2->w),
        static_cast<float>(t2->h),
    };
    SDL_RenderTexture(Window::renderer_, t2, nullptr, &pos_rect);
    auto q = QuestGameInstance::progress_quests_.at(detail_quest_);
    top = pos_rect.y + pos_rect.h + 5;
    for (auto [k, v] : q.check_mob) {
      auto mob_name = MobGameInstance::LoadMobName(k);
      int num = 0;
      if (q.mob.contains(k)) {
        num = q.mob.at(k).count;
      }
      int count = v.count;
      auto tmp = std::format("{}/{}", num, count);
      mob_name = mob_name + u" " + std::u16string{tmp.begin(), tmp.end()};
      Freetype::LoadColor(0, 0, 0, 255);
      Freetype::LoadSize(12);
      Freetype::DrawLine(mob_name, pos_rect.x + 5, top);
      top += 25;
    }
    for (auto [k, v] : q.check_item) {
      wz::Node* node;
      std::u16string name;
      if (ItemGameInstance::CheckItem(k)) {
        node = ItemGameInstance::LoadItemInfo(k, 1);
        name = ItemGameInstance::LoadItemText(k, u"name");
      } else {
        node = EquipGameInstance::LoadEquipInfo(k);
        name = EquipGameInstance::LoadEquipName(k);
      }
      auto t = WzResource::LoadTexture(node->get_child(u"icon"));
      SDL_FRect pos_rect{
          static_cast<float>((int)pos_.x + left + 5),
          static_cast<float>((int)top),
          static_cast<float>(t->w),
          static_cast<float>(t->h),
      };
      SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
      Freetype::LoadSize(12);
      Freetype::LoadColor(0, 0, 0, 255);
      int num = q.item.at(k).count;
      int count = v.count;
      auto tmp = std::format("{}/{}", num, count);
      name = name + u" " + std::u16string{tmp.begin(), tmp.end()};
      Freetype::DrawLine(name, pos_rect.x + 5 + t->w,
                         pos_rect.y - 12 + t->h / 2);
      top += 40;
    }
  }
}

std::u16string QuestUiSystem::LoadDetailStr() {
  if (detail_quest_.empty()) {
    return u"";
  }
  auto index = QuestGameInstance::LoadQuestProgress(detail_quest_);
  std::u16string info;
  auto node = QuestGameInstance::LoadQuestNode(detail_quest_);
  node = node->get_child(u"QuestInfo");
  node = node->get_child(std::to_string(index));
  if (node) {
    info = TextGameInstance::LoadRstr(node);
  }
  return info;
}

static const SDL_FPoint detail_lt = {245, 0};

void QuestUiSystem::RenderQuestDetail() {
  if (!detail_) {
    return;
  }
  static auto backgrnd2 = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIWindow.img/Quest/backgrnd2"));
  SDL_FRect pos_rect{
      pos_.x + detail_lt.x,
      pos_.y,
      static_cast<float>(backgrnd2->w),
      static_cast<float>(backgrnd2->h),
  };
  SDL_RenderTexture(Window::renderer_, backgrnd2, nullptr, &pos_rect);
  if (detail_quest_.empty()) {
    return;
  }
  // render npc
  auto node = QuestGameInstance::LoadQuestNode(detail_quest_);
  node = node->find(u"Check/0/npc");
  if (node != nullptr) {
    auto npc_i = static_cast<wz::Property<int>*>(node)->get();
    auto tmp = std::format("{:07d}", npc_i);
    std::u16string npc_id{tmp.begin(), tmp.end()};
    GameNpc npc;
    npc.id = npc_id;
    npc.action = u"stand";
    const auto& camera = CameraGameInstance::camera_;
    npc.pos.x = camera.x + pos_.x + 488;
    npc.pos.y = camera.y + pos_.y + 115;
    npc.ani_index = 0;
    NpcRenderSystem::RenderNpc(npc);
  }
  // level
  Freetype::LoadSize(12);
  Freetype::LoadColor(255, 255, 255, 255);
  node = WzResource::ms_->get_root()->find(u"String.img/Quest/lv");
  auto lv_str = static_cast<wz::Property<std::u16string>*>(node)->get();
  node = QuestGameInstance::LoadQuestNode(detail_quest_);
  node = node->find(u"Check/0/lvmin");
  int lv = 1;
  if (node) {
    lv = static_cast<wz::Property<int>*>(node)->get();
  }
  auto tmp = std::to_string(lv);
  lv_str = lv_str + u" " + std::u16string{tmp.begin(), tmp.end()} + u"+" +
           u" (" + detail_quest_ + u")";
  Freetype::DrawLine(lv_str, pos_.x + 269, pos_.y + 70);

  // name
  node = QuestGameInstance::LoadQuestNode(detail_quest_);
  node = node->find(u"QuestInfo/name");
  auto name = static_cast<wz::Property<std::u16string>*>(node)->get();
  if (name.size() > 19) {
    name = name.substr(0, 19) + u"...";
  }
  Freetype::LoadBold(true);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::LoadSize(12);
  Freetype::DrawLine(name, pos_.x + 276, pos_.y + 36);
  Freetype::LoadBold(false);
  RenderDetailAvaliable();
  RenderDetailProgress();
  RenderDetailVscr();
}

void QuestUiSystem::RenderTab() {
  const static std::array tab_pos = {
      SDL_FPoint{6, 24},    //
      SDL_FPoint{78, 24},   //
      SDL_FPoint{150, 24},  //
  };
  const static auto tab_node =
      WzResource::ui_->find(u"Quest.img/Quest/list/Tab");
  const static std::array active_texture = {
      WzResource::LoadTexture(tab_node->find(u"enabled/0")),
      WzResource::LoadTexture(tab_node->find(u"enabled/1")),
      WzResource::LoadTexture(tab_node->find(u"enabled/2")),
  };
  const static std::array disabled_texture = {
      WzResource::LoadTexture(tab_node->find(u"disabled/0")),
      WzResource::LoadTexture(tab_node->find(u"disabled/1")),
      WzResource::LoadTexture(tab_node->find(u"disabled/2")),
  };
  for (uint8_t i = 0; i < tab_pos.size(); i++) {
    SDL_Texture* t = active_tab_ == i ? active_texture[i] : disabled_texture[i];
    SDL_FRect pos_rect{
        static_cast<float>(int(pos_.x + tab_pos[i].x)),
        static_cast<float>(int(pos_.y + tab_pos[i].y)),
        static_cast<float>(t->w),
        static_cast<float>(t->h),
    };
    SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
  }
}

void QuestUiSystem::RenderVscr() {
  const SDL_FPoint lt{225, 48};
  int px = pos_.x + lt.x;
  int py = pos_.y + lt.y;
  const uint32_t length = 318;
  auto cursor_in = CursorGameInstance::cursor_ui_;
  bool top =
      cursor_in == Render && CursorGameInstance::modal_overlay_ == nullptr;
  auto count = LoadVscrNum0();
  ScrollUiSystem::RenderVscroll(px, py, pages_[0], count, length, top, 16);

  return;
}

void QuestUiSystem::RenderDetailVscr() {
  const SDL_FPoint lt{532, 122};
  int px = pos_.x + lt.x;
  int py = pos_.y + lt.y;
  const uint32_t length = 244;
  auto cursor_in = CursorGameInstance::cursor_ui_;
  bool top =
      cursor_in == Render && CursorGameInstance::modal_overlay_ == nullptr;
  auto count = LoadVscrNum1();
  ScrollUiSystem::RenderVscroll(px, py, pages_[1], count, length, top, 16);

  return;
}

bool QuestUiSystem::Render() {
  RenderBackgrnd();
  RenderTab();
  RenderQuests();
  RenderQuestDetail();
  RenderVscr();
  RenderButton();
  return true;
}

SDL_FPoint QuestUiSystem::LoadWh() {
  if (detail_) {
    return {553, 398};
  } else {
    return {247, 398};
  }
}

void QuestUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    detail_ = true;

    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);

    EventMotion(nullptr);
  }
}

void QuestUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::render_systems_, RenderInfo);
  std::erase(System::event_systems_, Event);

  EventDragEnd();
  EventVscrEnd();
}

void QuestUiSystem::EventTab(SDL_Event* event) {
  const static std::array tab_rect = {
      SDL_FRect{6, 24, 72, 19},    //
      SDL_FRect{78, 24, 72, 19},   //
      SDL_FRect{150, 24, 72, 19},  //
  };
  for (uint8_t i = 0; i < tab_rect.size(); i++) {
    auto pos_rect = tab_rect[i];
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
      if (active_tab_ == i) {
        return;
      }
      disable_fold_ = {};
      pages_[0] = 0;
      active_tab_ = i;
      return;
    }
  }
}

void QuestUiSystem::EventTop() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);

    EventMotion(nullptr);
  }
}

void QuestUiSystem::EventDragStart(SDL_Event* event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos_.x, pos_.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - event->button.x, pos_.y - event->button.y};
  }
  return;
}

void QuestUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
  return;
}

void QuestUiSystem::EventDragMove(SDL_Event* event) {
  if (drag_.has_value()) {
    pos_ = {event->motion.x + drag_->x, event->motion.y + drag_->y};
    auto& camera = CameraGameInstance::camera_;
    auto [w, h] = LoadWh();
    pos_.x = std::clamp(pos_.x, (float)0, camera.w - w);
    pos_.y = std::clamp(pos_.y, (float)0, camera.h - h);
  }
  return;
}

void QuestUiSystem::Toggle() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

bool QuestUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool QuestUiSystem::EventCheckbox() {
  if (!checkbox_quest_name.empty()) {
    auto fn = &QuestAlarmUiSystem::Render;
    if (!std::ranges::contains(System::render_systems_, fn)) {
      QuestAlarmUiSystem::Open();
    }
    if (QuestAlarmUiSystem::quests_.contains(checkbox_quest_name)) {
      QuestAlarmUiSystem::quests_.erase(checkbox_quest_name);
    } else {
      QuestAlarmUiSystem::quests_.insert(checkbox_quest_name);
    }
    return true;
  }
  return false;
}

void QuestUiSystem::EventFold() {
  if (fold.has_value()) {
    if (disable_fold_.contains(fold.value())) {
      disable_fold_.erase(fold.value());
    } else {
      disable_fold_.insert(fold.value());
    }
  }
  auto count = LoadVscrNum0();
  count = std::max(count - 16, 0);
  pages_[0] = std::min(pages_[0], count);
}

void QuestUiSystem::EventQuest() {
  if (d_quest.has_value()) {
    detail_quest_ = d_quest.value();
    pages_[1] = 0;
    vscr_motion_ = {};
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
  }
  return;
}

bool QuestUiSystem::EventVscr(SDL_Event* event) {
  SDL_FPoint lt{225, 48};
  int px = pos_.x + lt.x;
  int py = pos_.y + lt.y;
  uint32_t length = 318;

  auto size = LoadVscrNum0() - 16;
  size = std::max(0, size);

  auto mouse_pos = SDL_FPoint{event->button.x, event->button.y};
  bool top = true;
  auto val = ScrollUiSystem::ClickVscroll(px, py, pages_[0], size, length, top,
                                          mouse_pos);
  pages_[0] = val;

  if (detail_ && !detail_quest_.empty()) {
    lt = {532, 122};
    px = pos_.x + lt.x;
    py = pos_.y + lt.y;
    length = 244;
    size = LoadVscrNum1() - 16;
    val = ScrollUiSystem::ClickVscroll(px, py, pages_[1], size, length, top,
                                       mouse_pos);
    pages_[1] = val;
  }

  return true;
}

void QuestUiSystem::EventVscrMove(SDL_Event* event) {
  auto mouse_state = Window::mouse_state_;
  if (vscr_motion_[0]) {
    const SDL_FPoint lt{225, 48};
    const uint32_t length = 318;

    event->button.x = pos_.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos_.y + lt.y, pos_.y + lt.y + length);
    EventVscr(event);
  }
  if (vscr_motion_[1]) {
    const SDL_FPoint lt{532, 122};
    const uint32_t length = 244;

    event->button.x = pos_.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos_.y + lt.y, pos_.y + lt.y + length);
    EventVscr(event);
  }
}

void QuestUiSystem::EventVscrStart(SDL_Event* event) {
  SDL_FPoint lt{225, 48};
  uint32_t length = 318;
  if (vscr_motion_[0] == false) {
    vscr_motion_[0] =
        ScrollUiSystem::ClickThumb(pos_.x + lt.x, pos_.y + lt.y, length);
  }
  lt = {532, 122};
  length = 244;
  if (vscr_motion_[1] == false) {
    vscr_motion_[1] =
        ScrollUiSystem::ClickThumb(pos_.x + lt.x, pos_.y + lt.y, length);
  }
}

void QuestUiSystem::EventVscrEnd() {
  vscr_motion_ = {};
}

void QuestUiSystem::EventButtonClose() {
  Close();
}

void QuestUiSystem::EventButtonDetail() {
  detail_ = !detail_;
}

bool QuestUiSystem::EventButton(SDL_Event* event) {
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{227, 6, 12, 12},    //
      SDL_FRect{183, 374, 57, 17},  //
  };
  std::vector<void (*)()> fns = {
      EventButtonClose,
      EventButtonDetail,
  };

  if (detail_) {
    buttons_rect.push_back(SDL_FRect{530, 6, 13, 12});
    fns.push_back(EventButtonDetail);
  }

  for (size_t i = 0; i < components_.size() && i < fns.size(); ++i) {
    components_[i].callback = fns[i];
  }
  if (DispatchButtonClick(components_, true)) {
    return true;
  }
  return false;
}

void QuestUiSystem::EventMotionBtAudio(SDL_Event* event) {
  if (CursorGameInstance::cursor_ui_ != Render) {
    return;
  }
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{227, 6, 12, 12},    //
      SDL_FRect{183, 374, 57, 17},  //
  };
  if (detail_) {
    buttons_rect.push_back(SDL_FRect{530, 6, 13, 12});
  }
  static int8_t bt_audio = -1;
  auto& mouse_pos = Window::mouse_pos_;
  int8_t bt_in = -1;
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos_.x;
    pos_rect.y += (int)pos_.y;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      bt_in = i;
      break;
    }
  }
  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

bool QuestUiSystem::RenderInfo() {
  if (CursorGameInstance::cursor_ui_ != Render) {
    return true;
  }
  if (!quest_name.empty() && !CursorGameInstance::modal_overlay_) {
    auto& mouse_pos = Window::mouse_pos_;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 20};
    TooltipUiSystem::RenderStr(quest_name, show_pos.x, show_pos.y);
  }
  return true;
}

void QuestUiSystem::EventMotion(SDL_Event* event) {
  auto& sys = System::render_systems_;
  std::erase(sys, RenderInfo);
  auto it = std::ranges::find(sys, &CursorRenderSystem::Render);
  if (it != sys.end()) {
    sys.insert(it, RenderInfo);
  }
}

void QuestUiSystem::EventWheel(SDL_Event* event) {
  if (CursorGameInstance::cursor_ui_ != Render) {
    return;
  }
  auto& mouse_pos = Window::mouse_pos_;
  SDL_FRect pos_r;
  pos_r.x = (int)pos_.x + detail_lt.x;
  pos_r.y = (int)pos_.y + detail_lt.y;
  pos_r.w = 305;
  pos_r.h = 396;
  const auto wheel_func = [&](int& page, int size) {
    auto dy = event->wheel.integer_y;
    if (page > 0 && dy > 0) {
      page--;
    }
    if (page < size && dy < 0) {
      page++;
    }
    return;
  };
  bool detail_in = SDL_PointInRectFloat(&mouse_pos, &pos_r);
  if (detail_ && detail_in) {
    auto size = LoadVscrNum1() - 16;
    size = std::max(0, size);
    wheel_func(pages_[1], size);
  } else {
    auto size = LoadVscrNum0() - 16;
    size = std::max(0, size);
    wheel_func(pages_[0], size);
  }
  return;
}

bool QuestUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      auto scan_code = event->key.scancode;
      switch (scan_code) {
        case SDL_SCANCODE_ESCAPE: {
          Close();
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
        if (CursorGameInstance::cursor_ui_ == Render) {
          EventTop();
          EventDragStart(event);
          EventVscrStart(event);
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          EventFold();
          EventQuest();
          if (EventCheckbox()) {
            return false;
          }
          EventTab(event);
          EventVscr(event);
          r = !EventButton(event);
        }
        EventDragEnd();
        EventVscrEnd();
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      EventDragMove(event);
      EventVscrMove(event);
      EventMotionBtAudio(event);
      EventMotion(event);
      break;
    }
    case SDL_EVENT_MOUSE_WHEEL: {
      EventWheel(event);
      break;
    }
    default: {
      break;
    }
  }

  return r;
}

void QuestUiSystem::AcceptQuest(const std::u16string& id) {
  if (detail_quest_ == id) {
    detail_quest_ = u"";
  }
  if (quest_name == id) {
    quest_name = u"";
  }
}

void QuestUiSystem::CompleteQuest(const std::u16string& id) {
  AcceptQuest(id);
}

}  // namespace sdlms
