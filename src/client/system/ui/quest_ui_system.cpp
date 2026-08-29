#include "quest_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "scroll_ui_system.h"
#include "src/client/game/game_npc.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/npc_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <optional>
#include <string>
#include <vector>

int quest_ui_system::load_vscr_num0() {
  std::flat_map<int8_t, std::vector<game_quest>> quests;
  switch (active_tab) {
  case 0:
    quests = quest_game_instance::load_ui_avaliable_quest();
    break;
  case 1:
    quests = quest_game_instance::load_ui_progress_quest();
    break;
  case 2:
    quests = quest_game_instance::load_ui_complete_quest();
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

int quest_ui_system::load_vscr_num1() { return 0; }

void quest_ui_system::render_backgrnd() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd"));
  static auto backgrnd2 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd2"));
  static auto backgrnd3 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd3"));
  static auto backgrnd4 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd4"));
  static auto backgrnd5 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd5"));
  static auto notice0 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/notice0"));
  static auto notice1 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/notice1"));
  static auto notice2 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/notice2"));
  static auto notice3 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/notice3"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);
}

void quest_ui_system::render_button() {
  std::vector<wz::Node *> buttons_nodes = {
      wz_resource::ui->find(u"Basic.img/BtClose"),
      wz_resource::ui->find(u"Quest.img/Quest/list/BtDetail"),
  };
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{227, 6, 12, 12},   //
      SDL_FRect{183, 374, 57, 17}, //
  };
  if (detail) {
    buttons_nodes.push_back(wz_resource::ui->find(u"Basic.img/BtHide"));
    buttons_rect.push_back(SDL_FRect{530, 6, 13, 12});
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

static std::optional<int> fold;
static std::optional<std::u16string> d_quest;

void quest_ui_system::render_area_name(int i, int y) {
  const SDL_FPoint lt{10, 50};
  auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"Quest.img/Quest/icon/icon0"));
  int px = pos.x + lt.x;
  int py = pos.y + y + lt.y;

  SDL_FRect pos_rect{
      static_cast<float>(px - 2),
      static_cast<float>(py - 2),
      static_cast<float>(215),
      static_cast<float>(18),
  };
  SDL_SetRenderDrawColor(window::renderer, 154, 176, 203, 255);
  SDL_RenderFillRect(window::renderer, &pos_rect);

  std::u16string state = u"normal";

  pos_rect = {
      static_cast<float>(px),
      static_cast<float>(py),
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  bool mouse_in = SDL_PointInRectFloat(&window::mouse_pos, &pos_rect);
  // 渲染所有按钮
  bool mouse_down = window::mouse_state & SDL_BUTTON_LMASK;
  bool cursor_on_ui = cursor_game_instance::cursor_ui == render;
  bool modal_blocked = cursor_game_instance::modal_overlay;
  if (cursor_on_ui && !modal_blocked && mouse_in) {
    state = mouse_down ? u"pressed" : u"mouseOver";
  }
  auto node = wz_resource::ui->find(u"Basic.img/BtMin");
  if (disable_fold.contains(i)) {
    node = wz_resource::ui->find(u"Basic.img/BtMax");
  }
  auto texture = wz_resource::load_texture(node->find((state + u"/0")));
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  if (mouse_in) {
    fold = i;
  }

  auto name = quest_game_instance::load_area_name(i);
  freetype::load_size(12);
  freetype::load_color(255, 255, 255, 255);
  freetype::draw_line(name, px + 17, py - 3);
}

void quest_ui_system::render_quest(game_quest &q, int y) {
  const SDL_FPoint lt{10, 50};
  int px = pos.x + lt.x;
  int py = pos.y + y + lt.y;

  auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"Quest.img/Quest/icon/icon0"));
  SDL_FRect pos_rect{
      static_cast<float>(px),
      static_cast<float>(py),
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);

  if (detail_quest == q.quest_id) {
    pos_rect = {
        static_cast<float>(px + 14),
        static_cast<float>(py - 2),
        static_cast<float>(199),
        static_cast<float>(18),
    };
    SDL_SetRenderDrawColor(window::renderer, 51, 100, 148, 255);
    SDL_RenderFillRect(window::renderer, &pos_rect);
    freetype::load_color(255, 255, 255, 255);
  } else {
    freetype::load_color(0, 0, 0, 255);
  }

  auto node = quest_game_instance::load_quest_node(q.quest_id);
  node = node->find(u"QuestInfo/name");
  auto name = static_cast<wz::Property<std::u16string> *>(node)->get();
  if (name.size() > 30) {
    name = name.substr(0, 30) + u"...";
  }
  freetype::load_size(12);
  freetype::load_aligned(true);
  freetype::draw_line(name, px + 17, py - 3);

  pos_rect = {
      static_cast<float>(px - 2),
      static_cast<float>(py - 2),
      static_cast<float>(215),
      static_cast<float>(22),
  };
  auto &mouse_pos = window::mouse_pos;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    d_quest = q.quest_id;
  }
}

