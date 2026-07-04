#include "npc_dlg_ui_system.h"
#include "src/client/game/game_npc.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/system/render/npc_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

void npc_dlg_ui_system::render_backgrnd() {
  auto [w, h] = load_wh();
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/t"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  static auto c = wz_resource::load_texture(
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/c"));
  static auto s = wz_resource::load_texture(
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/s"));
  pos_rect = {
      pos.x,
      pos.y + t->h,
      static_cast<float>(c->w),
      static_cast<float>(h - t->h - s->h),
  };
  SDL_RenderTextureTiled(window::renderer, c, nullptr, 1, &pos_rect);

  pos_rect = {
      pos.x,
      pos.y + h - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_RenderTexture(window::renderer, s, nullptr, &pos_rect);
}

void npc_dlg_ui_system::render_npc() {
  auto [w, h] = load_wh();
  game_npc npc;
  npc.action = u"stand";
  const auto &camera = camera_game_instance::camera;
  npc.pos.x = camera.x + pos.x + 80;
  npc.pos.y = camera.y + pos.y + 15 + h / 2;
  npc.ani_index = 0;
  npc.id = npc_id;
  npc_render_system::render_npc(npc);

  //   render bar
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/bar"));
  SDL_FRect pos_rect{
      pos.x + 80 - t->w / 2,
      pos.y + 15 + h / 2,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);

  freetype::load_size(12);
  freetype::load_aligned(true);
  freetype::load_color(255, 255, 255, 255);
  auto npc_name = npc_game_instance::load_npc_text(npc.id, u"name");
  w = freetype::load_w(npc_name);
  freetype::draw_line(npc_name, pos_rect.x + 61 - w / 2, pos_rect.y);
  freetype::load_aligned(false);
}

void npc_dlg_ui_system::render_button() {
  auto [w, h] = load_wh();
  std::vector<wz::Node *> buttons_node = {
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/BtClose"),
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/BtOK"),
  };
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{10, h - 25, 85, 18},     //
      SDL_FRect{w - 56, h - 25, 46, 18}, //
  };
  auto w0 = w - 72;
  if (index < max_index) {
    buttons_node.push_back(
        wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/BtNext"));
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
    w0 -= 52;
  }
  if (index > 0) {
    buttons_node.push_back(
        wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/BtPrev"));
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
  }

  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    auto &mouse_pos = window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = cursor_game_instance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render) {
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

void npc_dlg_ui_system::render_text() {
  auto dt = window::dt_now - time;
  dt = dt / 45;
  dt = std::clamp(dt, (uint64_t)1, (uint64_t)text.size());
  auto str = text.substr(0, dt);
  switch (type) {
  case npc_dlg_enum::choose: {
    break;
  }
  case npc_dlg_enum::quest:
  case npc_dlg_enum::talk: {
    freetype::load_size(12);
    freetype::load_aligned(true);
    freetype::load_color(0, 0, 0, 255);
    freetype::draw_rstr(str, pos.x + 165, pos.y + 30, 330);
    freetype::load_aligned(false);
    break;
  }
  case npc_dlg_enum::select: {
    break;
  }
  }
}

void npc_dlg_ui_system::render_list() {
  if (type != npc_dlg_enum::quest) {
    return;
  }
  if (index != 0) {
    return;
  }
  auto [w, h] = load_wh();
  static auto t0 = wz_resource::load_texture(
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/list0"));
  static auto t1 = wz_resource::load_texture(
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/list1"));
  static auto t2 = wz_resource::load_texture(
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/list2"));
  SDL_FRect pos_rect{
      pos.x + 165,
      pos.y + h - 130,
      static_cast<float>(t1->w),
      static_cast<float>(t1->h),
  };
  SDL_RenderTexture(window::renderer, t1, nullptr, &pos_rect);
  auto quests = quest_game_instance::load_npc_quest(npc_id);
  freetype::load_size(12);
  freetype::load_aligned(true);
  freetype::load_color(128, 0, 128, 255);
  auto lh = freetype::load_lh();
  for (int i = 0; i < quests.size(); i++) {
    auto name_node = quests[i].node->find(u"QuestInfo/name");
    auto name = static_cast<wz::Property<std::u16string> *>(name_node)->get();
    auto lvl_node = quests[i].node->find(u"Check/0/lvmin");
    auto lvl = static_cast<wz::Property<int> *>(lvl_node)->get();
    auto lvl2 = std::to_string(lvl);
    std::u16string lvl3{lvl2.begin(), lvl2.end()};
    name = u"(Lv." + lvl3 + u") " + name;
    freetype::draw_line(name, pos_rect.x + 12, pos_rect.y + 25 + lh * i);
  }
  freetype::load_aligned(false);
}

bool npc_dlg_ui_system::render() {
  render_backgrnd();
  render_npc();
  render_button();
  render_text();
  render_list();
  return true;
}

SDL_FPoint npc_dlg_ui_system::load_wh() {
  freetype::load_size(12);
  auto h = freetype::load_h(text, 330, 1.1);
  h = h + 150;
  return {529, static_cast<float>(std::max((int)h, 200))};
}

void npc_dlg_ui_system::open() {
  auto wh = load_wh();
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.begin(), event);
}

void npc_dlg_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void npc_dlg_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

void npc_dlg_ui_system::event_button_close() { close(); }

void npc_dlg_ui_system::event_button_ok() { close(); }

void npc_dlg_ui_system::event_button_prev() {
  selected = u"";
  index--;
  cb();
}

void npc_dlg_ui_system::event_button_next() {
  selected = u"";
  index++;
  cb();
}

bool npc_dlg_ui_system::event_button(SDL_Event *event) {
  auto [w, h] = load_wh();
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{10, h - 25, 85, 18},     //
      SDL_FRect{w - 56, h - 25, 46, 18}, //
  };
  std::vector<std::function<void()>> func = {
      event_button_close,
      event_button_ok,
  };
  auto w0 = w - 72;
  if (index < max_index) {
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
    w0 -= 52;
    func.push_back(event_button_next);
  }
  if (index > 0) {
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
    func.push_back(event_button_prev);
  }

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      func[i]();
      audio_game_instance::load_audio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }
  return false;
}

bool npc_dlg_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool npc_dlg_ui_system::event(SDL_Event *event) {
  bool r = false;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        time = UINT64_MAX;
        event_button(event);
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    break;
  }
  default: {
    break;
  }
  }

  return r;
}