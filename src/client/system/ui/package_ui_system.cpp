#include "package_ui_system.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "notice_ui_system.h"
#include "scroll_ui_system.h"
#include "shop_ui_system.h"
#include "src/client/game/game_item.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_character_instance.h"
#include "tooltip_ui_system.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

std::vector<uint32_t> package_ui_system::load_blank_index(uint32_t tab) {
  std::vector<uint32_t> r;
  auto &d = package_game_instance::data[tab];
  for (int32_t i = 0; i < d.size(); i++) {
    if (d[i]->id.empty()) {
      r.push_back(i);
    }
  }
  return r;
}

std::vector<uint32_t>
package_ui_system::load_b_index(std::polymorphic<game_item> &item) {
  if (item->id == u"00000000") {
    // meso
    return {0};
  }
  if (!(item->type == item_enum::consume || item->type == item_enum::etc)) {
    return load_blank_index((int)item->type);
  }
  std::vector<uint32_t> blank;
  auto num = item_game_instance::load_item_num(item);
  std::vector<std::polymorphic<game_item>> *r;
  r = &package_game_instance::data[(int)item->type];
  auto slot_max = item_game_instance::load_slot_max(item->id);
  std::vector<int32_t> same_slots;
  std::vector<int32_t> add_slots;
  for (int32_t i = 0; i < r->size(); i++) {
    auto itm = r->at(i);
    if (itm->id == item->id) {
      same_slots.push_back(i);
    } else if (itm->id.empty()) {
      add_slots.push_back(i);
    }
  }
  same_slots.append_range(add_slots);
  for (auto i : same_slots) {
    auto itm = r->at(i);
    if (itm->id == item->id) {
      auto itm_num = item_game_instance::load_item_num(itm);
      if (itm_num < slot_max) {
        num = num - (slot_max - itm_num);
      }
      blank.push_back(i);
    } else if (itm->id.empty()) {
      num -= slot_max;
      blank.push_back(i);
    }
    if (num <= 0) {
      break;
    }
  }
  if (num <= 0) {
    return blank;
  } else {
    return {};
  }
}

std::optional<uint32_t> package_ui_system::load_mouse_index() {
  auto cursor_in = cursor_game_instance::cursor_ui;
  if (cursor_in != render) {
    return std::nullopt;
  }
  SDL_FPoint slot_pos{8, 51};
  const auto slot_space_x = 4;
  const auto slot_space_y = 2;

  auto &mouse_pos = window::mouse_pos;
  auto lx = pos.x + slot_pos.x;
  auto rx = lx + 36 * 5;

  auto ty = pos.y + slot_pos.y;
  auto by = ty + 34 * 6;
  if ((lx <= mouse_pos.x && mouse_pos.x <= rx) &&
      (ty <= mouse_pos.y && mouse_pos.y <= by)) {
    auto dx = int((mouse_pos.x - lx) / 36);
    auto dy = int((mouse_pos.y - ty) / 34);
    auto index = dy * 5 + dx;
    index += page * 5;
    return index;
  }

  return std::nullopt;
}

uint32_t package_ui_system::load_full_item_num(const std::u16string &id) {
  uint32_t r = 0;
  auto itm = item_game_instance::load_item(id, 1);
  auto &p = package_game_instance::data[(int)itm->type];
  for (auto &i : p) {
    if (i->id == id) {
      auto num = item_game_instance::load_item_num(i);
      r += num;
    }
  }
  return r;
}

void package_ui_system::render_active_ball() {
  auto &self = character_game_instance::self;
  auto ball_type = ball_game_instance::load_ball_type(self);
  auto ball_id = ball_game_instance::load_pkg_ball(1, ball_type);
  if (!ball_id.empty()) {
    auto p = package_game_instance::data[(int)item_enum::consume];
    for (int i = 0; i < p.size(); i++) {
      auto itm = p[i];
      auto itm_num = item_game_instance::load_item_num(itm);
      if (itm->id == ball_id && itm_num > 0) {
        // render
        static auto t = wz_resource::load_texture(
            wz_resource::ui->find(u"UI/Item.img/canvas:activeIcon"));
        SDL_FRect pos_rect;

        return;
      }
    }
  }
}

