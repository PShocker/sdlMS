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
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_character_instance.h"
#include "tooltip_ui_system.h"
#include "wz/Property.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

void PackageUiSystem::cal_mouse_index() {
  mouse_index = std::nullopt;
  auto cursor_in = CursorGameInstance::cursor_ui;
  if (cursor_in != render) {
    return;
  }
  SDL_FPoint slot_pos{8, 51};
  const auto slot_space_x = 4;
  const auto slot_space_y = 2;

  auto &mouse_pos = Window::mouse_pos;
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
    mouse_index = index;
  }
  return;
}

void PackageUiSystem::render_backgrnd() {
  static auto texture =
      WzResource::load_texture(WzResource::ui->find(u"Item.img/backgrnd"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
}

void PackageUiSystem::render_number(uint32_t num, int x, int y) {
  auto node = WzResource::ui->find(u"Basic.img/ItemNo");
  // 计算数字位数
  int digits = num == 0 ? 1 : static_cast<int>(std::log10(num)) + 1;
  // 从最高位开始遍历
  int w = 0;
  for (int i = digits - 1; i >= 0; --i) {
    int divisor = static_cast<int>(std::pow(10, i));
    int digit = (num / divisor) % 10;
    auto t = WzResource::load_texture(node->get_child(std::to_string(digit)));
    SDL_FRect pos_rect = {
        (float)x + w,
        (float)y,
        (float)t->w,
        (float)t->h,
    };
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    w += t->w;
  }
}

void PackageUiSystem::render_tab() {
  const static std::array tab_pos = {
      SDL_FPoint{5, 24},   //
      SDL_FPoint{38, 24},  //
      SDL_FPoint{71, 24},  //
      SDL_FPoint{104, 24}, //
      SDL_FPoint{137, 24}, //
      SDL_FPoint{170, 24}, //
  };
  const static auto tab_node = WzResource::ui->find(u"Item.img/Tab");
  const static std::array active_texture = {
      WzResource::load_texture(tab_node->find(u"enabled/0")),
      WzResource::load_texture(tab_node->find(u"enabled/1")),
      WzResource::load_texture(tab_node->find(u"enabled/2")),
      WzResource::load_texture(tab_node->find(u"enabled/3")),
      WzResource::load_texture(tab_node->find(u"enabled/4")),
      WzResource::load_texture(tab_node->find(u"enabled/5")),
  };
  const static std::array disabled_texture = {
      WzResource::load_texture(tab_node->find(u"disabled/0")),
      WzResource::load_texture(tab_node->find(u"disabled/1")),
      WzResource::load_texture(tab_node->find(u"disabled/2")),
      WzResource::load_texture(tab_node->find(u"disabled/3")),
      WzResource::load_texture(tab_node->find(u"disabled/4")),
      WzResource::load_texture(tab_node->find(u"disabled/5")),
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
  if (new_itm.has_value()) {
    // render new tab
    if (active_tab != (int)new_itm->type) {
      auto now = Window::dt_now;
      auto new_item_node = WzResource::ui->find(u"Item.img/New/Tab0");
      auto sum = new_item_node->children_count() * 100;
      auto offset = now % sum; // 取余，得到周期内偏移
      auto i = offset / 100;
      new_item_node = new_item_node->get_child(std::to_string(i));
      auto icon = WzResource::load_texture(new_item_node);
      auto origin =
          WzResource::load_fpoint(new_item_node->get_child(u"origin"));
      auto pos_point = tab_pos[(int)new_itm->type];
      SDL_FRect pos_rect{
          pos_point.x + pos.x - origin.x,
          pos_point.y + pos.y - origin.y,
          static_cast<float>(icon->w),
          static_cast<float>(icon->h),
      };
      SDL_RenderTexture(Window::renderer, icon, nullptr, &pos_rect);
    }
  }
}

bool PackageUiSystem::render_items_info() {
  if (CursorGameInstance::cursor_ui != render) {
    return true;
  }
  if (CursorGameInstance::modal_overlay) {
    return true;
  }
  constexpr int tooltip_offset = 15;
  auto index_opt = mouse_index;
  if (!index_opt.has_value()) {
    return true;
  }
  const size_t index = static_cast<size_t>(index_opt.value());
  auto &items = PackageGameInstance::data[active_tab];

  if (index >= items.size() || items[index]->id.empty()) {
    return true;
  }
  const SDL_FPoint show_pos{
      Window::mouse_pos.x + tooltip_offset,
      Window::mouse_pos.y + tooltip_offset,
  };

  // 统一使用指针，通过重载或模板区分类型
  if (active_tab == (int)ItemEnum::kEquip) {
    auto &equip = static_cast<GameEquipItem &>(*items[index]);
    TooltipUiSystem::render_equip(equip, show_pos.x, show_pos.y);
  } else if (active_tab == (int)ItemEnum::kDeco) {
    auto &deco = static_cast<GameDecoItem &>(*items[index]);
    TooltipUiSystem::render_deco(deco, show_pos.x, show_pos.y);
  } else {
    TooltipUiSystem::render_item(*items[index], show_pos.x, show_pos.y);
  }
  return true;
}

void PackageUiSystem::render_items() {
  constexpr SDL_FPoint slot_pos{8, 51};
  constexpr int slot_space_x = 4;
  constexpr int slot_space_y = 2;
  constexpr int slot_size = 32;
  constexpr int slots_per_row = 5;
  constexpr int max_rows = 6;
  constexpr int items_per_page = slots_per_row * max_rows;

  // 获取数据
  auto &items = PackageGameInstance::data[active_tab];

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

      auto icon = WzResource::load_texture(
          WzResource::ui->find(u"Item.img/canvas:disabled"));
      SDL_FRect pos_rect{
          std::floor(x),
          std::floor(y),
          static_cast<float>(icon->w),
          static_cast<float>(icon->h),
      };
      SDL_RenderTexture(Window::renderer, icon, nullptr, &pos_rect);
      continue;
    }
    auto &item = items[i];
    if (item->id.empty())
      continue;

    wz::Node *info;
    // 加载信息
    if (active_tab == (int)ItemEnum::kEquip ||
        active_tab == (int)ItemEnum::kDeco) {
      info = EquipGameInstance::load_equip_info(item->id);
    } else {
      info = ItemGameInstance::load_item_info(item->id, 0);
    }

    auto icon = WzResource::load_texture(info->get_child(u"icon"));
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

    SDL_RenderTexture(Window::renderer, icon, nullptr, &pos_rect);

    if (active_tab == (int)ItemEnum::kDeco) {
      icon = WzResource::load_texture(
          WzResource::ms->get_root()->find(u"UI.img/CashItem"));
      pos_rect.x =
          (int)pos.x + slot_pos.x + col * (slot_size + slot_space_x) + 19;
      pos_rect.y =
          (int)pos.y + slot_pos.y + row * (slot_size + slot_space_y) + 19;
      pos_rect.w = icon->w;
      pos_rect.h = icon->h;
      SDL_RenderTexture(Window::renderer, icon, nullptr, &pos_rect);
    }

    // render new item
    if (new_itm.has_value()) {
      if (active_tab == (int)new_itm->type && i == new_itm->index) {
        auto now = Window::dt_now;
        auto new_item_node = WzResource::ui->find(u"Item.img/New/inventory");
        auto sum = new_item_node->children_count() * 100;
        auto offset = now % sum; // 取余，得到周期内偏移
        auto i = offset / 100;
        new_item_node = new_item_node->get_child(std::to_string(i));
        icon = WzResource::load_texture(new_item_node);
        pos_rect.x =
            (int)pos.x + slot_pos.x + col * (slot_size + slot_space_x) - 1;
        pos_rect.y =
            (int)pos.y + slot_pos.y + row * (slot_size + slot_space_y) - 2;
        pos_rect.w = icon->w;
        pos_rect.h = icon->h;
        SDL_RenderTexture(Window::renderer, icon, nullptr, &pos_rect);
      }
    }
    // render active ball
    if (active_tab == (int)ItemEnum::kConsume) {
      auto active_ball = PackageGameInstance::load_active_ball();
      if (active_ball == &item) {
        auto node = WzResource::ui->find(u"Item.img/canvas:activeIcon");
        icon = WzResource::load_texture(node);
        pos_rect.x =
            (int)pos.x + slot_pos.x + col * (slot_size + slot_space_x) - 1;
        pos_rect.y =
            (int)pos.y + slot_pos.y + row * (slot_size + slot_space_y) - 2;
        pos_rect.w = icon->w;
        pos_rect.h = icon->h;
        SDL_RenderTexture(Window::renderer, icon, nullptr, &pos_rect);
      }
    }

    // render num
    auto item_num = ItemGameInstance::load_item_num(item);
    if (item->type == ItemEnum::kConsume || item->type == ItemEnum::kEtc) {
      PackageUiSystem::render_number(
          item_num, pos.x + slot_pos.x + col * (slot_size + slot_space_x) + 1,
          pos.y + slot_pos.y + row * (slot_size + slot_space_y) + 20);
    }
  }
}

void PackageUiSystem::render_scroll() {
  const SDL_FPoint lt{188, 51};
  const uint32_t length = 202;
  const auto &r = PackageGameInstance::data[active_tab];
  auto size = static_cast<size_t>(std::ceil(r.size() / 5.0));
  auto cursor_in = CursorGameInstance::cursor_ui;
  bool top = cursor_in == render && !CursorGameInstance::modal_overlay;
  ScrollUiSystem::render_vscroll((int)pos.x + lt.x, (int)pos.y + lt.y, page,
                                   size, length, top, 6);
  return;
}

void PackageUiSystem::render_button() {
  const static std::array buttons_nodes = {
      WzResource::ui->find(u"Basic.img/BtClose"),
      WzResource::ui->find(u"Item.img/AutoBuild/button:Coin"),
      WzResource::ui->find(u"Item.img/AutoBuild/button:Gather"),
      WzResource::ui->find(u"Item.img/AutoBuild/button:Sort"),
  };
  auto wh = load_wh();
  const std::array buttons_rect = {
      SDL_FRect{wh.x - 18, 6, 12, 12}, //
      SDL_FRect{8, 268, 14, 14},       //
      SDL_FRect{164, 6, 12, 12},       //
      SDL_FRect{177, 6, 12, 12},       //
  };

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

void PackageUiSystem::render_meso() {
  Freetype::load_size(12);
  Freetype::load_aligned(true);
  Freetype::load_color(0, 0, 0, 255);
  auto meso =
      TextGameInstance::format_with_commas(PackageGameInstance::meso);
  std::u16string meso2 = {meso.begin(), meso.end()};
  auto w = Freetype::load_w(meso2);
  Freetype::draw_line(meso2, pos.x + 165 - w, pos.y + 266);
}

bool PackageUiSystem::render() {
  cal_mouse_index();
  render_backgrnd();
  render_tab();
  render_items();
  render_scroll();
  render_button();
  render_meso();
  return true;
}

SDL_FPoint PackageUiSystem::load_wh() { return {209, 289}; }

void PackageUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    active_tab = 0;

    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

void PackageUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::render_systems, render_items_info);
  std::erase(System::event_systems, event);

  event_drag_end();
  event_vscr_end();
  new_itm = std::nullopt;
}

