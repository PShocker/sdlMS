#include "quest_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "scroll_ui_system.h"
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
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "tooltip_ui_system.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <optional>
#include <string>
#include <vector>

int QuestUiSystem::load_vscr_num0() {
  std::flat_map<int8_t, std::vector<GameQuest>> quests;
  switch (active_tab) {
  case 0:
    quests = QuestGameInstance::load_ui_avaliable_quest();
    break;
  case 1:
    quests = QuestGameInstance::load_ui_progress_quest();
    break;
  case 2:
    quests = QuestGameInstance::load_ui_complete_quest();
    break;
  default: {
    break;
  }
  }
  int count = 0;
  for (auto [k, v] : quests) {
    count++;
    if (!disable_fold.contains(k)) {
      count += v.size();
    }
  }
  return count;
}

int QuestUiSystem::load_vscr_num1() {
  int h = 0;
  auto info = load_detail_str();
  Freetype::load_size(12);
  h += Freetype::load_rh(info, 265, 1.3, std::nullopt);
  if (QuestGameInstance::progress_quests.contains(detail_quest)) {
    h += load_detail_summary_backgrnd_h();
  } else {
    h += 18;
  }
  auto lh = Freetype::load_lh();
  return h / lh;
}

int QuestUiSystem::load_detail_summary_backgrnd_h() {
  if (detail_quest.empty()) {
    return 0;
  }
  auto q = QuestGameInstance::progress_quests.at(detail_quest);
  int h = 0;
  h += q.check_item.size() * 40;
  h += q.check_mob.size() * 25;
  if (h == 0) {
    return 0;
  }
  h += 15;
  return h;
}

void QuestUiSystem::render_backgrnd() {
  static auto backgrnd = WzResource::load_texture(
      WzResource::ui->find(u"UIWindow.img/Quest/backgrnd"));
  static auto backgrnd2 = WzResource::load_texture(
      WzResource::ui->find(u"UIWindow.img/Quest/backgrnd2"));
  static auto backgrnd3 = WzResource::load_texture(
      WzResource::ui->find(u"UIWindow.img/Quest/backgrnd3"));
  static auto backgrnd4 = WzResource::load_texture(
      WzResource::ui->find(u"UIWindow.img/Quest/backgrnd4"));
  static auto backgrnd5 = WzResource::load_texture(
      WzResource::ui->find(u"UIWindow.img/Quest/backgrnd5"));
  static auto notice0 = WzResource::load_texture(
      WzResource::ui->find(u"UIWindow.img/Quest/notice0"));
  static auto notice1 = WzResource::load_texture(
      WzResource::ui->find(u"UIWindow.img/Quest/notice1"));
  static auto notice2 = WzResource::load_texture(
      WzResource::ui->find(u"UIWindow.img/Quest/notice2"));
  static auto notice3 = WzResource::load_texture(
      WzResource::ui->find(u"UIWindow.img/Quest/notice3"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);
}

void QuestUiSystem::render_button() {
  std::vector<wz::Node *> buttons_nodes = {
      WzResource::ui->find(u"Basic.img/BtClose"),
      WzResource::ui->find(u"Quest.img/Quest/list/BtDetail"),
  };
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{227, 6, 12, 12},   //
      SDL_FRect{183, 374, 57, 17}, //
  };
  if (detail) {
    buttons_nodes.push_back(WzResource::ui->find(u"Basic.img/BtHide"));
    buttons_rect.push_back(SDL_FRect{530, 6, 13, 12});
  }

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = Window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
        !CursorGameInstance::modal_overlay) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

static std::optional<int> fold;
static std::optional<std::u16string> d_quest;

