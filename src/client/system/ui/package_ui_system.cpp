#include "package_ui_system.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "notice_ui_system.h"
#include "scroll_ui_system.h"
#include "src/client/game/game_item.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
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
    if (!d[i]) {
      r.push_back(i);
    }
  }
  return r;
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
    index += page;
    return index;
  }

  return std::nullopt;
}

void package_ui_system::render_backgrnd() {
  static auto texture =
      wz_resource::load_texture(wz_resource::ui->find(u"Item.img/backgrnd"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
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
    SDL_FRect pos_rect{static_cast<float>(int(pos.x + tab_pos[i].x)),
                       static_cast<float>(int(pos.y + tab_pos[i].y)),
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  }
}

void package_ui_system::render_items_info() {
  constexpr int tooltip_offset = 15;
  auto index_opt = load_mouse_index();
  if (!index_opt.has_value()) {
    return;
  }
  const size_t index = static_cast<size_t>(index_opt.value());
  const auto &items = package_game_instance::data[active_tab];

  if (index >= items.size() || !items[index]) {
    return;
  }
  const SDL_FPoint show_pos{
      window::mouse_pos.x + tooltip_offset,
      window::mouse_pos.y + tooltip_offset,
  };

  // 统一使用指针，通过重载或模板区分类型
  if (active_tab == 0) {
    auto &equip = static_cast<game_equip_item &>(*items[index]);
    tooltip_ui_system::render_equip(equip, show_pos.x, show_pos.y);
  } else {
    tooltip_ui_system::render_item(*items[index], show_pos.x, show_pos.y);
  }
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
  const auto &items = (active_tab == 0)
                          ? package_game_instance::data[0]
                          : package_game_instance::data[active_tab];

  // 选择加载函数
  auto load_info = (active_tab == 0) ? &equip_game_instance::load_equip_info
                                     : &item_game_instance::load_item_info;

  const size_t start_index = static_cast<size_t>(page) * slots_per_row;
  const size_t end_index = std::min(start_index + items_per_page, items.size());

  for (size_t i = start_index; i < end_index; ++i) {
    const auto &item_ptr = items[i];
    if (!item_ptr)
      continue;

    // 计算行列
    const size_t local_index = i - start_index;
    const int row = local_index / slots_per_row;
    const int col = local_index % slots_per_row;

    // 加载信息
    auto info = load_info(item_ptr->id);
    if (!info)
      continue;

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
  }
}

void package_ui_system::render_scroll() {
  const SDL_FPoint lt{188, 51};
  const uint32_t length = 202;
  const auto &r = package_game_instance::data[active_tab];
  auto size = r.size() / 5;
  auto cursor_in = cursor_game_instance::cursor_ui;
  bool top = cursor_in == render && !cursor_game_instance::modal_overlay;
  scroll_ui_system::render_vscroll((int)pos.x + lt.x, (int)pos.y + lt.y, page,
                                   size, length, top, 30);
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
  render_items_info();
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
  }
}

void package_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void package_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
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
    if (index.value() >= r.size() || !r[index.value()]) {
      return false;
    }
    cursor_game_instance::cursor_hand = {
        .type = cursor_game_instance::package,
        .val = active_tab,
        .sub_val = index.value(),
    };
    return true;
  }

  // 有手持物品
  auto &hand = cursor_game_instance::cursor_hand.value();

  // 手持物品不匹配当前标签页，清除手持状态
  if (hand.type != cursor_game_instance::package || hand.val != active_tab) {
    cursor_game_instance::cursor_hand = std::nullopt;
    return true;
  }

  // 处理装备栏
  if (active_tab == static_cast<int>(item_enum::equip)) {
    auto &equips = package_game_instance::data[0];
    auto &sf = character_game_instance::self;

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
        notice_ui_system::type = notice_ui_system::notice_enum::equip_no_space;
        notice_ui_system::open();
        cursor_game_instance::cursor_hand = std::nullopt;
        return true;
      }

      // 执行穿戴
      equips[hand.sub_val] = nullptr;
      for (int32_t i = 0; i < ev.size(); i++) {
        equips[blank_slot[i]] = std::make_unique<game_equip_item>(ev[i]);
      }
      character_game_instance::self = sf;

      // 发包
      ClientCharacterT ct;
      ct.map_id = scene_system_instance::map_id;
      auto c = character_game_instance::load_characterT(sf);
      ct.payload = std::make_unique<fbs::CharacterT>(std::move(c));
      client_request::send_to_host(ct);

      cursor_game_instance::cursor_hand = std::nullopt;
      return true;
    }

    // 点击不同格子：交换
    std::swap(equips[hand.sub_val], equips[index.value()]);
    cursor_game_instance::cursor_hand = std::nullopt;
    return true;
  }

  // 处理普通背包栏
  auto &r = package_game_instance::data[active_tab];
  if (hand.sub_val != index.value()) {
    std::swap(r.at(hand.sub_val), r.at(index.value()));
  }
  cursor_game_instance::cursor_hand = std::nullopt;
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
      active_tab = i;
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