void PackageUiSystem::event_top() {
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

bool PackageUiSystem::event_click_item(SDL_Event *event) {
  // 网络手占用时拒绝操作
  if (CursorGameInstance::cursor_hand_net.has_value()) {
    return false;
  }

  auto index = mouse_index;
  if (!index.has_value()) {
    return false;
  }

  // 新获取的物品
  if (new_itm.has_value()) {
    if (active_tab == (int)new_itm->type && new_itm->index == index.value()) {
      new_itm = std::nullopt;
    }
  }

  // 无手持物品：拾取
  if (!CursorGameInstance::cursor_hand.has_value()) {
    const auto &r = PackageGameInstance::data[active_tab];
    if (index.value() >= r.size() || r[index.value()]->id.empty()) {
      return false;
    }
    CursorGameInstance::cursor_hand = {
        .type = CursorGameInstance::CursorHandType::kPackage,
        .val = active_tab,
        .sub_val = index.value(),
    };
    AudioGameInstance::load_audio(u"UI.img/DragStart", 0);
    return true;
  }

  // 有手持物品
  auto &sf = CharacterGameInstance::self;
  auto &hand = CursorGameInstance::cursor_hand.value();
  auto &r = PackageGameInstance::data[hand.val];
  if (index.value() >= r.size()) {
    AudioGameInstance::load_audio(u"UI.img/DragEnd", 0);
    CursorGameInstance::cursor_hand = std::nullopt;
    return false;
  }
  switch ((CursorGameInstance::CursorHandType)hand.type) {
  case CursorGameInstance::CursorHandType::kEquipment: {
    if (active_tab != (int)ItemEnum::kEquip) {
      break;
    }
    auto &r = PackageGameInstance::data[(int)ItemEnum::kEquip];
    if (r[index.value()]->id.empty()) {
      auto eqp = EquipGameInstance::load_equip(hand.sub_val);
      auto eqp2 = std::polymorphic<GameItem>(eqp->value());
      r[index.value()] = eqp2;
      *eqp = std::nullopt;
      AudioGameInstance::load_audio(u"UI.img/DragEnd", 0);
      CursorGameInstance::cursor_hand = std::nullopt;
      CharacterLogicSystem::cct.map_id = SceneSystemInstance::map_id;
    }
    break;
  }
  case CursorGameInstance::CursorHandType::kDeco: {
    if (active_tab != (int)ItemEnum::kDeco) {
      break;
    }
    auto &r = PackageGameInstance::data[(int)ItemEnum::kDeco];
    if (r[index.value()]->id.empty()) {
      auto eqp = EquipGameInstance::load_deco(hand.sub_val);
      auto eqp2 = std::polymorphic<GameItem>(eqp->value());
      r[index.value()] = eqp2;
      *eqp = std::nullopt;
      AudioGameInstance::load_audio(u"UI.img/DragEnd", 0);
      CursorGameInstance::cursor_hand = std::nullopt;
      CharacterLogicSystem::cct.map_id = SceneSystemInstance::map_id;
    }
    break;
  }
  case CursorGameInstance::CursorHandType::kPackage: {
    if (hand.val != active_tab) {
      // 只有卷轴类道具可以跨tab使用
      auto &itm = r.at(hand.sub_val);
      if (!((ItemEnum)active_tab == ItemEnum::kEquip &&
            hand.val == (int)ItemEnum::kConsume &&
            itm->id.starts_with(u"0204"))) {
        CursorGameInstance::cursor_hand = std::nullopt;
        return true;
      }
    }
    switch ((ItemEnum)hand.val) {
    case ItemEnum::kEquip: {
      // 点击的是同一个格子：尝试穿戴
      if (hand.val == active_tab) {
        AudioGameInstance::load_audio(u"UI.img/DragEnd", 0);
        if (hand.sub_val == index.value()) {
          EquipGameInstance::use_equip(index.value());
          CursorGameInstance::cursor_hand = std::nullopt;
          return true;
        } else {
          auto &equips = r;
          // 点击不同格子：交换
          std::swap(equips[hand.sub_val], equips[index.value()]);
          CursorGameInstance::cursor_hand = std::nullopt;
          return true;
        }
      } else {
      }
      break;
    }
    case ItemEnum::kDeco: {
      AudioGameInstance::load_audio(u"UI.img/DragEnd", 0);
      if (hand.sub_val == index.value()) {
        EquipGameInstance::use_deco(index.value());
        CursorGameInstance::cursor_hand = std::nullopt;
        return true;
      } else {
        auto &decos = r;
        // 点击不同格子：交换
        std::swap(decos[hand.sub_val], decos[index.value()]);
        CursorGameInstance::cursor_hand = std::nullopt;
        return true;
      }
      break;
    }
    case ItemEnum::kEtc:
    case ItemEnum::kInstall:
    case ItemEnum::kConsume: {
      AudioGameInstance::load_audio(u"UI.img/DragEnd", 0);
      // 处理普通背包栏
      auto &r = PackageGameInstance::data[hand.val];
      if ((int)ItemEnum::kEquip == active_tab) {
        // 卷轴
        auto &equips = PackageGameInstance::data[active_tab];
        auto &equip = equips[index.value()];
        auto &itm = r.at(hand.sub_val);
        auto &eq = static_cast<GameEquipItem &>(*equip);
        auto &it = static_cast<GameConsumeItem &>(*itm);
        EquipGameInstance::use_equip_scroll(eq, it);
        CursorGameInstance::cursor_hand = std::nullopt;
        return true;
      }
      if (hand.sub_val != index.value()) {
        // 是否可堆叠，尝试合并
        auto &itm0 = r.at(index.value());
        auto &itm1 = r.at(hand.sub_val);
        if (itm0->id == itm1->id && !itm0->id.starts_with(u"0207")) {
          auto itm_num0 = ItemGameInstance::load_item_num(itm0);
          auto itm_num1 = ItemGameInstance::load_item_num(itm1);
          auto slot_max = ItemGameInstance::load_slot_max(itm0->id);
          auto add_num = std::min(slot_max - itm_num0, itm_num1);
          ItemGameInstance::add_item_num(itm0, add_num);
          ItemGameInstance::dec_item_num(itm1, add_num);
        } else {
          std::swap(r.at(hand.sub_val), r.at(index.value()));
        }
      } else {
        // 使用道具
        auto &itm = r.at(index.value());
        ItemGameInstance::use_item(itm);
      }
      CursorGameInstance::cursor_hand = std::nullopt;
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

void PackageUiSystem::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void PackageUiSystem::event_drag_end() {
  drag = std::nullopt;
  return;
}

void PackageUiSystem::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = CameraGameInstance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void PackageUiSystem::toggle() {
  AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
  auto fn = &render;
  if (std::ranges::contains(System::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool PackageUiSystem::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void PackageUiSystem::event_tab(SDL_Event *event) {
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
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
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

void PackageUiSystem::event_close() { close(); }

void PackageUiSystem::event_button_coin() {
  NoticeUiSystem::type = NoticeUiSystem::NoticeEnum::kThrowMul;
  NoticeUiSystem::data = nullptr;
  // notice_ui_system::close();
  NoticeUiSystem::open();
}

void PackageUiSystem::event_button_gather() {}

void PackageUiSystem::event_button_sort() {
  const auto cmp = [](const std::polymorphic<GameItem> &a,
                      const std::polymorphic<GameItem> &b) {
    bool aEmpty = a->id.empty();
    bool bEmpty = b->id.empty();
    if (aEmpty != bEmpty)
      return bEmpty;
    return a->id < b->id;
  };
  std::ranges::sort(PackageGameInstance::data[active_tab], cmp);
}

bool PackageUiSystem::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto wh = load_wh();
  r = {
      SDL_FRect{wh.x - 18, 6, 12, 12}, //
      SDL_FRect{8, 268, 14, 14},       //
      SDL_FRect{164, 6, 12, 12},       //
      SDL_FRect{177, 6, 12, 12},       //
  };
  fns = {
      event_close,
      event_button_coin,
      event_button_gather,
      event_button_sort,
  };

  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
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

void PackageUiSystem::event_motion(SDL_Event *event) {
  auto &sys = System::render_systems;
  std::erase(sys, render_items_info);
  auto it = std::ranges::find(sys, &CursorRenderSystem::render);
  if (it != sys.end()) {
    sys.insert(it, render_items_info);
  }
}

void PackageUiSystem::event_vscr_move(SDL_Event *event) {
  auto mouse_state = Window::mouse_state;
  if (vscr_motion) {
    const SDL_FPoint lt{188, 51};
    const uint32_t length = 202;

    event->button.x = pos.x + lt.x;
    event->button.y =
        std::clamp(event->button.y, pos.y + lt.y, pos.y + lt.y + length);
    event_vscr(event);
  }
}

bool PackageUiSystem::event_vscr(SDL_Event *event) {
  const SDL_FPoint lt{188, 51};
  const uint32_t length = 202;

  auto &items = PackageGameInstance::data[active_tab];
  auto size = static_cast<int>(std::ceil((items.size() - 30) / 5.0));

  size = std::max(0, size);
  auto mouse_pos = SDL_FPoint{event->button.x, event->button.y};
  auto val = ScrollUiSystem::click_vscroll(pos.x + lt.x, pos.y + lt.y, page,
                                             size, length, true, mouse_pos);
  page = val;
  return true;
}

void PackageUiSystem::event_vscr_start(SDL_Event *event) {
  const SDL_FPoint lt{188, 51};
  const uint32_t length = 202;
  if (vscr_motion == false) {
    vscr_motion =
        ScrollUiSystem::click_thumb(pos.x + lt.x, pos.y + lt.y, length);
  }
}

void PackageUiSystem::event_vscr_end() { vscr_motion = false; }

void PackageUiSystem::event_wheel(SDL_Event *event) {
  if (CursorGameInstance::cursor_ui != render) {
    return;
  }
  auto dy = event->wheel.integer_y;
  if (dy > 0) {
    // up
    if (page > 0) {
      page -= 1;
    }
  } else {
    // down
    auto &items = PackageGameInstance::data[active_tab];
    auto size = static_cast<int>(std::ceil((items.size() - 30) / 5.0));
    size = std::max(0, size);
    if (page < size) {
      page++;
    }
  }
}

void PackageUiSystem::event_motion_bt_audio(SDL_Event *event) {
  if (CursorGameInstance::cursor_ui != render) {
    return;
  }
  auto wh = load_wh();
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{wh.x - 18, 6, 12, 12}, //
      SDL_FRect{8, 268, 14, 14},       //
      SDL_FRect{164, 6, 12, 12},       //
      SDL_FRect{177, 6, 12, 12},       //
  };
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

bool PackageUiSystem::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      toggle();
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
        event_tab(event);
        event_click_item(event);
        r = !event_button(event);
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
    event_motion_bt_audio(event);
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