void QuestUiSystem::render_area_name(int i, int y) {
  const SDL_FPoint lt{10, 50};
  auto t = WzResource::load_texture(
      WzResource::ui->find(u"Quest.img/Quest/icon/icon0"));
  int px = pos.x + lt.x;
  int py = pos.y + y + lt.y;

  SDL_FRect pos_rect{
      static_cast<float>(px - 2),
      static_cast<float>(py - 2),
      static_cast<float>(215),
      static_cast<float>(18),
  };
  SDL_SetRenderDrawColor(Window::renderer, 154, 176, 203, 255);
  SDL_RenderFillRect(Window::renderer, &pos_rect);

  std::u16string state = u"normal";

  pos_rect = {
      static_cast<float>(px),
      static_cast<float>(py),
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  bool mouse_in = SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect);
  // 渲染所有按钮
  bool mouse_down = Window::mouse_state & SDL_BUTTON_LMASK;
  bool cursor_on_ui = CursorGameInstance::cursor_ui == render;
  bool modal_blocked = CursorGameInstance::modal_overlay;
  if (cursor_on_ui && !modal_blocked && mouse_in) {
    state = mouse_down ? u"pressed" : u"mouseOver";
  }
  auto node = WzResource::ui->find(u"Basic.img/BtMin");
  if (disable_fold.contains(i)) {
    node = WzResource::ui->find(u"Basic.img/BtMax");
  }
  auto texture = WzResource::load_texture(node->find((state + u"/0")));
  SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
  if (mouse_in) {
    fold = i;
  }

  auto name = QuestGameInstance::load_area_name(i);
  Freetype::load_size(12);
  Freetype::load_color(255, 255, 255, 255);
  Freetype::draw_line(name, px + 17, py - 3);
}

static std::u16string quest_name;
static std::u16string checkbox_quest_name;

void QuestUiSystem::render_quest(GameQuest &q, int y) {
  const SDL_FPoint lt{10, 50};
  int px = pos.x + lt.x;
  int py = pos.y + y + lt.y;

  int str_len = 0;
  int str_left = 0;
  switch (active_tab) {
  case 0: {
    str_left = 17;
    str_len = 26;
    static auto t = WzResource::load_texture(
        WzResource::ui->find(u"Quest.img/Quest/icon/icon0"));
    SDL_FRect pos_rect{
        static_cast<float>(px),
        static_cast<float>(py),
        static_cast<float>(t->w),
        static_cast<float>(t->h),
    };
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    break;
  }
  case 1: {
    str_left = 30;
    str_len = 26;
    auto t = WzResource::load_texture(
        WzResource::ui->find(u"Basic.img/CheckBox/0/0"));
    if (QuestAlarmUiSystem::quests.contains(q.quest_id)) {
      t = WzResource::load_texture(
          WzResource::ui->find(u"Basic.img/CheckBox/0/1"));
    }
    SDL_FRect pos_rect{
        static_cast<float>(px),
        static_cast<float>(py),
        static_cast<float>(t->w),
        static_cast<float>(t->h),
    };
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    const auto &mouse_pos = Window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      checkbox_quest_name = q.quest_id;
    }
    std::vector<int> delays = {
        120, 120, 120, 120, 120, 120, 120, 4000,
    };
    auto AnimateData = TrapGameInstance::load_animate_index(delays);
    auto render_index = std::to_string(AnimateData.index);
    auto icon = WzResource::load_texture(
        WzResource::ui->find(u"Quest.img/Quest/icon/icon2")
            ->get_child(render_index));
    pos_rect.x += t->w;
    pos_rect.w = icon->w;
    pos_rect.h = icon->h;
    SDL_RenderTexture(Window::renderer, icon, nullptr, &pos_rect);
    break;
  }
  case 2: {
    break;
  }
  }
  auto node = QuestGameInstance::load_quest_node(q.quest_id);
  node = node->find(u"QuestInfo/name");
  auto name = static_cast<wz::Property<std::u16string> *>(node)->get();
  SDL_FRect pos_rect;
  pos_rect = {
      static_cast<float>(px + str_left - 2),
      static_cast<float>(py - 2),
      static_cast<float>(215 - str_left),
      static_cast<float>(18),
  };
  auto &mouse_pos = Window::mouse_pos;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    quest_name = name;
  }
  if (detail_quest == q.quest_id) {
    SDL_SetRenderDrawColor(Window::renderer, 51, 100, 148, 255);
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    Freetype::load_color(255, 255, 255, 255);
  } else {
    Freetype::load_color(0, 0, 0, 255);
  }
  if (name.size() > str_len) {
    name = name.substr(0, str_len) + u"...";
  }
  Freetype::load_size(12);
  Freetype::load_aligned(true);
  Freetype::draw_line(name, px + str_left, py - 3);

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

