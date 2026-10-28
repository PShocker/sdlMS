#include "npc_dlg_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "notice_ui_system.h"
#include "quest_ui_system.h"
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
#include "src/client/game_instance/random_game_instance.h"
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
#include <optional>
#include <string>
#include <vector>

static std::u16string selected;

void NpcDlgUiSystem::reset_pos() {
  auto wh = load_wh();
  auto &camera = CameraGameInstance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;
}

void NpcDlgUiSystem::render_backgrnd() {
  auto [w, h] = load_wh();
  static auto t = WzResource::load_texture(
      WzResource::ui->find(u"UtilDlgEx.img/UtilDlgEx/t"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
  static auto c = WzResource::load_texture(
      WzResource::ui->find(u"UtilDlgEx.img/UtilDlgEx/c"));
  static auto s = WzResource::load_texture(
      WzResource::ui->find(u"UtilDlgEx.img/UtilDlgEx/s"));
  pos_rect = {
      pos.x,
      pos.y + t->h,
      static_cast<float>(c->w),
      static_cast<float>(h - t->h - s->h),
  };
  SDL_RenderTextureTiled(Window::renderer, c, nullptr, 1, &pos_rect);

  pos_rect = {
      pos.x,
      pos.y + h - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_RenderTexture(Window::renderer, s, nullptr, &pos_rect);
}

void NpcDlgUiSystem::render_npc() {
  auto [w, h] = load_wh();
  GameNpc npc;
  npc.action = u"stand";
  const auto &camera = CameraGameInstance::camera;
  npc.pos.x = camera.x + pos.x + 80;
  npc.pos.y = camera.y + pos.y + 15 + h / 2;
  npc.ani_index = 0;
  npc.id = npc_id;
  NpcRenderSystem::render_npc(npc);

  //   render bar
  static auto t = WzResource::load_texture(
      WzResource::ui->find(u"UtilDlgEx.img/UtilDlgEx/bar"));
  SDL_FRect pos_rect{
      pos.x + 80 - t->w / 2,
      pos.y + 15 + h / 2,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);

  Freetype::load_size(12);
  Freetype::load_aligned(true);
  Freetype::load_color(255, 255, 255, 255);
  auto npc_name = NpcGameInstance::load_npc_text(npc.id, u"name");
  w = Freetype::load_w(npc_name);
  Freetype::draw_line(npc_name, pos_rect.x + 61 - w / 2, pos_rect.y);
  Freetype::load_aligned(false);
}

void NpcDlgUiSystem::render_button() {
  auto [w, h] = load_wh();
  std::vector<wz::Node *> buttons_node = {
      WzResource::ui->find(u"UtilDlgEx.img/UtilDlgEx/BtClose"),
  };
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{10, h - 25, 85, 18}, //
  };

  auto w0 = w - 72;
  if (index < max_index) {
    buttons_node.push_back(
        WzResource::ui->find(u"UtilDlgEx.img/UtilDlgEx/BtNext"));
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
    w0 -= 52;
  }
  if (index > 0) {
    buttons_node.push_back(
        WzResource::ui->find(u"UtilDlgEx.img/UtilDlgEx/BtPrev"));
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
  }
  if (type == NpcDlgEnum::quest_avaliable && index == max_index) {
    buttons_node.push_back(WzResource::ui->find(u"UIWindow.img/Quest/BtOK"));
    buttons_rect.push_back(SDL_FRect{w - 128, h - 25, 57, 17});

    buttons_node.push_back(WzResource::ui->find(u"UIWindow.img/Quest/BtNo"));
    buttons_rect.push_back(SDL_FRect{w - 66, h - 25, 57, 17});
  } else {
    buttons_node.push_back(
        WzResource::ui->find(u"UtilDlgEx.img/UtilDlgEx/BtOK"));
    buttons_rect.push_back(SDL_FRect{w - 56, h - 25, 46, 18});
  }

  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    auto &mouse_pos = Window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render) {
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

void NpcDlgUiSystem::render_text() {
  auto dt = Window::dt_now - time;
  int visibleCount = int(dt * 0.05);
  switch (type) {
  case NpcDlgEnum::quest:
  case NpcDlgEnum::quest_avaliable:
  case NpcDlgEnum::quest_complete:
  case NpcDlgEnum::quest_yes:
  case NpcDlgEnum::quest_no:
  case NpcDlgEnum::quest_stop_item:
  case NpcDlgEnum::quest_stop_lost:
  case NpcDlgEnum::quest_stop_mob:
  case NpcDlgEnum::quest_stop_npc:
  case NpcDlgEnum::talk:
  case NpcDlgEnum::select: {
    Freetype::load_size(12);
    Freetype::load_aligned(true);
    Freetype::load_color(0, 0, 0, 255);
    select_mouse =
        Freetype::draw_rstr(text, pos.x + 165, pos.y + 30, 330, 1.3,
                            std::nullopt, select, false, visibleCount)
            .select;
    break;
  }
  default: {
    break;
  }
  }
}

void NpcDlgUiSystem::render_list() {
  if (type != NpcDlgEnum::quest) {
    return;
  }
  if (index != 0) {
    return;
  }
  auto [w, h] = load_wh();

  static auto t0 = WzResource::load_texture(
      WzResource::ui->find(u"UtilDlgEx.img/UtilDlgEx/list0"));
  static auto t1 = WzResource::load_texture(
      WzResource::ui->find(u"UtilDlgEx.img/UtilDlgEx/list1"));
  static auto t2 = WzResource::load_texture(
      WzResource::ui->find(u"UtilDlgEx.img/UtilDlgEx/list3"));

  auto avaliable_quest = NpcGameInstance::load_avaliable_quest(npc_id);
  auto progress_quest = NpcGameInstance::load_progress_quest(npc_id);
  auto progress_complete_quest =
      NpcGameInstance::load_progress_complete_quest(npc_id);

  std::ranges::sort(progress_quest);
  std::ranges::sort(progress_complete_quest);

  std::vector<std::u16string> result;
  std::ranges::set_difference(progress_quest, progress_complete_quest,
                              std::back_inserter(result));
  progress_quest = result;

  const auto &mouse_pos = Window::mouse_pos;
  selected = u"";
  Freetype::load_size(12);
  Freetype::load_aligned(true);
  Freetype::load_color(128, 0, 128, 255);
  auto lh = Freetype::load_lh();

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
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    y += 20;
    for (int i = 0; i < q.size(); i++) {
      auto quest_node = QuestGameInstance::load_quest_node(q[i]);
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
      Freetype::draw_line(name, text_x, text_y);
      auto text_w = Freetype::load_w(name);
      SDL_FRect r{
          text_x,
          text_y,
          text_w,
          lh,
      };
      if (SDL_PointInRectFloat(&mouse_pos, &r)) {
        SDL_SetRenderDrawColor(Window::renderer, 128, 0, 128, 255);
        SDL_RenderLine(Window::renderer, text_x, text_y + lh, text_x + text_w,
                       text_y + lh);
        selected = q[i];
      }
      y += lh;
    }
    y += 12;
  };
  auto y = Freetype::load_ch(text, 330, 1.3) + 40;
  render_q(t2, progress_complete_quest, y, u"Complete");
  render_q(t1, avaliable_quest, y, u"Avaliable");
  render_q(t0, progress_quest, y, u"Progress");

  // script
  y += 10;
  script_id = u"";
  static auto t_etc = WzResource::load_texture(
      WzResource::ui->find(u"UtilDlgEx.img/UtilDlgEx/list2"));
  auto scp_id = NpcGameInstance::load_npc_script(npc_id);
  if (!scp_id.empty()) {
    SDL_FRect pos_rect{
        static_cast<float>((int)pos.x + 165),
        (int)pos.y + y,
        static_cast<float>(t_etc->w),
        static_cast<float>(t_etc->h),
    };
    SDL_RenderTexture(Window::renderer, t_etc, nullptr, &pos_rect);
    auto text_x = pos_rect.x + 12;
    auto text_y = pos_rect.y + 16;
    std::u16string str;
    str = u"(" + scp_id + u")" + u" Talk to " +
          NpcGameInstance::load_npc_text(npc_id, u"name");
    Freetype::draw_line(str, text_x, text_y);
    auto text_w = Freetype::load_w(str);
    SDL_FRect r{
        text_x,
        text_y,
        text_w,
        lh,
    };
    if (SDL_PointInRectFloat(&mouse_pos, &r)) {
      SDL_SetRenderDrawColor(Window::renderer, 128, 0, 128, 255);
      SDL_RenderLine(Window::renderer, text_x, text_y + lh, text_x + text_w,
                     text_y + lh);
      script_id = scp_id;
    }
  }
}

void NpcDlgUiSystem::render_obtain() {
  if (type != NpcDlgEnum::quest_complete) {
    return;
  }
  if (index != max_index) {
    return;
  }
  static auto t =
      WzResource::load_texture(WzResource::ui->find(u"QuestIcon.img/4/0"));
  auto y = Freetype::load_rh(text, 330, 1.3, std::nullopt) + 40;
  SDL_FRect pos_rect{
      static_cast<float>((int)pos.x + 165),
      (int)pos.y + y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
  pos_rect.y += 20;
  if (act_exp != 0) {
    static auto t2 =
        WzResource::load_texture(WzResource::ui->find(u"QuestIcon.img/8/0"));
    pos_rect.w = t2->w;
    pos_rect.h = t2->h;
    SDL_RenderTexture(Window::renderer, t2, nullptr, &pos_rect);
    Freetype::load_size(12);
    Freetype::load_aligned(true);
    Freetype::load_color(0, 0, 0, 255);
    auto tmp = std::to_string(act_exp);
    Freetype::draw_line({tmp.begin(), tmp.end()}, pos_rect.x + 22, pos_rect.y);
    pos_rect.y += t2->h + 8;
  }
  if (act_meso != 0) {
    static auto t3 =
        WzResource::load_texture(WzResource::ui->find(u"QuestIcon.img/7/0"));
    pos_rect.y += 3;
    pos_rect.w = t3->w;
    pos_rect.h = t3->h;
    SDL_RenderTexture(Window::renderer, t3, nullptr, &pos_rect);
    Freetype::load_size(12);
    Freetype::load_aligned(true);
    Freetype::load_color(0, 0, 0, 255);
    auto tmp = std::to_string(act_meso);
    Freetype::draw_line({tmp.begin(), tmp.end()}, pos_rect.x + 22, pos_rect.y);
    pos_rect.y += t3->h + 8;
  }
  if (!act_random_item.empty()) {
    static auto t =
        WzResource::load_texture(WzResource::ui->find(u"QuestIcon.img/5/0"));
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    std::u16string name = u"Random Item";
    Freetype::draw_line(name, pos_rect.x + t->w + 4, pos_rect.y + t->h - 18);
    pos_rect.y += t->h + 8;
  }
  if (!act_item.empty()) {
    for (auto [k, v] : act_item) {
      if (v <= 0) {
        continue;
      }
      SDL_Texture *t;
      std::u16string name;
      if (ItemGameInstance::check_item(k)) {
        auto info = ItemGameInstance::load_item_info(k, 0);
        t = WzResource::load_texture(info->get_child(u"icon"));
        name = ItemGameInstance::load_item_text(k, u"name");
      } else {
        auto info = EquipGameInstance::load_equip_info(k);
        t = WzResource::load_texture(info->get_child(u"icon"));
        name = EquipGameInstance::load_equip_name(k);
      }
      pos_rect.w = t->w;
      pos_rect.h = t->h;
      SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
      Freetype::load_size(12);
      Freetype::load_aligned(true);
      Freetype::load_color(0, 0, 0, 255);
      auto tmp = std::to_string(v);
      name = name + u" x" + std::u16string{tmp.begin(), tmp.end()};
      Freetype::draw_line(name, pos_rect.x + t->w + 4, pos_rect.y + t->h - 18);
      pos_rect.y += t->h + 8;
    }
  }
}

bool NpcDlgUiSystem::render() {
  render_backgrnd();
  render_npc();
  render_button();
  render_text();
  render_list();
  render_obtain();
  return true;
}

SDL_FPoint NpcDlgUiSystem::load_wh() {
  Freetype::load_size(12);
  auto h = Freetype::load_rh(text, 330, 1.3, std::nullopt);
  h = h + 130;
  if (type == NpcDlgEnum::quest && index == 0) {
    auto avaliable_quest = NpcGameInstance::load_avaliable_quest(npc_id);
    auto progress_quest = NpcGameInstance::load_progress_quest(npc_id);
    auto progress_complete_quest =
        NpcGameInstance::load_progress_complete_quest(npc_id);

    std::ranges::sort(progress_quest);
    std::ranges::sort(progress_complete_quest);

    std::vector<std::u16string> result;
    std::ranges::set_difference(progress_quest, progress_complete_quest,
                                std::back_inserter(result));
    progress_quest = result;
    Freetype::load_size(12);
    auto lh = Freetype::load_lh();
    for (const auto &v :
         {avaliable_quest, progress_quest, progress_complete_quest}) {
      if (!v.empty()) {
        h += v.size() * lh;
        h += 18;
      }
    }
    auto scp_id = NpcGameInstance::load_npc_script(npc_id);
    if (!scp_id.empty()) {
      h += 36;
    }
  } else if (type == NpcDlgEnum::quest_complete && index == max_index) {
    if (act_exp != 0) {
      h += 22;
    }
    if (!act_random_item.empty()) {
      h += 27;
    }
    for (auto [k, v] : act_item) {
      if (v > 0) {
        h += 32;
      }
    }
    if (act_meso != 0) {
      h += 22;
    }
  }
  h = std::max((int)h, 190);

  return {529, h};
}

void NpcDlgUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    reset_pos();

    KeyboardInputSystem::reset();

    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

void NpcDlgUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
}

void NpcDlgUiSystem::toggle() {
  auto fn = &render;
  if (std::ranges::contains(System::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

void NpcDlgUiSystem::event_button_close() { close(); }

void NpcDlgUiSystem::event_button_ok() {
  if (type == NpcDlgEnum::quest_complete && index == max_index) {
    auto back_meso = PackageGameInstance::meso;
    auto back_data = PackageGameInstance::data;
    if (!act_random_item.empty()) {
      auto &r = RandomGameInstance::gen;
      std::uniform_int_distribution<int> dist(0, act_random_item.size() - 1);
      auto it = act_random_item.begin();
      std::advance(it, dist(r));
      act_item.insert(*it);
    }
    for (auto [k, v] : act_item) {
      if (v > 0) {
        std::polymorphic<GameItem> item;
        if (ItemGameInstance::check_item(k)) {
          item = ItemGameInstance::load_item(k, v);
        } else {
          item = EquipGameInstance::load_item(k);
        }
        if (!PackageGameInstance::add_new_item(item)) {
          NoticeUiSystem::open_no_space(item->type);
          PackageGameInstance::meso = back_meso;
          PackageGameInstance::data = back_data;
          close();
          return;
        }
      } else {
        auto item = PackageGameInstance::load_item(k);
        ItemGameInstance::dec_item_num(*item, -v);
      }
    }
    PackageGameInstance::add_meso(act_meso);
    CharacterStatGameInstance::add_exp(act_exp);
    if (start_next_quest()) {
      return;
    }
    QuestGameInstance::complete_quest(quest_id);
    QuestAlarmUiSystem::complete_quest(quest_id);
    QuestUiSystem::complete_quest(quest_id);
    auto quest_node = QuestGameInstance::load_quest_node(quest_id);
    if (auto n = quest_node->find(u"Say/1/yes/0"); n != nullptr) {
      time = Window::dt_now;
      text = TextGameInstance::load_rstr(n);
      index = 0;
      max_index = n->children_count() - 1;
      return;
    }
  }
  close();
}

void NpcDlgUiSystem::event_button_update() {
  if (cb) {
    cb();
    reset_pos();
    return;
  }
  if (!script_id.empty()) {
    Script::fns().at(script_id)(nullptr);
    reset_pos();
    return;
  }
  if (quest_id.empty()) {
    return;
  }
  // 根据 type 决定后缀，统一拼接路径
  std::u16string suffix;
  switch (type) {
  case NpcDlgEnum::quest_complete:
  case NpcDlgEnum::quest:
    suffix = u"";
    break;
  case NpcDlgEnum::quest_yes:
    suffix = u"/yes";
    break;
  case NpcDlgEnum::quest_stop_item:
    suffix = u"/item";
    break;
  case NpcDlgEnum::quest_stop_lost:
    suffix = u"/lost";
    break;
  case NpcDlgEnum::quest_stop_npc:
    suffix = u"/npc";
    break;
  default:
    suffix = u"";
  }
  auto node = QuestGameInstance::load_quest_node(quest_id);
  if (node) {
    node = node->find(u"Say/" + quest_index + suffix);
  }
  if (node != nullptr) {
    text =
        TextGameInstance::load_rstr(node->get_child(std::to_string(index)));
  }
  reset_pos();
  time = Window::dt_now;
}

void NpcDlgUiSystem::event_button_prev() {
  selected = u"";
  --index;
  event_button_update();
}

void NpcDlgUiSystem::event_button_next() {
  switch (type) {
  case NpcDlgEnum::select: {
    if (cb) {
      cb();
      return;
    }
    auto node = QuestGameInstance::load_quest_node(quest_id);
    node = node->find(u"Say/" + quest_index + u"/stop/0/answer");
    auto ans = static_cast<wz::Property<int> *>(node)->get() - 1;
    auto st = select;
    if (select_mouse != -1) {
      st = select_mouse;
    }
    if (ans == st) {
      type = NpcDlgEnum::quest_complete;
      node = QuestGameInstance::load_quest_node(quest_id);
      node = node->find(u"Say/" + quest_index);
      node = node->get_child(std::to_string(index + 1));
      time = Window::dt_now;
      text = TextGameInstance::load_rstr(node);
      index = 0;
      max_index = 0;

      act_meso = QuestGameInstance::load_quest_act_meso(quest_id);
      act_item = QuestGameInstance::load_quest_act_item(quest_id);
      act_exp = QuestGameInstance::load_quest_act_exp(quest_id);
    } else {
      type = NpcDlgEnum::talk;
      node = QuestGameInstance::load_quest_node(quest_id);
      node = node->find(u"Say/" + quest_index + u"/stop");
      node = node->get_child(std::to_string(index));
      node = node->get_child(std::to_string(st));
      time = Window::dt_now;
      text = TextGameInstance::load_rstr(node);
      index = 0;
      max_index = 0;
    }
    break;
  }
  default: {
    selected = u"";
    ++index;
    event_button_update();
    break;
  }
  }
}

void NpcDlgUiSystem::event_list() {
  if (type != NpcDlgEnum::quest) {
    return;
  }
  if (index != 0) {
    return;
  }
  if (type == NpcDlgEnum::quest && !script_id.empty()) {
    Script::fns().at(script_id)(nullptr);
    return;
  }
  if (selected.empty()) {
    return;
  }
  script_id = u"";
  quest_id = selected;

  auto progress = QuestGameInstance::load_quest_progress(selected);
  auto tmp = std::to_string(progress);
  quest_index = {tmp.begin(), tmp.end()};
  auto progress_complete =
      NpcGameInstance::load_progress_complete_quest(npc_id);

  wz::Node *node = nullptr;
  if (std::ranges::contains(progress_complete, quest_id)) {
    // 待完成
    node = QuestGameInstance::load_quest_node(quest_id);
    if (auto n = node->find(u"Check/" + quest_index + u"/endscript");
        n != nullptr) {
      auto spt = static_cast<wz::Property<std::u16string> *>(n)->get();
      Script::fns().at(spt)(nullptr);
      script_id = spt;
      return;
    }
    if (node->find(u"Say/" + quest_index + u"/ask")) {
      text = TextGameInstance::load_rstr(
          node->find(u"Say/" + quest_index + u"/0"));
      type = NpcDlgEnum::select;
      select = 0;
      max_select = -1;
      for (size_t i = 0; i + 1 < text.size(); ++i) {
        if (text[i] == u'#' && text[i + 1] == u'L') {
          ++max_select;
          i = i + 3;
        }
      }
      auto n = node->find(u"Say/" + quest_index);
      auto m = n->children;
      m.erase(u"ask");
      m.erase(u"stop");
      index = 0;
      max_index = m.size() - 1;
      time = Window::dt_now;
      select_mouse = -1;
      return;
    }

    type = NpcDlgEnum::quest_complete;
    node = node->find(u"Say/" + quest_index);
    auto child = node->children;
    child.erase(u"yes");
    child.erase(u"no");
    child.erase(u"stop");
    child.erase(u"lost");
    max_index = child.size() - 1;

    act_meso = QuestGameInstance::load_quest_act_meso(quest_id);
    act_item = QuestGameInstance::load_quest_act_item(quest_id);
    act_exp = QuestGameInstance::load_quest_act_exp(quest_id);
    act_random_item = QuestGameInstance::load_quest_act_random_item(quest_id);
    if (act_meso == 0 && act_exp == 0 || max_index == -1) {
      max_index = 0;
      index = max_index;
      event_button_ok();
      return;
    }

  } else if (QuestGameInstance::progress_quests.contains(quest_id)) {
    // 判断是否是进行中的任务
    auto &q = QuestGameInstance::progress_quests.at(quest_id);
    if (!q.item_bool) {
      type = NpcDlgEnum::quest_stop_item;
      node = QuestGameInstance::load_quest_node(selected);
      node = node->find(u"Say/" + quest_index + u"/stop/npc");
    } else if (!q.mob_bool) {
      type = NpcDlgEnum::quest_stop_item;
      node = QuestGameInstance::load_quest_node(selected);
      node = node->find(u"Say/" + quest_index + u"/stop/mob");
    } else {
      type = NpcDlgEnum::quest_stop_npc;
      node = QuestGameInstance::load_quest_node(selected);
      node = node->find(u"Say/" + quest_index + u"/stop/item");
    }
    if (node) {
      max_index = node->children_count();
    }
  } else {
    type = NpcDlgEnum::quest_avaliable;
    node = QuestGameInstance::load_quest_node(selected);
    if (auto n = node->find(u"Check/0/startscript"); n != nullptr) {
      auto spt = static_cast<wz::Property<std::u16string> *>(n)->get();
      Script::fns().at(spt)(nullptr);
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
    text = TextGameInstance::load_rstr(node->get_child(u"0"));
    time = Window::dt_now;
    reset_pos();
  }
}

void NpcDlgUiSystem::event_button_quest_yes() {
  if (!script_id.empty()) {
    index++;
    Script::fns().at(script_id)(nullptr);
    return;
  }
  act_item = QuestGameInstance::load_quest_act_item(quest_id);
  auto back_data = PackageGameInstance::data;
  for (auto [k, v] : act_item) {
    if (v > 0) {
      std::polymorphic<GameItem> item;
      if (ItemGameInstance::check_item(k)) {
        item = ItemGameInstance::load_item(k, v);
      } else {
        item = EquipGameInstance::load_item(k);
      }
      if (!PackageGameInstance::add_new_item(item)) {
        NoticeUiSystem::open_no_space(item->type);
        PackageGameInstance::data = back_data;
        close();
        return;
      }
    } else {
      auto item = PackageGameInstance::load_item(k);
      ItemGameInstance::dec_item_num(*item, -v);
    }
  }
  // accept quest
  QuestGameInstance::accept_quest(quest_id);
  QuestAlarmUiSystem::accept_quest(quest_id);
  static wz::Node *yes_node;
  yes_node = QuestGameInstance::load_quest_node(quest_id);
  yes_node = yes_node->find(u"Say/" + quest_index + u"/yes");
  if (yes_node == nullptr) {
    close();
    return;
  } else {
    type = NpcDlgEnum::quest_yes;
    index = 0;
    max_index = yes_node->children_count() - 1;
    auto node = yes_node->get_child(u"0");
    text = TextGameInstance::load_rstr(node);
    time = Window::dt_now;
  }
}

void NpcDlgUiSystem::event_button_quest_no() {
  static wz::Node *no_node;
  no_node = QuestGameInstance::load_quest_node(quest_id);
  no_node = no_node->find(u"Say/" + quest_index + u"/no");
  if (no_node == nullptr) {
    close();
    return;
  } else {
    type = NpcDlgEnum::quest_no;
    index = 0;
    max_index = no_node->children_count() - 1;
    auto node = no_node->get_child(u"0");
    text = TextGameInstance::load_rstr(node);
    time = Window::dt_now;
  }
}

bool NpcDlgUiSystem::event_button(SDL_Event *event) {
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
      type == NpcDlgEnum::quest_avaliable) {
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
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      func[i]();
      AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }
  return false;
}

bool NpcDlgUiSystem::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void NpcDlgUiSystem::event_motion_bt_audio(SDL_Event *event) {
  if (CursorGameInstance::cursor_ui != render) {
    return;
  }
  auto [w, h] = load_wh();
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{10, h - 25, 85, 18}, //
  };
  auto w0 = w - 72;
  if (index < max_index) {
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
    w0 -= 52;
  }
  if (index > 0) {
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
  }
  if (index == max_index && cb == nullptr &&
      type == NpcDlgEnum::quest_avaliable) {
    buttons_rect.push_back(SDL_FRect{w - 128, h - 25, 57, 17});
    buttons_rect.push_back(SDL_FRect{w - 66, h - 25, 57, 17});
  } else {
    buttons_rect.push_back(SDL_FRect{w - 56, h - 25, 46, 18});
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

bool NpcDlgUiSystem::event(SDL_Event *event) {
  bool r = false;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_UP: {
      if (type == NpcDlgEnum::select) {
        select--;
        select = std::max((int8_t)0, select);
      }
      break;
    }
    case SDL_SCANCODE_DOWN: {
      if (type == NpcDlgEnum::select) {
        select++;
        select = std::min(select, max_select);
      }
      break;
    }
    case SDL_SCANCODE_ESCAPE: {
      event_button_close();
      return false;
      break;
    }
    case SDL_SCANCODE_RETURN: {
      if (index == max_index && cb == nullptr &&
          type == NpcDlgEnum::quest_avaliable) {
        event_button_quest_yes();
        return false;
      }
      if (type == NpcDlgEnum::quest_avaliable && index < max_index) {
        event_button_next();
        return false;
      }
      if (type == NpcDlgEnum::quest_complete) {
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
      if (CursorGameInstance::cursor_ui == render) {
        time = UINT64_MAX;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
        time = UINT64_MAX;
        event_list();
        event_button(event);
        if (type == NpcDlgEnum::select && select_mouse >= 0) {
          event_button_next();
        }
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_motion_bt_audio(event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

bool NpcDlgUiSystem::start_next_quest() {
  auto node = QuestGameInstance::load_quest_node(quest_id);
  if (auto n = node->find(u"Act/" + quest_index + u"/nextQuest");
      n != nullptr) {
    QuestGameInstance::complete_quest(selected);
    QuestAlarmUiSystem::complete_quest(selected);
    QuestUiSystem::complete_quest(quest_id);
    auto nq = static_cast<wz::Property<int> *>(n)->get();
    auto tmp = std::to_string(nq);
    selected = std::u16string{tmp.begin(), tmp.end()} + u".img";
    type = NpcDlgEnum::quest;
    event_list();
    return true;
  }
  return false;
}