void quest_ui_system::render_quests() {
  const int MAX_DISPLAY_ITEMS = 16;
  std::flat_map<int8_t, std::vector<game_quest>> quests;
  switch (active_tab) {
  case 0:
    quests = quest_game_instance::load_ui_avaliable_quest();
    break;
  case 1:
    quests = quest_game_instance::load_ui_progress_quest();
    break;
  case 2:
    break;
  default:
    return; // 或处理无效tab
  }
  int i = -pages[0];
  fold = std::nullopt;
  d_quest = std::nullopt;
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

void quest_ui_system::render_detail_text() {
  auto index = quest_game_instance::load_quest_progress(detail_quest);
  std::u16string info;
  auto node = quest_game_instance::load_quest_node(detail_quest);
  node = node->get_child(u"QuestInfo");
  node = node->get_child(std::to_string(index));
  if (node) {
    info = text_game_instance::load_rstr(node);
  }
  freetype::load_color(0, 0, 0, 255);
  freetype::draw_rstr(info, pos.x + 260, pos.y + 150, 260, 1.3, std::nullopt);
}

static const SDL_FPoint detail_lt = {245, 0};

void quest_ui_system::render_quest_detail() {
  if (!detail) {
    return;
  }
  static auto backgrnd2 = wz_resource::load_texture(
      wz_resource::ui->find(u"UIWindow.img/Quest/backgrnd2"));
  SDL_FRect pos_rect{
      pos.x + detail_lt.x,
      pos.y,
      static_cast<float>(backgrnd2->w),
      static_cast<float>(backgrnd2->h),
  };
  SDL_RenderTexture(window::renderer, backgrnd2, nullptr, &pos_rect);
  if (detail_quest.empty()) {
    return;
  }
  // render npc
  auto node = quest_game_instance::load_quest_node(detail_quest);
  node = node->find(u"Check/0/npc");
  auto npc_i = static_cast<wz::Property<int> *>(node)->get();
  auto tmp = std::format("{:07d}", npc_i);
  std::u16string npc_id{tmp.begin(), tmp.end()};
  game_npc npc;
  npc.id = npc_id;
  npc.action = u"stand";
  const auto &camera = camera_game_instance::camera;
  npc.pos.x = camera.x + pos.x + 488;
  npc.pos.y = camera.y + pos.y + 115;
  npc.ani_index = 0;
  npc_render_system::render_npc(npc);
  // name
  node = quest_game_instance::load_quest_node(detail_quest);
  node = node->find(u"QuestInfo/name");
  auto name = static_cast<wz::Property<std::u16string> *>(node)->get();
  if (name.size() > 19) {
    name = name.substr(0, 19) + u"...";
  }
  freetype::load_bold(true);
  freetype::load_color(255, 255, 255, 255);
  freetype::load_size(12);
  freetype::draw_line(name, pos.x + 276, pos.y + 36);
  freetype::load_bold(false);

  // level
  freetype::load_size(12);
  node = wz_resource::ms->get_root()->find(u"String.img/Quest/lv");
  auto lv_str = static_cast<wz::Property<std::u16string> *>(node)->get();
  node = quest_game_instance::load_quest_node(detail_quest);
  node = node->find(u"Check/0/lvmin");
  int lv = 1;
  if (node) {
    lv = static_cast<wz::Property<int> *>(node)->get();
  }
  tmp = std::to_string(lv);
  lv_str = lv_str + u" " + std::u16string{tmp.begin(), tmp.end()} + u"+";
  freetype::draw_line(lv_str, pos.x + 269, pos.y + 70);

  freetype::load_size(12);
  auto npc_name = npc_game_instance::load_npc_text(npc_id, u"name");
  node = wz_resource::ms->get_root()->find(u"String.img/Quest/talkNPC");
  auto npc_str = static_cast<wz::Property<std::u16string> *>(node)->get();
  npc_str = npc_str + npc_name;
  freetype::load_bold(true);
  freetype::load_color(0, 0, 193, 255);
  freetype::draw_line(npc_str, pos.x + 260, pos.y + 126);
  freetype::load_bold(false);

  render_detail_text();
}

void quest_ui_system::render_tab() {
  const static std::array tab_pos = {
      SDL_FPoint{6, 24},   //
      SDL_FPoint{78, 24},  //
      SDL_FPoint{150, 24}, //
  };
  const static auto tab_node =
      wz_resource::ui->find(u"Quest.img/Quest/list/Tab");
  const static std::array active_texture = {
      wz_resource::load_texture(tab_node->find(u"enabled/0")),
      wz_resource::load_texture(tab_node->find(u"enabled/1")),
      wz_resource::load_texture(tab_node->find(u"enabled/2")),
  };
  const static std::array disabled_texture = {
      wz_resource::load_texture(tab_node->find(u"disabled/0")),
      wz_resource::load_texture(tab_node->find(u"disabled/1")),
      wz_resource::load_texture(tab_node->find(u"disabled/2")),
  };
  for (uint8_t i = 0; i < tab_pos.size(); i++) {
    SDL_Texture *t = active_tab == i ? active_texture[i] : disabled_texture[i];
    SDL_FRect pos_rect{
        static_cast<float>(int(pos.x + tab_pos[i].x)),
        static_cast<float>(int(pos.y + tab_pos[i].y)),
        static_cast<float>(t->w),
        static_cast<float>(t->h),
    };
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  }
}

void quest_ui_system::render_vscr() {
  const SDL_FPoint lt{225, 48};
  int px = pos.x + lt.x;
  int py = pos.y + lt.y;
  const uint32_t length = 318;
  auto cursor_in = cursor_game_instance::cursor_ui;
  bool top =
      cursor_in == render && cursor_game_instance::modal_overlay == nullptr;
  auto count = load_vscr_num0();
  scroll_ui_system::render_vscroll(px, py, pages[0], count, length, top, 16);

  return;
}

bool quest_ui_system::render() {
  render_backgrnd();
  render_tab();
  render_quests();
  render_quest_detail();
  render_vscr();
  render_button();
  return true;
}

SDL_FPoint quest_ui_system::load_wh() {
  if (detail) {
    return {553, 398};
  } else {
    return {247, 398};
  }
}

void quest_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    detail = true;

    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void quest_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void quest_ui_system::event_tab(SDL_Event *event) {
  const static std::array tab_rect = {
      SDL_FRect{6, 24, 72, 19},   //
      SDL_FRect{78, 24, 72, 19},  //
      SDL_FRect{150, 24, 72, 19}, //
  };
  for (uint8_t i = 0; i < tab_rect.size(); i++) {
    auto pos_rect = tab_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      if (active_tab == i) {
        return;
      }
      disable_fold = {};
      pages = {};
      active_tab = i;
      return;
    }
  }
}