void QuestUiSystem::render_quests() {
  const int MAX_DISPLAY_ITEMS = 16;
  std::flat_map<int8_t, std::vector<GameQuest>> quests;
  switch (active_tab) {
  case 0:
    quests = QuestGameInstance::load_ui_avaliable_quest();
    break;
  case 1:
    quests = QuestGameInstance::load_ui_progress_quest();
    break;
  case 2:
    break;
  default:
    return; // 或处理无效tab
  }
  int i = -pages[0];
  fold = std::nullopt;
  d_quest = std::nullopt;
  quest_name = u"";
  checkbox_quest_name = u"";
  for (const auto &[k, v] : quests) {
    if (i >= MAX_DISPLAY_ITEMS)
      break;
    if (i >= 0) {
      render_area_name(k, i * 20);
    }
    i++;
    if (disable_fold.contains(k)) {
      continue;
    }
    for (auto &q : v) {
      if (i >= MAX_DISPLAY_ITEMS)
        break;
      if (i >= 0) {
        render_quest(q, i * 20);
      }
      i++;
    }
  }
  return;
}

void QuestUiSystem::render_detail_avaliable() {
  if (QuestGameInstance::progress_quests.contains(detail_quest) ||
      QuestGameInstance::complete_quests.contains(detail_quest)) {
    return;
  }
  auto node = QuestGameInstance::load_quest_node(detail_quest);
  node = node->find(u"Check/0/npc");
  Freetype::load_size(12);
  if (node != nullptr) {
    auto npc_i = static_cast<wz::Property<int> *>(node)->get();
    auto tmp = std::format("{:07d}", npc_i);
    std::u16string npc_id{tmp.begin(), tmp.end()};
    auto npc_name = NpcGameInstance::load_npc_text(npc_id, u"name");
    node = WzResource::ms->get_root()->find(u"String.img/Quest/talkNPC");
    auto npc_str = static_cast<wz::Property<std::u16string> *>(node)->get();
    npc_str = npc_str + npc_name;
    Freetype::load_bold(true);
    Freetype::load_color(0, 0, 193, 255);
    Freetype::draw_line(npc_str, pos.x + 260, pos.y + 126);
    Freetype::load_bold(false);
  }
  auto info = load_detail_str();
  Freetype::load_color(0, 0, 0, 255);
  Freetype::draw_rstr(info, pos.x + 260, pos.y + 148, 265, 1.3, std::nullopt);
}

void QuestUiSystem::render_detail_progress() {
  if (!QuestGameInstance::progress_quests.contains(detail_quest)) {
    return;
  }
  auto left = 260;
  auto top = 125;
  SDL_Rect clip_rect = {
      static_cast<int>(pos.x + left),
      static_cast<int>(pos.y + top),
      270,
      240,
  };
  // 启用裁剪
  SDL_SetRenderClipRect(Window::renderer, &clip_rect);
  render_detail_summary();
  auto info = load_detail_str();
  int h = 140;
  auto backgrnd_h = load_detail_summary_backgrnd_h();
  if (backgrnd_h == 0) {
    h = 130;
  } else {
    h = h + backgrnd_h;
  }
  Freetype::load_size(12);
  Freetype::load_color(0, 0, 0, 255);
  Freetype::draw_rstr(info, pos.x + 260, pos.y + h - pages[1] * 18, 265, 1.3,
                      std::nullopt);
  SDL_SetRenderClipRect(Window::renderer, NULL);
}