void package_ui_system::render_backgrnd() {
  static auto texture =
      wz_resource::load_texture(wz_resource::ui->find(u"Item.img/backgrnd"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

void package_ui_system::render_number(uint32_t num, int x, int y) {
  auto node = wz_resource::ui->find(u"Basic.img/ItemNo");
  // 计算数字位数
  int digits = num == 0 ? 1 : static_cast<int>(std::log10(num)) + 1;
  // 从最高位开始遍历
  int w = 0;
  for (int i = digits - 1; i >= 0; --i) {
    int divisor = static_cast<int>(std::pow(10, i));
    int digit = (num / divisor) % 10;
    auto t = wz_resource::load_texture(node->get_child(std::to_string(digit)));
    SDL_FRect pos_rect = {
        (float)x + w,
        (float)y,
        (float)t->w,
        (float)t->h,
    };
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
    w += t->w;
  }
}

void package_ui_system::render_tab() {
  const static std::array tab_pos = {
      SDL_FPoint{5, 24},   //
      SDL_FPoint{38, 24},  //
      SDL_FPoint{71, 24},  //
      SDL_FPoint{104, 24}, //
      SDL_FPoint{137, 24}, //
      SDL_FPoint{170, 24}, //
  };
  const static auto tab_node = wz_resource::ui->find(u"Item.img/Tab");
  const static std::array active_texture = {
      wz_resource::load_texture(tab_node->find(u"enabled/0")),
      wz_resource::load_texture(tab_node->find(u"enabled/1")),
      wz_resource::load_texture(tab_node->find(u"enabled/2")),
      wz_resource::load_texture(tab_node->find(u"enabled/3")),
      wz_resource::load_texture(tab_node->find(u"enabled/4")),
      wz_resource::load_texture(tab_node->find(u"enabled/5")),
  };
  const static std::array disabled_texture = {
      wz_resource::load_texture(tab_node->find(u"disabled/0")),
      wz_resource::load_texture(tab_node->find(u"disabled/1")),
      wz_resource::load_texture(tab_node->find(u"disabled/2")),
      wz_resource::load_texture(tab_node->find(u"disabled/3")),
      wz_resource::load_texture(tab_node->find(u"disabled/4")),
      wz_resource::load_texture(tab_node->find(u"disabled/5")),
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
  if (new_itm.has_value()) {
    // render new tab
    if (active_tab != (int)new_itm->type) {
      auto now = window::dt_now;
      auto new_item_node = wz_resource::ui->find(u"Item.img/New/Tab0");
      auto sum = new_item_node->children_count() * 100;
      auto offset = now % sum; // 取余，得到周期内偏移
      auto i = offset / 100;
      new_item_node = new_item_node->get_child(std::to_string(i));
      auto icon = wz_resource::load_texture(new_item_node);
      auto origin =
          wz_resource::load_fpoint(new_item_node->get_child(u"origin"));
      auto pos_point = tab_pos[(int)new_itm->type];
      SDL_FRect pos_rect{
          pos_point.x + pos.x - origin.x,
          pos_point.y + pos.y - origin.y,
          static_cast<float>(icon->w),
          static_cast<float>(icon->h),
      };
      SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
    }
  }
}

bool package_ui_system::render_items_info() {
  if (cursor_game_instance::modal_overlay) {
    return true;
  }
  constexpr int tooltip_offset = 15;
  auto index_opt = load_mouse_index();
  if (!index_opt.has_value()) {
    return true;
  }
  const size_t index = static_cast<size_t>(index_opt.value());
  auto &items = package_game_instance::data[active_tab];

  if (index >= items.size() || items[index]->id.empty()) {
    return true;
  }
  const SDL_FPoint show_pos{
      window::mouse_pos.x + tooltip_offset,
      window::mouse_pos.y + tooltip_offset,
  };

  // 统一使用指针，通过重载或模板区分类型
  if (active_tab == (int)item_enum::equip) {
    auto &equip = static_cast<game_equip_item &>(*items[index]);
    tooltip_ui_system::render_equip(equip, show_pos.x, show_pos.y);
  } else if (active_tab == (int)item_enum::deco) {
    auto &deco = static_cast<game_deco_item &>(*items[index]);
    tooltip_ui_system::render_deco(deco, show_pos.x, show_pos.y);
  } else {
    tooltip_ui_system::render_item(*items[index], show_pos.x, show_pos.y);
  }
  return true;
}

void package_ui_system::render_items() {
  constexpr SDL_FPoint slot_pos{8, 51};
  constexpr int slot_space_x = 4;
  constexpr int slot_space_y = 2;
  constexpr int slot_size = 32;
  constexpr int slots_per_row = 5;
  constexpr int max_rows = 6;
  constexpr int items_per_page = slots_per_row * max_rows;

  // 获取数据
  auto &items = package_game_instance::data[active_tab];

  const size_t start_index = static_cast<size_t>(page) * slots_per_row;
  const size_t end_index = std::min(start_index + items_per_page, items.size());

  for (size_t i = start_index; i < start_index + 30; ++i) {
    // 计算行列
    const size_t local_index = i - start_index;
    const int row = local_index / slots_per_row;
    const int col = local_index % slots_per_row;
    if (i >= items.size()) {
      auto x = pos.x + slot_pos.x + col * (slot_size + slot_space_x);
      auto y = pos.y + slot_pos.y + row * (slot_size + slot_space_y);

      auto icon = wz_resource::load_texture(
          wz_resource::ui->find(u"Item.img/canvas:disabled"));
      SDL_FRect pos_rect{
          std::floor(x),
          std::floor(y),
          static_cast<float>(icon->w),
          static_cast<float>(icon->h),
      };
      SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
      continue;
    }
    auto &item = items[i];
    if (item->id.empty())
      continue;

    wz::Node *info;
    // 加载信息
    if (active_tab == (int)item_enum::equip ||
        active_tab == (int)item_enum::deco) {
      info = equip_game_instance::load_equip_info(item->id);
    } else {
      info = item_game_instance::load_item_info(item->id, 0);
    }

    auto icon = wz_resource::load_texture(info->get_child(u"icon"));
    if (!icon)
      continue;

    // 计算位置
    const float x = pos.x + slot_pos.x + col * (slot_size + slot_space_x) +
                    (slot_size - icon->w) * 0.5f;
    const float y = pos.y + slot_pos.y + row * (slot_size + slot_space_y) +
                    (slot_size - icon->h) * 0.5f;

    SDL_FRect pos_rect{
        std::floor(x),
        std::floor(y),
        static_cast<float>(icon->w),
        static_cast<float>(icon->h),
    };

    SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);

    if (active_tab == (int)item_enum::deco) {
      icon = wz_resource::load_texture(
          wz_resource::ui->find(u"CashShop.img/CashItem/0"));
      pos_rect.x =
          (int)pos.x + slot_pos.x + col * (slot_size + slot_space_x) + 19;
      pos_rect.y =
          (int)pos.y + slot_pos.y + row * (slot_size + slot_space_y) + 19;
      pos_rect.w = icon->w;
      pos_rect.h = icon->h;
      SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
    }

    // render new item
    if (new_itm.has_value()) {
      if (active_tab == (int)new_itm->type && i == new_itm->index) {
        auto now = window::dt_now;
        auto new_item_node = wz_resource::ui->find(u"Item.img/New/inventory");
        auto sum = new_item_node->children_count() * 100;
        auto offset = now % sum; // 取余，得到周期内偏移
        auto i = offset / 100;
        new_item_node = new_item_node->get_child(std::to_string(i));
        icon = wz_resource::load_texture(new_item_node);
        pos_rect.x =
            (int)pos.x + slot_pos.x + col * (slot_size + slot_space_x) - 1;
        pos_rect.y =
            (int)pos.y + slot_pos.y + row * (slot_size + slot_space_y) - 2;
        pos_rect.w = icon->w;
        pos_rect.h = icon->h;
        SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
      }
    }

    // render num
    auto item_num = item_game_instance::load_item_num(item);
    if (item->type == item_enum::consume || item->type == item_enum::etc) {
      package_ui_system::render_number(
          item_num, pos.x + slot_pos.x + col * (slot_size + slot_space_x) + 1,
          pos.y + slot_pos.y + row * (slot_size + slot_space_y) + 20);
    }
  }
}

void package_ui_system::render_scroll() {
  const SDL_FPoint lt{188, 51};
  const uint32_t length = 202;
  const auto &r = package_game_instance::data[active_tab];
  auto size = static_cast<size_t>(std::ceil(r.size() / 5.0));
  auto cursor_in = cursor_game_instance::cursor_ui;
  bool top = cursor_in == render && !cursor_game_instance::modal_overlay;
  scroll_ui_system::render_vscroll((int)pos.x + lt.x, (int)pos.y + lt.y, page,
                                   size, length, top, 6);
  return;
}

void package_ui_system::render_button() {
  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"Basic.img/BtClose"),
  };
  auto wh = load_wh();
  const std::array buttons_rect = {
      SDL_FRect{wh.x - 20, 7, 12, 12}, //
  };

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

void package_ui_system::render_meso() {
  freetype::load_size(12);
  freetype::load_aligned(true);
  freetype::load_color(0, 0, 0, 255);
  auto meso = std::to_string(package_game_instance::meso);
  std::u16string meso2 = {meso.begin(), meso.end()};
  auto w = freetype::load_w(meso2);
  freetype::draw_line(meso2, pos.x + 165 - w, pos.y + 266);
}

bool package_ui_system::render() {
  render_backgrnd();
  render_tab();
  render_items();
  render_scroll();
  render_button();
  render_meso();
  return true;
}

SDL_FPoint package_ui_system::load_wh() { return {209, 289}; }

void package_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

void package_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  new_itm = std::nullopt;
}

void package_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

bool package_ui_system::event_click_item(SDL_Event *event) {
  // 网络手占用时拒绝操作
  if (cursor_game_instance::cursor_hand_net.has_value()) {
    return false;
  }

  auto index = load_mouse_index();
  if (!index.has_value()) {
    return false;
  }

  // 无手持物品：拾取
  if (!cursor_game_instance::cursor_hand.has_value()) {
    const auto &r = package_game_instance::data[active_tab];
    if (index.value() >= r.size() || r[index.value()]->id.empty()) {
      return false;
    }
    cursor_game_instance::cursor_hand = {
        .type = cursor_game_instance::package,
        .val = active_tab,
        .sub_val = index.value(),
    };
    return true;
  }

  // 新获取的物品
  if (new_itm.has_value()) {
    if (active_tab == (int)new_itm->type && new_itm->index == index.value()) {
      new_itm = std::nullopt;
    }
  }
  // 有手持物品
  auto &sf = character_game_instance::self;
  auto &hand = cursor_game_instance::cursor_hand.value();
  switch ((cursor_game_instance::cursor_hand_type)hand.type) {
  case cursor_game_instance::equipment: {
    switch (hand.val) {
    case 0: {
      if (active_tab == 0) {
        auto &r = package_game_instance::data[0];
        if (r[index.value()]->id.empty()) {
          auto eqp = equip_ui_system::load_equip(
              (equip_ui_system::equip_mouse_index)hand.sub_val);
          auto eqp2 = std::polymorphic<game_item>(eqp->value());
          r[index.value()] = eqp2;
          *eqp = std::nullopt;
          cursor_game_instance::cursor_hand = std::nullopt;
          character_logic_system::cct.map_id = scene_system_instance::map_id;
        }
      }
      break;
    }
    }
    break;
  }
  case cursor_game_instance::deco: {
    break;
  }
  case cursor_game_instance::package: {
    if (hand.val != active_tab) {
      cursor_game_instance::cursor_hand = std::nullopt;
      return true;
    }
    switch ((item_enum)hand.val) {
    case item_enum::equip: {
      auto &equips = package_game_instance::data[0];
      // 点击的是同一个格子：尝试穿戴
      if (hand.sub_val == index.value()) {
        auto equip = static_cast<game_equip_item &>(*equips[index.value()]);
        auto ev = equip_game_instance::load_equip_slot(equip, sf);

        if (!equip_game_instance::add_equip_limit(equip, sf, 0)) {
          notice_ui_system::type =
              notice_ui_system::notice_enum::equip_no_ability;
          notice_ui_system::open();
          cursor_game_instance::cursor_hand = std::nullopt;
          return true;
        }

        auto blank_slot = load_blank_index(active_tab);
        blank_slot.push_back(index.value());
        std::ranges::sort(blank_slot);

        if (blank_slot.size() < ev.size()) {
          notice_ui_system::type =
              notice_ui_system::notice_enum::equip_no_space;
          notice_ui_system::open();
          cursor_game_instance::cursor_hand = std::nullopt;
          return true;
        }

        // 执行穿戴
        equips[hand.sub_val]->id = u"";
        for (int32_t i = 0; i < ev.size(); i++) {
          auto eqp = ev[i];
          equips[blank_slot[i]] = std::polymorphic<game_item>(
              std::in_place_type<game_equip_item>, eqp);
        }
        // 发包
        character_logic_system::cct.map_id = scene_system_instance::map_id;

        cursor_game_instance::cursor_hand = std::nullopt;
        return true;
      }

      // 点击不同格子：交换
      std::swap(equips[hand.sub_val], equips[index.value()]);
      cursor_game_instance::cursor_hand = std::nullopt;
      return true;
      break;
    }
    case item_enum::etc:
    case item_enum::consume: {
      // 处理普通背包栏
      auto &r = package_game_instance::data[active_tab];
      if (hand.sub_val != index.value()) {
        // 是否可堆叠，尝试合并
        auto &itm0 = r.at(index.value());
        auto &itm1 = r.at(hand.sub_val);
        if (itm0->id == itm1->id) {
          auto itm_num0 = item_game_instance::load_item_num(itm0);
          auto itm_num1 = item_game_instance::load_item_num(itm1);
          auto slot_max = item_game_instance::load_slot_max(itm0->id);
          auto add_num = std::min(slot_max - itm_num0, itm_num1);
          add_item_num(itm0, add_num);
          dec_item_num(itm1, add_num);
        } else {
          std::swap(r.at(hand.sub_val), r.at(index.value()));
        }
      } else {
        // 使用道具
        auto &itm = r.at(index.value());
        if (character_logic_system::run_item(sf, itm, 1)) {
          dec_item_num(itm, 1);
        }
      }
      cursor_game_instance::cursor_hand = std::nullopt;
      break;
    }
    default: {
      break;
    }
    }
    break;
  }
  default: {
    break;
  }
  }
  return true;
}

void package_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void package_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void package_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void package_ui_system::toggle() {
  audio_game_instance::load_audio(u"UI.img/BtMouseClick", 0);
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool package_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void package_ui_system::event_tab(SDL_Event *event) {
  const static std::array tab_rect = {
      SDL_FRect{5, 24, 33, 19},   //
      SDL_FRect{38, 24, 33, 19},  //
      SDL_FRect{71, 24, 33, 19},  //
      SDL_FRect{104, 24, 33, 19}, //
      SDL_FRect{137, 24, 33, 19}, //
      SDL_FRect{170, 24, 33, 19}, //
  };
  for (uint8_t i = 0; i < tab_rect.size(); i++) {
    auto pos_rect = tab_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      if (active_tab == i) {
        return;
      }
      if (new_itm.has_value()) {
        if ((int)new_itm->type == active_tab) {
          new_itm = std::nullopt;
        }
      }
      page = 0;
      active_tab = i;
      return;
    }
  }
}

void package_ui_system::event_close() { close(); }

bool package_ui_system::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto wh = load_wh();
  r = {
      SDL_FRect{wh.x - 20, 7, 12, 12}, //
  };
  fns = {
      event_close,
  };

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

void package_ui_system::event_motion(SDL_Event *event) {
  auto &sys = system::render_systems;
  std::erase(sys, render_items_info);
  auto it = std::ranges::find(sys, &cursor_render_system::render);
  if (it != sys.end()) {
    sys.insert(it, render_items_info);
  }
}

void package_ui_system::event_vscr_move(SDL_Event *event) {
  auto mouse_state = window::mouse_state;
  if (vscr_motion) {
    const SDL_FPoint lt{188, 51};
    const uint32_t length = 202;

    event->button.x = pos.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos.y + lt.y, pos.y + lt.y + length);
    event_vscr(event);
  }
}