void quest_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void quest_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void quest_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void quest_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void quest_ui_system::toggle() {
  audio_game_instance::load_audio(u"UI.img/BtMouseClick", 0);
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool quest_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void quest_ui_system::event_fold() {
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

void quest_ui_system::event_quest() {
  if (d_quest.has_value()) {
    detail_quest = d_quest.value();
  }
  return;
}

bool quest_ui_system::event_vscr(SDL_Event *event) {
  const SDL_FPoint lt{225, 48};
  int px = pos.x + lt.x;
  int py = pos.y + lt.y;
  const uint32_t length = 318;

  auto size = load_vscr_num0() - 16;
  size = std::max(0, size);

  auto cursor_in = cursor_game_instance::cursor_ui;
  auto mouse_pos = SDL_FPoint{event->button.x, event->button.y};
  bool top = cursor_in == render;
  auto val = scroll_ui_system::click_vscroll(px, py, pages[0], size, length,
                                             top, mouse_pos);
  pages[0] = val;
  return true;
}

void quest_ui_system::event_vscr_move(SDL_Event *event) {
  auto mouse_state = window::mouse_state;
  if (vscr_motion[0]) {
    const SDL_FPoint lt{225, 48};
    const uint32_t length = 318;

    event->button.x = pos.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos.y + lt.y, pos.y + lt.y + length);
    event_vscr(event);
  }
}

void quest_ui_system::event_vscr_start(SDL_Event *event) {
  const SDL_FPoint lt{225, 48};
  const uint32_t length = 318;
  if (vscr_motion[0] == false) {
    vscr_motion[0] =
        scroll_ui_system::click_thumb(pos.x + lt.x, pos.y + lt.y, length);
  }
}

void quest_ui_system::event_vscr_end() { vscr_motion = {}; }

void quest_ui_system::event_button_close() { close(); }

void quest_ui_system::event_button_detail() { detail = !detail; }

bool quest_ui_system::event_button(SDL_Event *event) {
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
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      audio_game_instance::load_audio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }
  return false;
}

bool quest_ui_system::event(SDL_Event *event) {
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
      if (cursor_game_instance::cursor_ui == render) {
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
      if (cursor_game_instance::cursor_ui == render) {
        event_fold();
        event_quest();
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
    // event_motion(event);
    event_drag_move(event);
    event_vscr_move(event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}