void QuestUiSystem::render_detail_summary_backgrnd() {
  static auto t = WzResource::load_texture(
      WzResource::ui->find(u"Quest.img/Quest/quest_info/summary_pattern/t"));
  static auto c = WzResource::load_texture(
      WzResource::ui->find(u"Quest.img/Quest/quest_info/summary_pattern/c"));
  static auto b = WzResource::load_texture(
      WzResource::ui->find(u"Quest.img/Quest/quest_info/summary_pattern/b"));
  auto left = 260;
  auto top = 125 - pages[1] * 18;
  auto h = load_detail_summary_backgrnd_h();
  SDL_FRect pos_rect{
      static_cast<float>((int)pos.x + left),
      static_cast<float>((int)pos.y + top),
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
  pos_rect.y += t->h;
  pos_rect.h = h;
  SDL_RenderTextureTiled(Window::renderer, c, nullptr, 1.0, &pos_rect);
  pos_rect.y += pos_rect.h;
  pos_rect.h = b->h;
  SDL_RenderTexture(Window::renderer, b, nullptr, &pos_rect);
}

void QuestUiSystem::render_detail_summary() {
  if (!QuestGameInstance::progress_quests.contains(detail_quest)) {
    return;
  }
  auto h = load_detail_summary_backgrnd_h();
  if (h > 0) {
    render_detail_summary_backgrnd();
    auto left = 260;
    auto top = 125 - pages[1] * 18;
    static auto t2 = WzResource::load_texture(WzResource::ui->find(
        u"Quest.img/Quest/quest_info/summary_icon/summary"));
    SDL_FRect pos_rect{
        static_cast<float>((int)pos.x + left + 5),
        static_cast<float>((int)pos.y + top + 5),
        static_cast<float>(t2->w),
        static_cast<float>(t2->h),
    };
    SDL_RenderTexture(Window::renderer, t2, nullptr, &pos_rect);
    auto q = QuestGameInstance::progress_quests.at(detail_quest);
    top = pos_rect.y + pos_rect.h + 5;
    for (auto [k, v] : q.check_mob) {
      auto mob_name = MobGameInstance::load_mob_name(k);
      int num = 0;
      if (q.mob.contains(k)) {
        num = q.mob.at(k).count;
      }
      int count = v.count;
      auto tmp = std::format("{}/{}", num, count);
      mob_name = mob_name + u" " + std::u16string{tmp.begin(), tmp.end()};
      Freetype::load_color(0, 0, 0, 255);
      Freetype::load_size(12);
      Freetype::draw_line(mob_name, pos_rect.x + 5, top);
      top += 25;
    }
    for (auto [k, v] : q.check_item) {
      wz::Node *node;
      std::u16string name;
      if (ItemGameInstance::check_item(k)) {
        node = ItemGameInstance::load_item_info(k, 1);
        name = ItemGameInstance::load_item_text(k, u"name");
      } else {
        node = EquipGameInstance::load_equip_info(k);
        name = EquipGameInstance::load_equip_name(k);
      }
      auto t = WzResource::load_texture(node->get_child(u"icon"));
      SDL_FRect pos_rect{
          static_cast<float>((int)pos.x + left + 5),
          static_cast<float>((int)top),
          static_cast<float>(t->w),
          static_cast<float>(t->h),
      };
      SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
      Freetype::load_size(12);
      Freetype::load_color(0, 0, 0, 255);
      int num = q.item.at(k).count;
      int count = v.count;
      auto tmp = std::format("{}/{}", num, count);
      name = name + u" " + std::u16string{tmp.begin(), tmp.end()};
      Freetype::draw_line(name, pos_rect.x + 5 + t->w,
                          pos_rect.y - 12 + t->h / 2);
      top += 40;
    }
  }
}

std::u16string QuestUiSystem::load_detail_str() {
  if (detail_quest.empty()) {
    return u"";
  }
  auto index = QuestGameInstance::load_quest_progress(detail_quest);
  std::u16string info;
  auto node = QuestGameInstance::load_quest_node(detail_quest);
  node = node->get_child(u"QuestInfo");
  node = node->get_child(std::to_string(index));
  if (node) {
    info = TextGameInstance::load_rstr(node);
  }
  return info;
}

static const SDL_FPoint detail_lt = {245, 0};

void QuestUiSystem::render_quest_detail() {
  if (!detail) {
    return;
  }
  static auto backgrnd2 = WzResource::load_texture(
      WzResource::ui->find(u"UIWindow.img/Quest/backgrnd2"));
  SDL_FRect pos_rect{
      pos.x + detail_lt.x,
      pos.y,
      static_cast<float>(backgrnd2->w),
      static_cast<float>(backgrnd2->h),
  };
  SDL_RenderTexture(Window::renderer, backgrnd2, nullptr, &pos_rect);
  if (detail_quest.empty()) {
    return;
  }
  // render npc
  auto node = QuestGameInstance::load_quest_node(detail_quest);
  node = node->find(u"Check/0/npc");
  if (node != nullptr) {
    auto npc_i = static_cast<wz::Property<int> *>(node)->get();
    auto tmp = std::format("{:07d}", npc_i);
    std::u16string npc_id{tmp.begin(), tmp.end()};
    GameNpc npc;
    npc.id = npc_id;
    npc.action = u"stand";
    const auto &camera = CameraGameInstance::camera;
    npc.pos.x = camera.x + pos.x + 488;
    npc.pos.y = camera.y + pos.y + 115;
    npc.ani_index = 0;
    NpcRenderSystem::render_npc(npc);
  }
  // level
  Freetype::load_size(12);
  Freetype::load_color(255, 255, 255, 255);
  node = WzResource::ms->get_root()->find(u"String.img/Quest/lv");
  auto lv_str = static_cast<wz::Property<std::u16string> *>(node)->get();
  node = QuestGameInstance::load_quest_node(detail_quest);
  node = node->find(u"Check/0/lvmin");
  int lv = 1;
  if (node) {
    lv = static_cast<wz::Property<int> *>(node)->get();
  }
  auto tmp = std::to_string(lv);
  lv_str = lv_str + u" " + std::u16string{tmp.begin(), tmp.end()} + u"+" +
           u" (" + detail_quest + u")";
  Freetype::draw_line(lv_str, pos.x + 269, pos.y + 70);

  // name
  node = QuestGameInstance::load_quest_node(detail_quest);
  node = node->find(u"QuestInfo/name");
  auto name = static_cast<wz::Property<std::u16string> *>(node)->get();
  if (name.size() > 19) {
    name = name.substr(0, 19) + u"...";
  }
  Freetype::load_bold(true);
  Freetype::load_color(255, 255, 255, 255);
  Freetype::load_size(12);
  Freetype::draw_line(name, pos.x + 276, pos.y + 36);
  Freetype::load_bold(false);
  render_detail_avaliable();
  render_detail_progress();
  render_detail_vscr();
}

void QuestUiSystem::render_tab() {
  const static std::array tab_pos = {
      SDL_FPoint{6, 24},   //
      SDL_FPoint{78, 24},  //
      SDL_FPoint{150, 24}, //
  };
  const static auto tab_node =
      WzResource::ui->find(u"Quest.img/Quest/list/Tab");
  const static std::array active_texture = {
      WzResource::load_texture(tab_node->find(u"enabled/0")),
      WzResource::load_texture(tab_node->find(u"enabled/1")),
      WzResource::load_texture(tab_node->find(u"enabled/2")),
  };
  const static std::array disabled_texture = {
      WzResource::load_texture(tab_node->find(u"disabled/0")),
      WzResource::load_texture(tab_node->find(u"disabled/1")),
      WzResource::load_texture(tab_node->find(u"disabled/2")),
  };
  for (uint8_t i = 0; i < tab_pos.size(); i++) {
    SDL_Texture *t = active_tab == i ? active_texture[i] : disabled_texture[i];
    SDL_FRect pos_rect{
        static_cast<float>(int(pos.x + tab_pos[i].x)),
        static_cast<float>(int(pos.y + tab_pos[i].y)),
        static_cast<float>(t->w),
        static_cast<float>(t->h),
    };
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
  }
}

void QuestUiSystem::render_vscr() {
  const SDL_FPoint lt{225, 48};
  int px = pos.x + lt.x;
  int py = pos.y + lt.y;
  const uint32_t length = 318;
  auto cursor_in = CursorGameInstance::cursor_ui;
  bool top =
      cursor_in == render && CursorGameInstance::modal_overlay == nullptr;
  auto count = load_vscr_num0();
  ScrollUiSystem::render_vscroll(px, py, pages[0], count, length, top, 16);

  return;
}

void QuestUiSystem::render_detail_vscr() {
  const SDL_FPoint lt{532, 122};
  int px = pos.x + lt.x;
  int py = pos.y + lt.y;
  const uint32_t length = 244;
  auto cursor_in = CursorGameInstance::cursor_ui;
  bool top =
      cursor_in == render && CursorGameInstance::modal_overlay == nullptr;
  auto count = load_vscr_num1();
  ScrollUiSystem::render_vscroll(px, py, pages[1], count, length, top, 16);

  return;
}

bool QuestUiSystem::render() {
  render_backgrnd();
  render_tab();
  render_quests();
  render_quest_detail();
  render_vscr();
  render_button();
  return true;
}

SDL_FPoint QuestUiSystem::load_wh() {
  if (detail) {
    return {553, 398};
  } else {
    return {247, 398};
  }
}

void QuestUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    detail = true;

    auto wh = load_wh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

void QuestUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::render_systems, render_info);
  std::erase(System::event_systems, event);

  event_drag_end();
  event_vscr_end();
}

