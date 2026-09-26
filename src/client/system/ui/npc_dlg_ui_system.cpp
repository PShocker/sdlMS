#include "npc_dlg_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "notice_ui_system.h"
#include "src/client/game/game_npc.h"
#include "src/client/game/game_quest.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/npc_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/quest_alarm_ui_system.h"
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
  if (type == npc_dlg_enum::quest_avaliable && index == max_index) {
    buttons_node.push_back(wz_resource::ui->find(u"UIWindow.img/Quest/BtOK"));
    buttons_rect.push_back(SDL_FRect{w - 128, h - 25, 57, 17});

    buttons_node.push_back(wz_resource::ui->find(u"UIWindow.img/Quest/BtNo"));
    buttons_rect.push_back(SDL_FRect{w - 66, h - 25, 57, 17});
  } else {
    buttons_node.push_back(
        wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/BtOK"));
    buttons_rect.push_back(SDL_FRect{w - 56, h - 25, 46, 18});
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
  case npc_dlg_enum::quest_avaliable:
  case npc_dlg_enum::quest_complete:
  case npc_dlg_enum::quest_yes:
  case npc_dlg_enum::quest_no:
  case npc_dlg_enum::quest_stop_item:
  case npc_dlg_enum::quest_stop_lost:
  case npc_dlg_enum::quest_stop_mob:
  case npc_dlg_enum::quest_stop_npc:
  case npc_dlg_enum::talk: {
    freetype::load_size(12);
    freetype::load_aligned(true);
    freetype::load_color(0, 0, 0, 255);
    freetype::draw_rstr(str, pos.x + 165, pos.y + 30, 330, 1.3, std::nullopt);
    break;
  }
  case npc_dlg_enum::select: {
    freetype::load_size(12);
    freetype::load_aligned(true);
    freetype::load_color(0, 0, 0, 255);
    freetype::draw_rstr(str, pos.x + 165, pos.y + 30, 330, 1.3, std::nullopt);
    break;
  }
  default: {
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
      wz_resource::ui->find(u"UtilDlgEx.img/UtilDlgEx/list3"));

  auto avaliable_quest = npc_game_instance::load_avaliable_quest(npc_id);
  auto progress_quest = npc_game_instance::load_progress_quest(npc_id);
  auto progress_complete_quest =
      npc_game_instance::load_progress_complete_quest(npc_id);

  std::ranges::sort(progress_quest);
  std::ranges::sort(progress_complete_quest);

  std::vector<std::u16string> result;
  std::ranges::set_difference(progress_quest, progress_complete_quest,
                              std::back_inserter(result));
  progress_quest = result;

  const auto &mouse_pos = window::mouse_pos;
  selected = u"";
  freetype::load_size(12);
  freetype::load_aligned(true);
  freetype::load_color(128, 0, 128, 255);
  auto lh = freetype::load_lh();

  const auto render_q = [&](SDL_Texture *t, std::vector<std::u16string> &q,
                            float &y, std::u16string type) {
    if (q.empty()) {
      return;
    }
    SDL_FRect pos_rect{
        static_cast<float>((int)pos.x + 165),
        (int)pos.y + y,
        static_cast<float>(t->w),
        static_cast<float>(t->h),
    };
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
    y += 20;
    for (int i = 0; i < q.size(); i++) {
      auto quest_node = quest_game_instance::load_quest_node(q[i]);
      auto name_node = quest_node->find(u"QuestInfo/name");
      auto name = static_cast<wz::Property<std::u16string> *>(name_node)->get();
      auto lv_node = quest_node->find(u"Check/0/lvmin");
      auto lv = 0;
      if (lv_node) {
        lv = static_cast<wz::Property<int> *>(lv_node)->get();
      }
      auto lv2 = std::to_string(lv);
      std::u16string lv3{lv2.begin(), lv2.end()};
      name = u"(Lv." + lv3 + u") " + name + u" (" + type + u")";
      auto text_x = pos_rect.x + 12;
      auto text_y = pos.y + y;
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
        selected = q[i];
      }
      y += lh;
    }
    y += 12;
  };
  auto y = freetype::load_h(text, 330, 1.3) + 40;
  render_q(t2, progress_complete_quest, y, u"Complete");
  render_q(t1, avaliable_quest, y, u"Avaliable");
  render_q(t0, progress_quest, y, u"Progress");
}

