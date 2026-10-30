#include "src/client/system/ui/package_ui_system.h"

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
#include "src/client/system/ui/component/ui_component.h"
#include "src/client/system/ui/component/tab.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/component/scroll.h"
#include "src/client/system/ui/shop_ui_system.h"
#include "src/client/system/ui/tooltip_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_character_instance.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <wz/Property.h>

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

namespace sdlms {

void PackageUiSystem::CalMouseIndex() {
  mouse_index_ = std::nullopt;
  auto cursor_in = CursorGameInstance::cursor_ui_;
  if (cursor_in != Render) {
    return;
  }
  SDL_FPoint slot_pos{8, 51};
  const auto slot_space_x = 4;
  const auto slot_space_y = 2;

  auto& mouse_pos = Window::mouse_pos_;
  auto lx = pos_.x + slot_pos.x;
  auto rx = lx + 36 * 5;

  auto ty = pos_.y + slot_pos.y;
  auto by = ty + 34 * 6;
  if ((lx <= mouse_pos.x && mouse_pos.x <= rx) &&
      (ty <= mouse_pos.y && mouse_pos.y <= by)) {
    auto dx = int((mouse_pos.x - lx) / 36);
    auto dy = int((mouse_pos.y - ty) / 34);
    auto index = dy * 5 + dx;
    index += page_ * 5;
    mouse_index_ = index;
  }
  return;
}

void PackageUiSystem::RenderBackgrnd() {
  static auto texture =
      WzResource::LoadTexture(WzResource::ui_->find(u"Item.img/backgrnd"));
  SDL_FRect pos_rect{
      pos_.x,
      pos_.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

void PackageUiSystem::RenderNumber(uint32_t num, int x, int y) {
  auto node = WzResource::ui_->find(u"Basic.img/ItemNo");
  // 计算数字位数
  int digits = num == 0 ? 1 : static_cast<int>(std::log10(num)) + 1;
  // 从最高位开始遍历
  int w = 0;
  for (int i = digits - 1; i >= 0; --i) {
    int divisor = static_cast<int>(std::pow(10, i));
    int digit = (num / divisor) % 10;
    auto t = WzResource::LoadTexture(node->get_child(std::to_string(digit)));
    SDL_FRect pos_rect = {
        (float)x + w,
        (float)y,
        (float)t->w,
        (float)t->h,
    };
    SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
    w += t->w;
  }
}

void PackageUiSystem::RenderTab() {
  if (new_itm_.has_value()) {
    // render new tab
    if (active_tab_ != (int)new_itm_->type) {
      auto now = Window::dt_now_;
      auto new_item_node = WzResource::ui_->find(u"Item.img/New/Tab0");
      auto sum = new_item_node->children_count() * 100;
      auto offset = now % sum;  // 取余，得到周期内偏移
      auto i = offset / 100;
      new_item_node = new_item_node->get_child(std::to_string(i));
      auto icon = WzResource::LoadTexture(new_item_node);
      auto origin = WzResource::LoadFpoint(new_item_node->get_child(u"origin"));
      auto tab_x = static_cast<float>(5 + (int)new_itm_->type * 33);
      SDL_FRect pos_rect{
          tab_x + pos_.x - origin.x,
          24 + pos_.y - origin.y,
          static_cast<float>(icon->w),
          static_cast<float>(icon->h),
      };
      SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
    }
  }
}

bool PackageUiSystem::RenderItemsInfo() {
  if (CursorGameInstance::cursor_ui_ != Render) {
    return true;
  }
  if (CursorGameInstance::modal_overlay_) {
    return true;
  }
  constexpr int tooltip_offset = 15;
  auto index_opt = mouse_index_;
  if (!index_opt.has_value()) {
    return true;
  }
  const size_t index = static_cast<size_t>(index_opt.value());
  auto& items = PackageGameInstance::data_[active_tab_];

  if (index >= items.size() || items[index]->id.empty()) {
    return true;
  }
  const SDL_FPoint show_pos{
      Window::mouse_pos_.x + tooltip_offset,
      Window::mouse_pos_.y + tooltip_offset,
  };

  // 统一使用指针，通过重载或模板区分类型
  if (active_tab_ == (int)ItemEnum::kEquip) {
    auto& equip = static_cast<GameEquipItem&>(*items[index]);
    TooltipUiSystem::RenderEquip(equip, show_pos.x, show_pos.y);
  } else if (active_tab_ == (int)ItemEnum::kDeco) {
    auto& deco = static_cast<GameDecoItem&>(*items[index]);
    TooltipUiSystem::RenderDeco(deco, show_pos.x, show_pos.y);
  } else {
    TooltipUiSystem::RenderItem(*items[index], show_pos.x, show_pos.y);
  }
  return true;
}

void PackageUiSystem::RenderItems() {
  constexpr SDL_FPoint slot_pos{8, 51};
  constexpr int slot_space_x = 4;
  constexpr int slot_space_y = 2;
  constexpr int slot_size = 32;
  constexpr int slots_per_row = 5;
  constexpr int max_rows = 6;
  constexpr int items_per_page = slots_per_row * max_rows;

  // 获取数据
  auto& items = PackageGameInstance::data_[active_tab_];

  const size_t start_index = static_cast<size_t>(page_) * slots_per_row;
  const size_t end_index = std::min(start_index + items_per_page, items.size());

  for (size_t i = start_index; i < start_index + 30; ++i) {
    // 计算行列
    const size_t local_index = i - start_index;
    const int row = local_index / slots_per_row;
    const int col = local_index % slots_per_row;
    if (i >= items.size()) {
      auto x = pos_.x + slot_pos.x + col * (slot_size + slot_space_x);
      auto y = pos_.y + slot_pos.y + row * (slot_size + slot_space_y);

      auto icon = WzResource::LoadTexture(
          WzResource::ui_->find(u"Item.img/canvas:disabled"));
      SDL_FRect pos_rect{
          std::floor(x),
          std::floor(y),
          static_cast<float>(icon->w),
          static_cast<float>(icon->h),
      };
      SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
      continue;
    }
    auto& item = items[i];
    if (item->id.empty())
      continue;

    wz::Node* info;
    // 加载信息
    if (active_tab_ == (int)ItemEnum::kEquip ||
        active_tab_ == (int)ItemEnum::kDeco) {
      info = EquipGameInstance::LoadEquipInfo(item->id);
    } else {
      info = ItemGameInstance::LoadItemInfo(item->id, 0);
    }

    auto icon = WzResource::LoadTexture(info->get_child(u"icon"));
    if (!icon)
      continue;

    // 计算位置
    const float x = pos_.x + slot_pos.x + col * (slot_size + slot_space_x) +
                    (slot_size - icon->w) * 0.5f;
    const float y = pos_.y + slot_pos.y + row * (slot_size + slot_space_y) +
                    (slot_size - icon->h) * 0.5f;

    SDL_FRect pos_rect{
        std::floor(x),
        std::floor(y),
        static_cast<float>(icon->w),
        static_cast<float>(icon->h),
    };

    SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);

    if (active_tab_ == (int)ItemEnum::kDeco) {
      icon = WzResource::LoadTexture(
          WzResource::ms_->get_root()->find(u"UI.img/CashItem"));
      pos_rect.x =
          (int)pos_.x + slot_pos.x + col * (slot_size + slot_space_x) + 19;
      pos_rect.y =
          (int)pos_.y + slot_pos.y + row * (slot_size + slot_space_y) + 19;
      pos_rect.w = icon->w;
      pos_rect.h = icon->h;
      SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
    }

    // render new item
    if (new_itm_.has_value()) {
      if (active_tab_ == (int)new_itm_->type && i == new_itm_->index) {
        auto now = Window::dt_now_;
        auto new_item_node = WzResource::ui_->find(u"Item.img/New/inventory");
        auto sum = new_item_node->children_count() * 100;
        auto offset = now % sum;  // 取余，得到周期内偏移
        auto i = offset / 100;
        new_item_node = new_item_node->get_child(std::to_string(i));
        icon = WzResource::LoadTexture(new_item_node);
        pos_rect.x =
            (int)pos_.x + slot_pos.x + col * (slot_size + slot_space_x) - 1;
        pos_rect.y =
            (int)pos_.y + slot_pos.y + row * (slot_size + slot_space_y) - 2;
        pos_rect.w = icon->w;
        pos_rect.h = icon->h;
        SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
      }
    }
    // render active ball
    if (active_tab_ == (int)ItemEnum::kConsume) {
      auto active_ball = PackageGameInstance::LoadActiveBall();
      if (active_ball == &item) {
        auto node = WzResource::ui_->find(u"Item.img/canvas:activeIcon");
        icon = WzResource::LoadTexture(node);
        pos_rect.x =
            (int)pos_.x + slot_pos.x + col * (slot_size + slot_space_x) - 1;
        pos_rect.y =
            (int)pos_.y + slot_pos.y + row * (slot_size + slot_space_y) - 2;
        pos_rect.w = icon->w;
        pos_rect.h = icon->h;
        SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
      }
    }

    // render num
    auto item_num = ItemGameInstance::LoadItemNum(item);
    if (item->type == ItemEnum::kConsume || item->type == ItemEnum::kEtc) {
      PackageUiSystem::RenderNumber(
          item_num, pos_.x + slot_pos.x + col * (slot_size + slot_space_x) + 1,
          pos_.y + slot_pos.y + row * (slot_size + slot_space_y) + 20);
    }
  }
}


UIComponents& PackageUiSystem::GetComponent() {
  const static std::array<Button::Callback, 4> kCallbacks = {
      EventClose,
      EventButtonCoin,
      EventButtonGather,
      EventButtonSort,
  };

  components_.clear();
  const static std::array buttons_nodes = {
      WzResource::ui_->find(u"Basic.img/BtClose"),
      WzResource::ui_->find(u"Item.img/AutoBuild/button:Coin"),
      WzResource::ui_->find(u"Item.img/AutoBuild/button:Gather"),
      WzResource::ui_->find(u"Item.img/AutoBuild/button:Sort"),
  };
  auto wh = LoadWh();
  const std::array buttons_rect = {
      SDL_FRect{wh.x - 18, 6, 12, 12},  //
      SDL_FRect{8, 268, 14, 14},        //
      SDL_FRect{164, 6, 12, 12},        //
      SDL_FRect{177, 6, 12, 12},        //
  };

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
    if (i < kCallbacks.size()) {
      std::get<Button>(components_.back()).callback = kCallbacks[i];
    }
  }

  const static auto tab_node = WzResource::ui_->find(u"Item.img/Tab");
  std::vector<SDL_FRect> tab_rects;
  for (uint8_t i = 0; i < 6; i++) {
    tab_rects.push_back(
        SDL_FRect{static_cast<float>(int(pos_.x + 5 + i * 33)),
                  static_cast<float>(int(pos_.y + 24)), 33, 19});
  }
  components_.push_back(
      Tab{tab_node, u"enabled", u"disabled", tab_rects, active_tab_});

  const SDL_FPoint vscr_lt{188, 51};
  const uint32_t vscr_length = 202;
  const auto& r = PackageGameInstance::data_[active_tab_];
  auto size = static_cast<size_t>(std::ceil(r.size() / 5.0));
  auto cursor_in = CursorGameInstance::cursor_ui_;
  bool top = cursor_in == Render && !CursorGameInstance::modal_overlay_;
  Scroll scroll(page_, static_cast<uint32_t>(size), vscr_length, 6);
  scroll.pos = {static_cast<float>((int)pos_.x + vscr_lt.x),
                static_cast<float>((int)pos_.y + vscr_lt.y)};
  scroll.top = top;
  components_.push_back(scroll);
  return components_;
}


void PackageUiSystem::RenderMeso() {
  Freetype::LoadSize(12);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(0, 0, 0, 255);
  auto meso = TextGameInstance::FormatWithCommas(PackageGameInstance::meso_);
  std::u16string meso2 = {meso.begin(), meso.end()};
  auto w = Freetype::LoadW(meso2);
  Freetype::DrawLine(meso2, pos_.x + 165 - w, pos_.y + 266);
}

bool PackageUiSystem::Render() {
  CalMouseIndex();
  RenderBackgrnd();
  RenderUI();
  RenderTab();
  RenderItems();
  RenderMeso();
  return true;
}

SDL_FPoint PackageUiSystem::LoadWh() {
  return {209, 289};
}

void PackageUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    active_tab_ = 0;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);

    EventMotion(nullptr);
  }
}

void PackageUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::render_systems_, RenderItemsInfo);
  std::erase(System::event_systems_, Event);

  EventDragEnd();
  new_itm_ = std::nullopt;
}

void PackageUiSystem::EventTop() {
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

bool PackageUiSystem::EventClickItem(SDL_Event* event) {
  // 网络手占用时拒绝操作
  if (CursorGameInstance::cursor_hand_net_.has_value()) {
    return false;
  }

  auto index = mouse_index_;
  if (!index.has_value()) {
    return false;
  }

  // 新获取的物品
  if (new_itm_.has_value()) {
    if (active_tab_ == (int)new_itm_->type &&
        new_itm_->index == index.value()) {
      new_itm_ = std::nullopt;
    }
  }

  // 无手持物品：拾取
  if (!CursorGameInstance::cursor_hand_.has_value()) {
    const auto& r = PackageGameInstance::data_[active_tab_];
    if (index.value() >= r.size() || r[index.value()]->id.empty()) {
      return false;
    }
    CursorGameInstance::cursor_hand_ = {
        .type = CursorGameInstance::CursorHandType::kPackage,
        .val = active_tab_,
        .sub_val = index.value(),
    };
    AudioGameInstance::LoadAudio(u"UI.img/DragStart", 0);
    return true;
  }

  // 有手持物品
  auto& sf = CharacterGameInstance::self_;
  auto& hand = CursorGameInstance::cursor_hand_.value();
  auto& r = PackageGameInstance::data_[hand.val];
  if (index.value() >= r.size()) {
    AudioGameInstance::LoadAudio(u"UI.img/DragEnd", 0);
    CursorGameInstance::cursor_hand_ = std::nullopt;
    return false;
  }
  switch ((CursorGameInstance::CursorHandType)hand.type) {
    case CursorGameInstance::CursorHandType::kEquipment: {
      if (active_tab_ != (int)ItemEnum::kEquip) {
        break;
      }
      auto& r = PackageGameInstance::data_[(int)ItemEnum::kEquip];
      if (r[index.value()]->id.empty()) {
        auto eqp = EquipGameInstance::LoadEquip(hand.sub_val);
        auto eqp2 = std::polymorphic<GameItem>(eqp->value());
        r[index.value()] = eqp2;
        *eqp = std::nullopt;
        AudioGameInstance::LoadAudio(u"UI.img/DragEnd", 0);
        CursorGameInstance::cursor_hand_ = std::nullopt;
        CharacterLogicSystem::cct_.map_id = SceneSystemInstance::map_id_;
      }
      break;
    }
    case CursorGameInstance::CursorHandType::kDeco: {
      if (active_tab_ != (int)ItemEnum::kDeco) {
        break;
      }
      auto& r = PackageGameInstance::data_[(int)ItemEnum::kDeco];
      if (r[index.value()]->id.empty()) {
        auto eqp = EquipGameInstance::LoadDeco(hand.sub_val);
        auto eqp2 = std::polymorphic<GameItem>(eqp->value());
        r[index.value()] = eqp2;
        *eqp = std::nullopt;
        AudioGameInstance::LoadAudio(u"UI.img/DragEnd", 0);
        CursorGameInstance::cursor_hand_ = std::nullopt;
        CharacterLogicSystem::cct_.map_id = SceneSystemInstance::map_id_;
      }
      break;
    }
    case CursorGameInstance::CursorHandType::kPackage: {
      if (hand.val != active_tab_) {
        // 只有卷轴类道具可以跨tab使用
        auto& itm = r.at(hand.sub_val);
        if (!((ItemEnum)active_tab_ == ItemEnum::kEquip &&
              hand.val == (int)ItemEnum::kConsume &&
              itm->id.starts_with(u"0204"))) {
          CursorGameInstance::cursor_hand_ = std::nullopt;
          return true;
        }
      }
      switch ((ItemEnum)hand.val) {
        case ItemEnum::kEquip: {
          // 点击的是同一个格子：尝试穿戴
          if (hand.val == active_tab_) {
            AudioGameInstance::LoadAudio(u"UI.img/DragEnd", 0);
            if (hand.sub_val == index.value()) {
              EquipGameInstance::UseEquip(index.value());
              CursorGameInstance::cursor_hand_ = std::nullopt;
              return true;
            } else {
              auto& equips = r;
              // 点击不同格子：交换
              std::swap(equips[hand.sub_val], equips[index.value()]);
              CursorGameInstance::cursor_hand_ = std::nullopt;
              return true;
            }
          } else {
          }
          break;
        }
        case ItemEnum::kDeco: {
          AudioGameInstance::LoadAudio(u"UI.img/DragEnd", 0);
          if (hand.sub_val == index.value()) {
            EquipGameInstance::UseDeco(index.value());
            CursorGameInstance::cursor_hand_ = std::nullopt;
            return true;
          } else {
            auto& decos = r;
            // 点击不同格子：交换
            std::swap(decos[hand.sub_val], decos[index.value()]);
            CursorGameInstance::cursor_hand_ = std::nullopt;
            return true;
          }
          break;
        }
        case ItemEnum::kEtc:
        case ItemEnum::kInstall:
        case ItemEnum::kConsume: {
          AudioGameInstance::LoadAudio(u"UI.img/DragEnd", 0);
          // 处理普通背包栏
          auto& r = PackageGameInstance::data_[hand.val];
          if ((int)ItemEnum::kEquip == active_tab_) {
            // 卷轴
            auto& equips = PackageGameInstance::data_[active_tab_];
            auto& equip = equips[index.value()];
            auto& itm = r.at(hand.sub_val);
            auto& eq = static_cast<GameEquipItem&>(*equip);
            auto& it = static_cast<GameConsumeItem&>(*itm);
            EquipGameInstance::UseEquipScroll(eq, it);
            CursorGameInstance::cursor_hand_ = std::nullopt;
            return true;
          }
          if (hand.sub_val != index.value()) {
            // 是否可堆叠，尝试合并
            auto& itm0 = r.at(index.value());
            auto& itm1 = r.at(hand.sub_val);
            if (itm0->id == itm1->id && !itm0->id.starts_with(u"0207")) {
              auto itm_num0 = ItemGameInstance::LoadItemNum(itm0);
              auto itm_num1 = ItemGameInstance::LoadItemNum(itm1);
              auto slot_max = ItemGameInstance::LoadSlotMax(itm0->id);
              auto add_num = std::min(slot_max - itm_num0, itm_num1);
              ItemGameInstance::AddItemNum(itm0, add_num);
              ItemGameInstance::DecItemNum(itm1, add_num);
            } else {
              std::swap(r.at(hand.sub_val), r.at(index.value()));
            }
          } else {
            // 使用道具
            auto& itm = r.at(index.value());
            ItemGameInstance::UseItem(itm);
          }
          CursorGameInstance::cursor_hand_ = std::nullopt;
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

void PackageUiSystem::EventDragStart(SDL_Event* event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos_.x, pos_.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - event->button.x, pos_.y - event->button.y};
  }
  return;
}

void PackageUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
  return;
}

void PackageUiSystem::EventDragMove(SDL_Event* event) {
  if (drag_.has_value()) {
    pos_ = {event->motion.x + drag_->x, event->motion.y + drag_->y};
    auto& camera = CameraGameInstance::camera_;
    auto [w, h] = LoadWh();
    pos_.x = std::clamp(pos_.x, (float)0, camera.w - w);
    pos_.y = std::clamp(pos_.y, (float)0, camera.h - h);
  }
  return;
}

void PackageUiSystem::Toggle() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

bool PackageUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void PackageUiSystem::EventClose() {
  Close();
}

void PackageUiSystem::EventButtonCoin() {
  NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kThrowMul;
  NoticeUiSystem::data_ = nullptr;
  // notice_ui_system::close();
  NoticeUiSystem::Open();
}

void PackageUiSystem::EventButtonGather() {}

void PackageUiSystem::EventButtonSort() {
  const auto cmp = [](const std::polymorphic<GameItem>& a,
                      const std::polymorphic<GameItem>& b) {
    bool a_empty = a->id.empty();
    bool b_empty = b->id.empty();
    if (a_empty != b_empty)
      return b_empty;
    return a->id < b->id;
  };
  std::ranges::sort(PackageGameInstance::data_[active_tab_], cmp);
}

bool PackageUiSystem::EventUI(SDL_Event* event) {
  EventClickItem(event);
  auto& components = GetComponent();
  for (auto& component : components) {
    if (auto* tab = std::get_if<Tab>(&component)) {
      if (auto i = tab->Click()) {
        AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
        if (active_tab_ != *i) {
          if (new_itm_.has_value() && (int)new_itm_->type == active_tab_) {
            new_itm_ = std::nullopt;
          }
          page_ = 0;
          active_tab_ = *i;
          // components 是按旧 tab 的 page 构建的；这里直接结束本次点击，
          // 避免下面的 Scroll::Click() 把 page_ 写回旧页。
          return false;
        }
      }
    }
  }
  if (DispatchButtonClick(components, false)) {
    return true;
  }
  for (auto& component : components) {
    if (auto* scroll = std::get_if<Scroll>(&component)) {
      page_ = scroll->Click();
    }
  }
  return false;
}

void PackageUiSystem::EventMotion(SDL_Event* event) {
  auto& sys = System::render_systems_;
  std::erase(sys, RenderItemsInfo);
  auto it = std::ranges::find(sys, &CursorRenderSystem::Render);
  if (it != sys.end()) {
    sys.insert(it, RenderItemsInfo);
  }
}

void PackageUiSystem::EventWheel(SDL_Event* event) {
  if (CursorGameInstance::cursor_ui_ != Render) {
    return;
  }
  auto dy = event->wheel.integer_y;
  if (dy > 0) {
    // up
    if (page_ > 0) {
      page_ -= 1;
    }
  } else {
    // down
    auto& items = PackageGameInstance::data_[active_tab_];
    auto size = static_cast<int>(std::ceil((items.size() - 30) / 5.0));
    size = std::max(0, size);
    if (page_ < size) {
      page_++;
    }
  }
}

void PackageUiSystem::EventMotionBtAudio(SDL_Event* event) {
  static int8_t bt_audio = -1;
  int8_t bt_in = static_cast<int8_t>(ButtonHoverIndex(GetComponent()));
  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

bool PackageUiSystem::Event(SDL_Event* event) {
  static int vscroll_drag = -1;
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      auto scan_code = event->key.scancode;
      switch (scan_code) {
        case SDL_SCANCODE_ESCAPE: {
          Toggle();
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
          DispatchScrollDragStart(GetComponent(), vscroll_drag,
                                  {event->button.x, event->button.y});
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = !EventUI(event);
        }
        EventDragEnd();
        vscroll_drag = -1;
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      if (vscroll_drag >= 0) {
        uint32_t val = page_;
        if (DispatchScrollDragMove(GetComponent(), vscroll_drag,
                                   {event->motion.x, event->motion.y}, val)) {
          page_ = static_cast<uint8_t>(val);
        }
      }
      EventMotion(event);
      EventDragMove(event);
      EventMotionBtAudio(event);
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

}  // namespace sdlms