void QuestUiSystem::event_tab(SDL_Event *event) {
  const static std::array tab_rect = {
      SDL_FRect{6, 24, 72, 19},   //
      SDL_FRect{78, 24, 72, 19},  //
      SDL_FRect{150, 24, 72, 19}, //
  };
  for (uint8_t i = 0; i < tab_rect.size(); i++) {
    auto pos_rect = tab_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
      if (active_tab == i) {
        return;
      }
      disable_fold = {};
      pages[0] = 0;
      active_tab = i;
      return;
    }
  }
}

void QuestUiSystem::event_top() {
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

void QuestUiSystem::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void QuestUiSystem::event_drag_end() {
  drag = std::nullopt;
  return;
}

void QuestUiSystem::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = CameraGameInstance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void QuestUiSystem::toggle() {
  AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
  auto fn = &render;
  if (std::ranges::contains(System::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool QuestUiSystem::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool QuestUiSystem::event_checkbox() {
  if (!checkbox_quest_name.empty()) {
    auto fn = &QuestAlarmUiSystem::render;
    if (!std::ranges::contains(System::render_systems, fn)) {
      QuestAlarmUiSystem::open();
    }
    if (QuestAlarmUiSystem::quests.contains(checkbox_quest_name)) {
      QuestAlarmUiSystem::quests.erase(checkbox_quest_name);
    } else {
      QuestAlarmUiSystem::quests.insert(checkbox_quest_name);
    }
    return true;
  }
  return false;
}

void QuestUiSystem::event_fold() {
  if (fold.has_value()) {
    if (disable_fold.contains(fold.value())) {
      disable_fold.erase(fold.value());
    } else {
      disable_fold.insert(fold.value());
    }
  }
  auto count = load_vscr_num0();
  count = std::max(count - 16, 0);
  pages[0] = std::min(pages[0], count);
}

void QuestUiSystem::event_quest() {
  if (d_quest.has_value()) {
    detail_quest = d_quest.value();
    pages[1] = 0;
    vscr_motion = {};
    AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
  }
  return;
}

bool QuestUiSystem::event_vscr(SDL_Event *event) {
  SDL_FPoint lt{225, 48};
  int px = pos.x + lt.x;
  int py = pos.y + lt.y;
  uint32_t length = 318;

  auto size = load_vscr_num0() - 16;
  size = std::max(0, size);

  auto mouse_pos = SDL_FPoint{event->button.x, event->button.y};
  bool top = true;
  auto val = ScrollUiSystem::click_vscroll(px, py, pages[0], size, length,
                                             top, mouse_pos);
  pages[0] = val;

  if (detail && !detail_quest.empty()) {
    lt = {532, 122};
    px = pos.x + lt.x;
    py = pos.y + lt.y;
    length = 244;
    size = load_vscr_num1() - 16;
    val = ScrollUiSystem::click_vscroll(px, py, pages[1], size, length, top,
                                          mouse_pos);
    pages[1] = val;
  }

  return true;
}

void QuestUiSystem::event_vscr_move(SDL_Event *event) {
  auto mouse_state = Window::mouse_state;
  if (vscr_motion[0]) {
    const SDL_FPoint lt{225, 48};
    const uint32_t length = 318;

    event->button.x = pos.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos.y + lt.y, pos.y + lt.y + length);
    event_vscr(event);
  }
  if (vscr_motion[1]) {
    const SDL_FPoint lt{532, 122};
    const uint32_t length = 244;

    event->button.x = pos.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos.y + lt.y, pos.y + lt.y + length);
    event_vscr(event);
  }
}

void QuestUiSystem::event_vscr_start(SDL_Event *event) {
  SDL_FPoint lt{225, 48};
  uint32_t length = 318;
  if (vscr_motion[0] == false) {
    vscr_motion[0] =
        ScrollUiSystem::click_thumb(pos.x + lt.x, pos.y + lt.y, length);
  }
  lt = {532, 122};
  length = 244;
  if (vscr_motion[1] == false) {
    vscr_motion[1] =
        ScrollUiSystem::click_thumb(pos.x + lt.x, pos.y + lt.y, length);
  }
}

void QuestUiSystem::event_vscr_end() { vscr_motion = {}; }

void QuestUiSystem::event_button_close() { close(); }

void QuestUiSystem::event_button_detail() { detail = !detail; }

bool QuestUiSystem::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{227, 6, 12, 12},   //
      SDL_FRect{183, 374, 57, 17}, //
  };
  std::vector<void (*)()> fns = {
      event_button_close,
      event_button_detail,
  };

  if (detail) {
    buttons_rect.push_back(SDL_FRect{530, 6, 13, 12});
    fns.push_back(event_button_detail);
  }

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
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

void QuestUiSystem::event_motion_bt_audio(SDL_Event *event) {
  if (CursorGameInstance::cursor_ui != render) {
    return;
  }
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{227, 6, 12, 12},   //
      SDL_FRect{183, 374, 57, 17}, //
  };
  if (detail) {
    buttons_rect.push_back(SDL_FRect{530, 6, 13, 12});
  }
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

bool QuestUiSystem::render_info() {
  if (CursorGameInstance::cursor_ui != render) {
    return true;
  }
  if (!quest_name.empty() && !CursorGameInstance::modal_overlay) {
    auto &mouse_pos = Window::mouse_pos;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 20};
    TooltipUiSystem::render_str(quest_name, show_pos.x, show_pos.y);
  }
  return true;
}