bool package_ui_system::event_vscr(SDL_Event *event) {
  const SDL_FPoint lt{188, 51};
  const uint32_t length = 202;

  auto &items = package_game_instance::data[active_tab];
  auto size = static_cast<int>(std::ceil((items.size() - 30) / 5.0));
  size = std::max(0, size);

  auto cursor_in = cursor_game_instance::cursor_ui;
  size = std::max(0, size);
  auto mouse_pos = SDL_FPoint{event->button.x, event->button.y};
  auto val = scroll_ui_system::click_vscroll(pos.x + lt.x, pos.y + lt.y, page,
                                             size, length, true, mouse_pos);
  page = val;
  return true;
}

void package_ui_system::event_vscr_start(SDL_Event *event) {
  const SDL_FPoint lt{188, 51};
  const uint32_t length = 202;
  if (vscr_motion == false) {
    vscr_motion =
        scroll_ui_system::click_thumb(pos.x + lt.x, pos.y + lt.y, length);
  }
}

void package_ui_system::event_vscr_end() { vscr_motion = false; }

bool package_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      event_close();
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
        event_tab(event);
        event_click_item(event);
        r = event_button(event);
        event_vscr(event);
      }
      event_drag_end();
      event_vscr_end();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_motion(event);
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

std::optional<int>
package_ui_system::add_item(std::polymorphic<game_item> &item) {
  return shop_ui_system::add_item(item);
}

void package_ui_system::add_item_num(std::polymorphic<game_item> &item,
                                     int num) {
  shop_ui_system::add_item_num(item, num);
  return;
}

void package_ui_system::dec_item_num(std::polymorphic<game_item> &item,
                                     int num) {
  shop_ui_system::dec_item_num(item, num);
  return;
}

std::polymorphic<game_item> *
package_ui_system::load_f_item(const std::u16string &id) {
  auto type = item_game_instance::load_item_type(id);
  std::vector<std::polymorphic<game_item>> *r;
  if (type == u"Cash" || type == u"Pet") {
    auto r = package_game_instance::data[(int)item_enum::cash];
  } else if (type == u"Consume") {
    auto r = package_game_instance::data[(int)item_enum::consume];
  } else if (type == u"Install") {
    auto r = package_game_instance::data[(int)item_enum::install];
  }
  for (auto &itm : *r) {
    if (itm->id == id) {
      return &itm;
    }
  }
  return nullptr;
}