void npc_dlg_ui_system::render_obtain() {
  if (type != npc_dlg_enum::quest_complete) {
    return;
  }
  if (index != max_index) {
    return;
  }
  static auto t =
      wz_resource::load_texture(wz_resource::ui->find(u"QuestIcon.img/4/0"));
  auto y = freetype::load_h(text, 330, 1.3) + 40;
  SDL_FRect pos_rect{
      static_cast<float>((int)pos.x + 165),
      (int)pos.y + y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  auto act_exp = quest_game_instance::load_quest_act_exp(quest_id);
  if (act_exp != 0) {
    static auto t2 =
        wz_resource::load_texture(wz_resource::ui->find(u"QuestIcon.img/8/0"));
    pos_rect.y += 20;
    pos_rect.w = t2->w;
    pos_rect.h = t2->h;
    SDL_RenderTexture(window::renderer, t2, nullptr, &pos_rect);
    freetype::load_size(12);
    freetype::load_aligned(true);
    freetype::load_color(0, 0, 0, 255);
    auto tmp = std::to_string(act_exp);
    freetype::draw_line({tmp.begin(), tmp.end()}, pos_rect.x + 22, pos_rect.y);
  }
  auto act_meso = quest_game_instance::load_quest_act_meso(quest_id);
  if (act_meso != 0) {
    pos_rect.y += 22;
    static auto t3 =
        wz_resource::load_texture(wz_resource::ui->find(u"QuestIcon.img/7/0"));
    pos_rect.y += 3;
    pos_rect.w = t3->w;
    pos_rect.h = t3->h;
    SDL_RenderTexture(window::renderer, t3, nullptr, &pos_rect);
    freetype::load_size(12);
    freetype::load_aligned(true);
    freetype::load_color(0, 0, 0, 255);
    auto tmp = std::to_string(act_meso);
    freetype::draw_line({tmp.begin(), tmp.end()}, pos_rect.x + 22, pos_rect.y);
  }
  auto act_item = quest_game_instance::load_quest_act_item(quest_id);
  if (!act_item.empty()) {
    pos_rect.y += 26;
    for (auto [k, v] : act_item) {
      if (v <= 0) {
        continue;
      }
      SDL_Texture *t;
      std::u16string name;
      if (item_game_instance::check_item(k)) {
        auto info = item_game_instance::load_item_info(k, 0);
        t = wz_resource::load_texture(info->get_child(u"icon"));
        name = item_game_instance::load_item_text(k, u"name");
      } else {
        auto info = equip_game_instance::load_equip_info(k);
        t = wz_resource::load_texture(info->get_child(u"icon"));
        name = equip_game_instance::load_equip_name(k);
      }
      pos_rect.w = t->w;
      pos_rect.h = t->h;
      SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
      freetype::load_size(12);
      freetype::load_aligned(true);
      freetype::load_color(0, 0, 0, 255);
      auto tmp = std::to_string(v);
      name = name + u" x" + std::u16string{tmp.begin(), tmp.end()};
      freetype::draw_line(name, pos_rect.x + t->w + 4, pos_rect.y + t->h - 18);
      pos_rect.y += t->h + 4;
    }
  }
}

bool npc_dlg_ui_system::render() {
  render_backgrnd();
  render_npc();
  render_button();
  render_text();
  render_list();
  render_obtain();
  return true;
}

SDL_FPoint npc_dlg_ui_system::load_wh() {
  freetype::load_size(12);
  auto h = freetype::load_h(text, 330, 1.3);
  h = h + 140;
  if (type == npc_dlg_enum::quest && index == 0) {
    auto avaliable_quest = npc_game_instance::load_avaliable_quest(npc_id);
    auto progress_quest = npc_game_instance::load_progress_quest(npc_id);
    auto progress_complete =
        npc_game_instance::load_progress_complete_quest(npc_id);
    freetype::load_size(12);
    auto lh = freetype::load_lh();
    for (const auto &v : {avaliable_quest, progress_quest, progress_complete}) {
      if (!v.empty()) {
        h += v.size() * lh;
        h += 18;
      }
    }
  } else if (type == npc_dlg_enum::quest_complete && index == max_index) {
    auto exp = quest_game_instance::load_quest_act_exp(quest_id);
    if (exp != 0) {
      h += 22;
    }
    auto item = quest_game_instance::load_quest_act_item(quest_id);
    for (auto [k, v] : item) {
      if (v > 0) {
        h += 32;
      }
    }
    auto meso = quest_game_instance::load_quest_act_meso(quest_id);
    if (meso != 0) {
      h += 22;
    }
  }
  h = std::max((int)h, 190);

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

void npc_dlg_ui_system::event_button_ok() {
  if (type == npc_dlg_enum::quest_complete && index == max_index) {
    auto back_meso = package_game_instance::meso;
    auto back_data = package_game_instance::data;
    auto act_item = quest_game_instance::load_quest_act_item(quest_id);
    for (auto [k, v] : act_item) {
      if (v > 0) {
        std::polymorphic<game_item> item;
        if (item_game_instance::check_item(k)) {
          item = item_game_instance::load_item(k, v);
        } else {
          item = equip_game_instance::load_item(k);
        }
        if (!package_game_instance::add_new_item(item)) {
          notice_ui_system::open_no_space(item->type);
          package_game_instance::meso = back_meso;
          package_game_instance::data = back_data;
          close();
          return;
        }
      } else {
        auto item = package_game_instance::load_item(k);
        item_game_instance::dec_item_num(*item, -v);
      }
    }
    auto act_meso = quest_game_instance::load_quest_act_meso(quest_id);
    package_game_instance::add_meso(act_meso);
    auto act_exp = quest_game_instance::load_quest_act_exp(quest_id);
    character_stat_game_instance::add_exp(act_exp);
    quest_game_instance::complete_quest(quest_id);
    quest_alarm_ui_system::complete_quest(quest_id);
    auto quest_node = quest_game_instance::load_quest_node(quest_id);
    if (auto n = quest_node->find(u"Say/1/yes/0"); n != nullptr) {
      text = text_game_instance::load_rstr(n);
      index = 0;
      max_index = n->children_count() - 1;
      return;
    }
  }
  close();
}

void npc_dlg_ui_system::event_button_prev() {
  selected = u"";
  --index;
  if (cb) {
    cb();
    return;
  }
  if (!script_id.empty()) {
    script::fns().at(script_id)(nullptr);
    return;
  }
  if (quest_id.empty()) {
    return;
  }
  // 根据 type 决定后缀，统一拼接路径
  std::u16string suffix;
  switch (type) {
  case npc_dlg_enum::quest:
    suffix = u"";
    break;
  case npc_dlg_enum::quest_stop_item:
    suffix = u"/item";
    break;
  case npc_dlg_enum::quest_stop_lost:
    suffix = u"/lost";
    break;
  case npc_dlg_enum::quest_stop_npc:
    suffix = u"/npc";
    break;
  default:
    suffix = u"";
  }
  auto node = quest_game_instance::load_quest_node(quest_id);
  if (node) {
    node = node->find(u"Say/" + quest_index + suffix);
  }
  if (node != nullptr) {
    text =
        text_game_instance::load_rstr(node->get_child(std::to_string(index)));
  }
  time = window::dt_now;
}

void npc_dlg_ui_system::event_button_next() {
  selected = u"";
  ++index;
  if (cb) {
    cb();
    return;
  }
  if (!script_id.empty()) {
    script::fns().at(script_id)(nullptr);
    return;
  }
  if (quest_id.empty()) {
    return;
  }
  // 根据 type 决定后缀，统一拼接路径
  std::u16string suffix;
  switch (type) {
  case npc_dlg_enum::quest_complete:
  case npc_dlg_enum::quest:
    suffix = u"";
    break;
  case npc_dlg_enum::quest_stop_item:
    suffix = u"/item";
    break;
  case npc_dlg_enum::quest_stop_lost:
    suffix = u"/lost";
    break;
  case npc_dlg_enum::quest_stop_npc:
    suffix = u"/npc";
    break;
  default:
    suffix = u"";
  }
  auto node = quest_game_instance::load_quest_node(quest_id);
  if (node) {
    node = node->find(u"Say/" + quest_index + suffix);
  }
  if (node != nullptr) {
    text =
        text_game_instance::load_rstr(node->get_child(std::to_string(index)));
  }
  time = window::dt_now;
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
  script_id = u"";
  quest_id = selected;

  auto progress = quest_game_instance::load_quest_progress(selected);
  auto tmp = std::to_string(progress);
  quest_index = {tmp.begin(), tmp.end()};
  auto progress_complete =
      npc_game_instance::load_progress_complete_quest(npc_id);

  wz::Node *node = nullptr;
  if (std::ranges::contains(progress_complete, quest_id)) {
    // 待完成
    node = quest_game_instance::load_quest_node(selected);
    if (auto n = node->find(u"Check/0/endscript"); n != nullptr) {
      auto spt = static_cast<wz::Property<std::u16string> *>(n)->get();
      script::fns().at(spt)(nullptr);
      script_id = spt;
      return;
    }
    if (node->find(u"Say/" + quest_index + u"/ask")) {
      text = text_game_instance::load_rstr(
          node->find(u"Say/" + quest_index + u"/0"));
      type = npc_dlg_enum::select;
      index = 0;
    }
    if (auto n = node->find(u"Act/" + quest_index + u"/nextQuest");
        n != nullptr) {
      quest_game_instance::complete_quest(selected);
      quest_alarm_ui_system::complete_quest(selected);
      auto nq = static_cast<wz::Property<int> *>(n)->get();
      tmp = std::to_string(nq);
      selected = std::u16string{tmp.begin(), tmp.end()} + u".img";
      event_quest_list();
      return;
    }
    type = npc_dlg_enum::quest_complete;
    node = node->find(u"Say/" + quest_index);
    auto child = node->children;
    child.erase(u"yes");
    child.erase(u"no");
    child.erase(u"stop");
    child.erase(u"lost");
    max_index = child.size() - 1;
  } else if (quest_game_instance::progress_quests.contains(quest_id)) {
    // 判断是否是进行中的任务
    auto &q = quest_game_instance::progress_quests.at(quest_id);
    if (!q.item_bool) {
      type = npc_dlg_enum::quest_stop_item;
      node = quest_game_instance::load_quest_node(selected);
      node = node->find(u"Say/" + quest_index + u"/stop/npc");
    } else if (!q.mob_bool) {
      type = npc_dlg_enum::quest_stop_item;
      node = quest_game_instance::load_quest_node(selected);
      node = node->find(u"Say/" + quest_index + u"/stop/mob");
    } else {
      type = npc_dlg_enum::quest_stop_npc;
      node = quest_game_instance::load_quest_node(selected);
      node = node->find(u"Say/" + quest_index + u"/stop/item");
    }
    if (node) {
      max_index = node->children_count();
    }
  } else {
    type = npc_dlg_enum::quest_avaliable;
    node = quest_game_instance::load_quest_node(selected);
    if (auto n = node->find(u"Check/0/startscript"); n != nullptr) {
      auto spt = static_cast<wz::Property<std::u16string> *>(n)->get();
      script::fns().at(spt)(nullptr);
      script_id = spt;
      return;
    }
    node = node->find(u"Say/" + quest_index);
    auto child = node->children;
    child.erase(u"yes");
    child.erase(u"no");
    child.erase(u"stop");
    child.erase(u"lost");
    max_index = child.size() - 1;
  }
  if (node != nullptr && node->get_child(u"0")) {
    index = 0;
    text = text_game_instance::load_rstr(node->get_child(u"0"));
    time = window::dt_now;
  }
}

void npc_dlg_ui_system::event_button_quest_yes() {
  if (!script_id.empty()) {
    index++;
    script::fns().at(script_id)(nullptr);
    return;
  }

  auto act_item = quest_game_instance::load_quest_act_item(quest_id);
  auto back_data = package_game_instance::data;
  for (auto [k, v] : act_item) {
    if (v > 0) {
      std::polymorphic<game_item> item;
      if (item_game_instance::check_item(k)) {
        item = item_game_instance::load_item(k, v);
      } else {
        item = equip_game_instance::load_item(k);
      }
      if (!package_game_instance::add_new_item(item)) {
        notice_ui_system::open_no_space(item->type);
        package_game_instance::data = back_data;
        close();
        return;
      }
    } else {
      auto item = package_game_instance::load_item(k);
      item_game_instance::dec_item_num(*item, -v);
    }
  }
  // accept quest
  quest_game_instance::accept_quest(quest_id);
  quest_alarm_ui_system::accept_quest(quest_id);
  static wz::Node *yes_node;
  yes_node = quest_game_instance::load_quest_node(quest_id);
  yes_node = yes_node->find(u"Say/" + quest_index + u"/yes");
  if (yes_node == nullptr) {
    close();
    return;
  } else {
    type = npc_dlg_enum::quest_yes;
    index = 0;
    max_index = yes_node->children_count() - 1;
    auto node = yes_node->get_child(u"0");
    text = text_game_instance::load_rstr(node);
    time = window::dt_now;
  }
}

void npc_dlg_ui_system::event_button_quest_no() {
  static wz::Node *no_node;
  no_node = quest_game_instance::load_quest_node(quest_id);
  no_node = no_node->find(u"Say/" + quest_index + u"/no");
  if (no_node == nullptr) {
    close();
    return;
  } else {
    type = npc_dlg_enum::quest_no;
    index = 0;
    max_index = no_node->children_count() - 1;
    auto node = no_node->get_child(u"0");
    text = text_game_instance::load_rstr(node);
    time = window::dt_now;
  }
}

bool npc_dlg_ui_system::event_button(SDL_Event *event) {
  auto [w, h] = load_wh();
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{10, h - 25, 85, 18}, //
  };
  std::vector<std::function<void()>> func = {
      event_button_close,
  };

  auto w0 = w - 72;
  if (index < max_index) {
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
    func.push_back(event_button_next);
    w0 -= 52;
  }
  if (index > 0) {
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
    func.push_back(event_button_prev);
  }
  if (index == max_index && cb == nullptr &&
      type == npc_dlg_enum::quest_avaliable) {
    buttons_rect.push_back(SDL_FRect{w - 128, h - 25, 57, 17});
    func.push_back(event_button_quest_yes);

    buttons_rect.push_back(SDL_FRect{w - 66, h - 25, 57, 17});
    func.push_back(event_button_quest_no);
  } else {
    buttons_rect.push_back(SDL_FRect{w - 56, h - 25, 46, 18});
    func.push_back(event_button_ok);
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
    case SDL_SCANCODE_RETURN: {
      if (index == max_index && cb == nullptr &&
          type == npc_dlg_enum::quest_avaliable) {
        event_button_quest_yes();
        return false;
      }
      if (type == npc_dlg_enum::quest_avaliable && index < max_index) {
        event_button_next();
        return false;
      }
      if (type == npc_dlg_enum::quest_complete) {
        event_button_ok();
        return false;
      }
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