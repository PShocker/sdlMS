#include "npc_dlg_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_npc.h"
#include "src/client/game/game_quest.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/npc_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/script/script.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

static std::u16string selected;

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

  };
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{10, h - 25, 85, 18}, //
  };
  auto i = index;
  auto m = max_index;
  switch (type) {
  case npc_dlg_enum::choose: {
    break;
  }
  case npc_dlg_enum::quest: {
    if (index == max_index && cb == nullptr) {
      buttons_node.push_back(wz_resource::ui->find(u"UIWindow.img/Quest/BtOK"));
      buttons_rect.push_back(SDL_FRect{w - 128, h - 25, 57, 17});

      buttons_node.push_back(wz_resource::ui->find(u"UIWindow.img/Quest/BtNo"));
      buttons_rect.push_back(SDL_FRect{w - 66, h - 25, 57, 17});
      break;
    }
    if (index == 0) {
      break;
    }
    i -= 1;
    m -= 1;
  }
  case npc_dlg_enum::talk: {
    buttons_node.push_back(
        wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/BtOK"));
    buttons_rect.push_back(SDL_FRect{w - 56, h - 25, 46, 18});

    auto w0 = w - 72;
    if (i < m) {
      buttons_node.push_back(
          wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/BtNext"));
      buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
      w0 -= 52;
    }
    if (i > 0) {
      buttons_node.push_back(
          wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/BtPrev"));
      buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
    }

    break;
  }
  case npc_dlg_enum::select: {
    break;
  }
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
  dt = dt / 35;
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
    freetype::draw_rstr(str, pos.x + 165, pos.y + 30, 330, 1.3, std::nullopt);
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

  auto lh = freetype::load_lh();
  static auto t0 = wz_resource::load_texture(
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/list0"));
  static auto t1 = wz_resource::load_texture(
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/list1"));
  static auto t2 = wz_resource::load_texture(
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/list2"));

  auto quests = npc_game_instance::load_avaliable_quest(npc_id);

  SDL_FRect pos_rect{
      pos.x + 165,
      pos.y + h - 100 - lh * quests.size(),
      static_cast<float>(t1->w),
      static_cast<float>(t1->h),
  };
  SDL_RenderTexture(window::renderer, t1, nullptr, &pos_rect);

  const auto &mouse_pos = window::mouse_pos;
  selected = u"";
  freetype::load_size(12);
  freetype::load_aligned(true);
  freetype::load_color(128, 0, 128, 255);
  for (int i = 0; i < quests.size(); i++) {
    auto quest_node = quest_game_instance::load_quest_node(quests[i]);
    auto name_node = quest_node->find(u"QuestInfo/name");
    auto name = static_cast<wz::Property<std::u16string> *>(name_node)->get();
    auto lvl_node = quest_node->find(u"Check/0/lvmin");
    auto lvl = 0;
    if (lvl_node) {
      lvl = static_cast<wz::Property<int> *>(lvl_node)->get();
    }
    auto lvl2 = std::to_string(lvl);
    std::u16string lvl3{lvl2.begin(), lvl2.end()};
    name = u"(Lv." + lvl3 + u") " + name;
    auto text_x = pos_rect.x + 12;
    auto text_y = pos_rect.y + 25 + lh * i;
    freetype::draw_line(name, text_x, text_y);
    auto text_w = freetype::load_w(name);
    SDL_FRect r{
        text_x,
        text_y,
        text_w,
        lh,
    };
    if (SDL_PointInRectFloat(&mouse_pos, &r)) {
      SDL_SetRenderDrawColor(window::renderer, 128, 0, 128, 255);
      SDL_RenderLine(window::renderer, text_x, text_y + lh, text_x + text_w,
                     text_y + lh);
      selected = quests[i];
    }
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
  auto h = freetype::load_h(text, 330, 1.3);
  h = h + 140;
  h = std::max((int)h, 190);
  if (type == npc_dlg_enum::quest && index == 0) {
    auto quests = quest_game_instance::load_npc_quest(npc_id);
    freetype::load_size(12);
    auto lh = freetype::load_lh();
    h += quests.size() * lh;
  }
  return {529, h};
}

void npc_dlg_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    keyboard_input_system::reset();

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
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
  if (cb) {
    cb();
  } else if (!script_id.empty()) {
    script::fns().at(script_id)(nullptr);
  } else if (!quest_id.empty()) {
    auto node = quest_game_instance::load_quest_node(quest_id);
    node = node->find(u"Say/" + quest_index);
    text = text_game_instance::load_rstr(
        node->get_child(std::to_string(index - 1)));
    time = window::dt_now;
  }
}

void npc_dlg_ui_system::event_button_next() {
  selected = u"";
  index++;
  if (cb) {
    cb();
  } else if (!script_id.empty()) {
    script::fns().at(script_id)(nullptr);
  } else if (!quest_id.empty()) {
    auto node = quest_game_instance::load_quest_node(quest_id);
    node = node->find(u"Say/" + quest_index);
    text = text_game_instance::load_rstr(
        node->get_child(std::to_string(index - 1)));
    time = window::dt_now;
  }
}

void npc_dlg_ui_system::event_quest_list() {
  if (type != npc_dlg_enum::quest) {
    return;
  }
  if (index != 0) {
    return;
  }
  if (selected.empty()) {
    return;
  }
  quest_id = selected;
  script_id = u"";
  auto node = quest_game_instance::load_quest_node(selected);
  if (auto n = node->find(u"Check/0/startscript"); n != nullptr) {
    auto spt = static_cast<wz::Property<std::u16string> *>(n)->get();
    script::fns().at(spt)(nullptr);
    script_id = spt;
    return;
  }
  index++;

  auto progress = quest_game_instance::load_quest_progress(selected);
  auto tmp = std::to_string(progress);
  quest_index = {tmp.begin(), tmp.end()};

  node = node->find(u"Say/" + quest_index);
  auto child = node->children;
  child.erase(u"yes");
  child.erase(u"no");
  child.erase(u"stop");
  max_index = child.size();

  text = text_game_instance::load_rstr(node->get_child(u"0"));
  time = window::dt_now;
}

void npc_dlg_ui_system::event_button_quest_yes() {
  if (!script_id.empty()) {
    index = -1;
    script::fns().at(script_id)(nullptr);
    return;
  }
  // accept quest
  quest_game_instance::accept_quest(quest_id);

  static wz::Node *yes_node;
  yes_node = quest_game_instance::load_quest_node(quest_id);
  yes_node = yes_node->find(u"Say/" + quest_index + u"/yes");
  if (yes_node == nullptr) {
    close();
    return;
  }
  index = 1;

  cb = []() {
    auto node = yes_node->get_child(std::to_string(index - 1));
    text = text_game_instance::load_rstr(node);
    time = window::dt_now;
  };
  max_index = yes_node->children_count();
  cb();
}

void npc_dlg_ui_system::event_button_quest_no() {
  static wz::Node *no_node;
  no_node = quest_game_instance::load_quest_node(quest_id);
  no_node = no_node->find(u"Say/" + quest_index + u"/no");
  if (no_node == nullptr) {
    close();
    return;
  }
  index = 1;
  cb = []() {
    auto node = no_node->get_child(std::to_string(index - 1));
    text = text_game_instance::load_rstr(node);
    time = window::dt_now;
  };
  max_index = no_node->children_count();
  cb();
}

bool npc_dlg_ui_system::event_button(SDL_Event *event) {
  auto [w, h] = load_wh();
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{10, h - 25, 85, 18}, //
  };
  std::vector<std::function<void()>> func = {
      event_button_close,
  };

  auto i = index;
  auto m = max_index;
  switch (type) {
  case npc_dlg_enum::choose: {
    break;
  }
  case npc_dlg_enum::quest: {
    if (index == max_index && cb == nullptr) {
      buttons_rect.push_back(SDL_FRect{w - 128, h - 25, 57, 17});
      func.push_back(event_button_quest_yes);

      buttons_rect.push_back(SDL_FRect{w - 66, h - 25, 57, 17});
      func.push_back(event_button_quest_no);
      break;
    }
    if (index == 0) {
      break;
    }
    i -= 1;
    m -= 1;
  }
  case npc_dlg_enum::talk: {
    buttons_rect.push_back(SDL_FRect{w - 56, h - 25, 46, 18});
    func.push_back(event_button_ok);

    auto w0 = w - 72;
    if (i < m) {
      buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
      func.push_back(event_button_next);
      w0 -= 52;
    }
    if (i > 0) {
      buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
      func.push_back(event_button_prev);
    }

    break;
  }
  case npc_dlg_enum::select: {
    break;
  }
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
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      event_button_close();
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
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        time = UINT64_MAX;
        event_quest_list();
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