void QuestUiSystem::event_motion(SDL_Event *event) {
  auto &sys = System::render_systems;
  std::erase(sys, render_info);
  auto it = std::ranges::find(sys, &CursorRenderSystem::render);
  if (it != sys.end()) {
    sys.insert(it, render_info);
  }
}

void QuestUiSystem::event_wheel(SDL_Event *event) {
  if (CursorGameInstance::cursor_ui != render) {
    return;
  }
  auto &mouse_pos = Window::mouse_pos;
  SDL_FRect pos_r;
  pos_r.x = (int)pos.x + detail_lt.x;
  pos_r.y = (int)pos.y + detail_lt.y;
  pos_r.w = 305;
  pos_r.h = 396;
  const auto wheel_func = [&](int &page, int size) {
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
  if (detail && detail_in) {
    auto size = load_vscr_num1() - 16;
    size = std::max(0, size);
    wheel_func(pages[1], size);
  } else {
    auto size = load_vscr_num0() - 16;
    size = std::max(0, size);
    wheel_func(pages[0], size);
  }
  return;
}

bool QuestUiSystem::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      close();
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
        event_vscr_start(event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
        event_fold();
        event_quest();
        if (event_checkbox()) {
          return false;
        }
        event_tab(event);
        event_vscr(event);
        r = !event_button(event);
      }
      event_drag_end();
      event_vscr_end();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_drag_move(event);
    event_vscr_move(event);
    event_motion_bt_audio(event);
    event_motion(event);
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

void QuestUiSystem::accept_quest(const std::u16string &id) {
  if (detail_quest == id) {
    detail_quest = u"";
  }
  if (quest_name == id) {
    quest_name = u"";
  }
}

void QuestUiSystem::complete_quest(const std::u16string &id) {
  accept_quest(id);
}