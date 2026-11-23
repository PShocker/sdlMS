#include "src/client/system/ui/storage_ui_system.h"

#include "src/client/game/game_storage.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/storage_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/npc_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/component/scroll.h"
#include "src/client/system/ui/component/tab.h"
#include "src/client/system/ui/component/ui_component.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system/ui/tooltip_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <wz/Property.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

namespace {

// 布局全部取自 Storage.img/Trunk 的 vector:* 与 slot* 字段。
constexpr SDL_FPoint kTrunkGridLt{9, 127};
constexpr SDL_FPoint kPkgGridLt{217, 127};
constexpr SDL_FPoint kTrunkScrollLt{191, 128};
constexpr SDL_FPoint kPkgScrollLt{398, 127};
constexpr SDL_FPoint kTabGetLt{6, 98};
constexpr SDL_FPoint kTabPutLt{214, 98};
constexpr SDL_FPoint kMesoTrunkLt{174, 343};
constexpr SDL_FPoint kMesoPkgLt{382, 343};
constexpr SDL_FPoint kNpcCb{42, 77};
constexpr SDL_FPoint kAvatarCb{258, 77};

constexpr int kSlotW = 36;
constexpr int kSlotH = 34;
constexpr int kSlotsPerRow = 5;
constexpr int kRowsPerPage = 6;
constexpr int kSlotsPerPage = kSlotsPerRow * kRowsPerPage;
constexpr int kScrollLength = 203;
constexpr int kTabW = 33;
constexpr int kTabH = 19;
constexpr int kTabCount = 4;

// 0 = 仓库格，1 = 背包格
constexpr int kActiveTrunk = 0;
constexpr int kActivePkg = 1;

}  // namespace

SDL_FPoint StorageUiSystem::LoadWh() {
  return {421, 366};
}

bool StorageUiSystem::CursorIn() {
  auto wh = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, wh.x, wh.y};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

std::optional<uint32_t> StorageUiSystem::LoadMouseIndex(SDL_FPoint lt,
                                                        uint16_t page,
                                                        size_t count) {
  auto& mouse_pos = Window::mouse_pos_;
  const float lx = pos_.x + lt.x;
  const float ty = pos_.y + lt.y;
  SDL_FRect grid{lx, ty, static_cast<float>(kSlotW * kSlotsPerRow),
                 static_cast<float>(kSlotH * kRowsPerPage)};
  if (!SDL_PointInRectFloat(&mouse_pos, &grid)) {
    return std::nullopt;
  }
  const auto col = static_cast<int>((mouse_pos.x - lx) / kSlotW);
  const auto row = static_cast<int>((mouse_pos.y - ty) / kSlotH);
  const auto index = static_cast<uint32_t>(page) * kSlotsPerRow +
                     static_cast<uint32_t>(row * kSlotsPerRow + col);
  if (index >= count) {
    return std::nullopt;
  }
  return index;
}

void StorageUiSystem::RenderBackgrnd() {
  static auto back_texture = WzResource::LoadTexture(
      WzResource::ui_->find(u"Storage.img/Trunk/backgrnd"));
  SDL_FRect pos_rect = {
      pos_.x,
      pos_.y,
      static_cast<float>(back_texture->w),
      static_cast<float>(back_texture->h),
  };
  SDL_RenderTexture(Window::renderer_, back_texture, nullptr, &pos_rect);
}

void StorageUiSystem::RenderTab() {
  // 仓库一侧只有一页，"get" 标签固定显示为选中态。
  static auto get_tab = WzResource::LoadTexture(
      WzResource::ui_->find(u"Storage.img/Trunk/tab:get/selected/0"));
  SDL_FRect pos_rect{
      static_cast<float>((int)pos_.x + (int)kTabGetLt.x),
      static_cast<float>((int)pos_.y + (int)kTabGetLt.y),
      static_cast<float>(get_tab->w),
      static_cast<float>(get_tab->h),
  };
  SDL_RenderTexture(Window::renderer_, get_tab, nullptr, &pos_rect);
}

void StorageUiSystem::RenderNpc() {
  if (!npc_.has_value()) {
    return;
  }
  auto g_npc = npc_.value();
  g_npc.action = u"stand";
  const auto& camera = CameraGameInstance::camera_;
  g_npc.pos.x = camera.x + pos_.x + kNpcCb.x;
  g_npc.pos.y = camera.y + pos_.y + kNpcCb.y;
  g_npc.ani_index = 0;
  NpcRenderSystem::RenderNpc(g_npc);
}

void StorageUiSystem::RenderAvatar() {
  auto self = CharacterGameInstance::self_;
  CharacterLogicSystem::RunStandAction(self);
  self.action_index = 0;
  self.flip = 0;
  self.face.action = u"default";
  self.face.index = 0;
  const auto& camera = CameraGameInstance::camera_;
  self.pos.x = camera.x + (int)pos_.x + (int)kAvatarCb.x;
  self.pos.y = camera.y + (int)pos_.y + (int)kAvatarCb.y;
  CharacterRenderSystem::RenderCharacter(self);
}

void StorageUiSystem::RenderItems(
    std::vector<std::polymorphic<GameItem>>& items, SDL_FPoint lt,
    uint16_t page, const std::optional<uint16_t>& active) {
  const size_t start = static_cast<size_t>(page) * kSlotsPerRow;
  for (size_t i = start; i < start + kSlotsPerPage; ++i) {
    if (i >= items.size()) {
      break;
    }
    auto& item = items[i];
    const size_t local = i - start;
    const int row = static_cast<int>(local) / kSlotsPerRow;
    const int col = static_cast<int>(local) % kSlotsPerRow;
    const float x = pos_.x + lt.x + col * kSlotW;
    const float y = pos_.y + lt.y + row * kSlotH;

    // 选中格高亮
    if (active.has_value() && active.value() == i) {
      SDL_FRect sel{x, y, kSlotW - 1.0f, kSlotH - 1.0f};
      SDL_SetRenderDrawBlendMode(Window::renderer_, SDL_BLENDMODE_BLEND);
      SDL_SetRenderDrawColor(Window::renderer_, 255, 255, 255, 80);
      SDL_RenderFillRect(Window::renderer_, &sel);
    }

    if (item->id.empty()) {
      continue;
    }

    wz::Node* info = nullptr;
    if (item->type == ItemEnum::kEquip || item->type == ItemEnum::kDeco) {
      info = EquipGameInstance::LoadEquipInfo(item->id);
    } else {
      info = ItemGameInstance::LoadItemInfo(item->id, 0);
    }
    if (info == nullptr || info->get_child(u"icon") == nullptr) {
      continue;
    }
    auto icon = WzResource::LoadTexture(info->get_child(u"icon"));
    if (icon == nullptr) {
      continue;
    }
    SDL_FRect pos_rect{
        std::floor(x + (32 - icon->w) / 2.0f),
        std::floor(y + (32 - icon->h) / 2.0f),
        static_cast<float>(icon->w),
        static_cast<float>(icon->h),
    };
    SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);

    // 可叠加道具显示数量
    if (item->type == ItemEnum::kConsume || item->type == ItemEnum::kEtc) {
      auto num = ItemGameInstance::LoadItemNum(item);
      if (num > 1) {
        PackageUiSystem::RenderNumber(num, (int)x + 1, (int)y + 20);
      }
    }
  }
}

void StorageUiSystem::RenderMeso() {
  Freetype::LoadSize(12);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(0, 0, 0, 255);
  auto trunk =
      TextGameInstance::FormatWithCommas(StorageGameInstance::storage_.meso);
  std::u16string trunk2{trunk.begin(), trunk.end()};
  Freetype::DrawLine(trunk2, pos_.x + kMesoTrunkLt.x - Freetype::LoadW(trunk2),
                     pos_.y + kMesoTrunkLt.y);

  auto mine = TextGameInstance::FormatWithCommas(PackageGameInstance::meso_);
  std::u16string mine2{mine.begin(), mine.end()};
  Freetype::DrawLine(mine2, pos_.x + kMesoPkgLt.x - Freetype::LoadW(mine2),
                     pos_.y + kMesoPkgLt.y);
}

void StorageUiSystem::RenderItemInfo() {
  if (CursorGameInstance::cursor_ui_ != Render ||
      CursorGameInstance::modal_overlay_) {
    return;
  }
  std::polymorphic<GameItem>* hovered = nullptr;
  if (auto index = LoadMouseIndex(kTrunkGridLt, page_[kActiveTrunk],
                                  StorageGameInstance::storage_.data.size());
      index.has_value()) {
    auto& itm = StorageGameInstance::storage_.data[index.value()];
    if (!itm->id.empty()) {
      hovered = &itm;
    }
  }
  if (hovered == nullptr) {
    auto& pkg = PackageGameInstance::data_[active_tab_];
    if (auto index = LoadMouseIndex(kPkgGridLt, page_[kActivePkg], pkg.size());
        index.has_value()) {
      auto& itm = pkg[index.value()];
      if (!itm->id.empty()) {
        hovered = &itm;
      }
    }
  }
  if (hovered == nullptr) {
    return;
  }
  constexpr int kTooltipOffset = 15;
  const SDL_FPoint show_pos{
      Window::mouse_pos_.x + kTooltipOffset,
      Window::mouse_pos_.y + kTooltipOffset,
  };
  switch ((*hovered)->type) {
    case ItemEnum::kEquip: {
      auto& equip = static_cast<GameEquipItem&>(**hovered);
      TooltipUiSystem::RenderEquip(equip, show_pos.x, show_pos.y);
      break;
    }
    case ItemEnum::kDeco: {
      auto& deco = static_cast<GameDecoItem&>(**hovered);
      TooltipUiSystem::RenderDeco(deco, show_pos.x, show_pos.y);
      break;
    }
    default: {
      TooltipUiSystem::RenderItem(**hovered, show_pos.x, show_pos.y);
      break;
    }
  }
}

UIComponents& StorageUiSystem::GetComponent() {
  const static std::array<Button::Callback, 4> kCallbacks = {
      EventButtonGet,
      EventButtonPut,
      EventButtonSort,
      EventButtonExit,
  };

  components_.clear();
  const static std::array buttons_node = {
      WzResource::ui_->find(u"Storage.img/Trunk/button:get"),
      WzResource::ui_->find(u"Storage.img/Trunk/button:put"),
      WzResource::ui_->find(u"Storage.img/Trunk/button:sort"),
      WzResource::ui_->find(u"Storage.img/Trunk/button:exit"),
  };
  const std::array buttons_rect = {
      SDL_FRect{132, 31, 70, 18},  // get
      SDL_FRect{340, 11, 70, 18},  // put
      SDL_FRect{132, 51, 70, 19},  // sort
      SDL_FRect{132, 11, 70, 18},  // exit
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x += (int)pos_.x;
    rect.y += (int)pos_.y;
    Button button{.node = buttons_node[i],
                  .rect = rect,
                  .enable = true,
                  .focused = CursorGameInstance::cursor_ui_ == Render &&
                             !CursorGameInstance::modal_overlay_};
    components_.push_back(button);
    if (i < kCallbacks.size()) {
      std::get<Button>(components_.back()).callback = kCallbacks[i];
    }
  }

  // 背包一侧可以翻页，四格分别对应装备／消耗／设置／其它四个背包页。
  std::vector<SDL_FRect> tab_rects;
  for (int i = 0; i < kTabCount; ++i) {
    tab_rects.push_back(SDL_FRect{
        static_cast<float>((int)(pos_.x + kTabPutLt.x) + i * kTabW + i * 12),
        static_cast<float>((int)(pos_.y + kTabPutLt.y)),
        static_cast<float>(kTabW),
        static_cast<float>(kTabH),
    });
  }
  components_.push_back(Tab{WzResource::ui_->find(u"Storage.img/Trunk/tab:put"),
                            u"selected", u"normal", tab_rects,
                            static_cast<uint8_t>(active_tab_)});

  const bool top = CursorGameInstance::cursor_ui_ == Render &&
                   !CursorGameInstance::modal_overlay_;
  auto& trunk = StorageGameInstance::storage_.data;
  auto trunk_rows =
      static_cast<uint32_t>(std::ceil(trunk.size() / (float)kSlotsPerRow));
  Scroll trunk_scroll(page_[kActiveTrunk], trunk_rows, kScrollLength,
                      kRowsPerPage);
  trunk_scroll.pos = {pos_.x + kTrunkScrollLt.x, pos_.y + kTrunkScrollLt.y};
  trunk_scroll.top = top;
  components_.push_back(trunk_scroll);

  auto& pkg = PackageGameInstance::data_[active_tab_];
  auto pkg_rows =
      static_cast<uint32_t>(std::ceil(pkg.size() / (float)kSlotsPerRow));
  Scroll pkg_scroll(page_[kActivePkg], pkg_rows, kScrollLength, kRowsPerPage);
  pkg_scroll.pos = {pos_.x + kPkgScrollLt.x, pos_.y + kPkgScrollLt.y};
  pkg_scroll.top = top;
  components_.push_back(pkg_scroll);

  return components_;
}

bool StorageUiSystem::Render() {
  RenderBackgrnd();
  RenderTab();
  RenderUI();
  RenderNpc();
  RenderAvatar();
  auto& trunk = StorageGameInstance::storage_.data;
  RenderItems(trunk, kTrunkGridLt, page_[kActiveTrunk],
              active_item_[kActiveTrunk]);
  auto& pkg = PackageGameInstance::data_[active_tab_];
  RenderItems(pkg, kPkgGridLt, page_[kActivePkg], active_item_[kActivePkg]);
  RenderMeso();
  RenderItemInfo();
  return true;
}

void StorageUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

void StorageUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    active_item_ = {};
    page_ = {};
    // 背包页只开放前四个（装备／消耗／设置／其它），其它页不能存入仓库
    if (active_tab_ < 0 || active_tab_ >= kTabCount) {
      active_tab_ = 0;
    }
    StorageGameInstance::EnsureSlots();

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void StorageUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  active_item_ = {};
}

void StorageUiSystem::EventClose() {
  Close();
}

void StorageUiSystem::ClearItem(std::polymorphic<GameItem>& item) {
  item->id = u"";
  item->expire = 0;
  if (item->type == ItemEnum::kConsume || item->type == ItemEnum::kEtc) {
    ItemGameInstance::SetItemNum(item, 0);
  }
}

bool StorageUiSystem::AddItem(std::polymorphic<GameItem>& item) {
  if (item->id.empty()) {
    return true;
  }
  auto& slots = StorageGameInstance::storage_.data;
  if (slots.empty()) {
    return false;
  }

  const bool stackable =
      item->type == ItemEnum::kConsume || item->type == ItemEnum::kEtc;
  if (stackable) {
    auto slot_max = ItemGameInstance::LoadSlotMax(item->id);
    int remain = ItemGameInstance::LoadItemNum(item);
    const int total = remain;
    if (remain <= 0) {
      return false;
    }
    // 先补满已有堆叠
    for (auto& slot : slots) {
      if (remain <= 0) {
        break;
      }
      if (slot->id != item->id || slot->type != item->type) {
        continue;
      }
      const int cur = ItemGameInstance::LoadItemNum(slot);
      if (cur >= slot_max) {
        continue;
      }
      const int add = std::min(remain, slot_max - cur);
      ItemGameInstance::AddItemNum(slot, add);
      remain -= add;
    }
    // 再占用空槽
    for (auto& slot : slots) {
      if (remain <= 0) {
        break;
      }
      if (!slot->id.empty()) {
        continue;
      }
      const int add = std::min(remain, slot_max);
      slot->id = item->id;
      slot->expire = item->expire;
      ItemGameInstance::SetItemNum(slot, add);
      remain -= add;
    }
    ItemGameInstance::DecItemNum(item, total - remain);
    if (item->id.empty() || ItemGameInstance::LoadItemNum(item) == 0) {
      ClearItem(item);
    }
    return remain == 0;
  }

  for (auto& slot : slots) {
    if (!slot->id.empty()) {
      continue;
    }
    slot = item;
    ClearItem(item);
    return true;
  }
  return false;
}

void StorageUiSystem::EventButtonGet() {
  auto& index = active_item_[kActiveTrunk];
  if (!index.has_value()) {
    return;
  }
  auto& slots = StorageGameInstance::storage_.data;
  if (index.value() >= slots.size()) {
    index = std::nullopt;
    return;
  }
  auto& itm = slots[index.value()];
  if (itm->id.empty()) {
    index = std::nullopt;
    return;
  }
  if (!PackageGameInstance::AddItem(itm).has_value()) {
    NoticeUiSystem::OpenNoSpace(itm->type);
    return;
  }
  if (itm->id.empty() || ItemGameInstance::LoadItemNum(itm) == 0) {
    ClearItem(itm);
    index = std::nullopt;
  }
}

void StorageUiSystem::EventButtonPut() {
  auto& index = active_item_[kActivePkg];
  if (!index.has_value()) {
    return;
  }
  auto& pkg = PackageGameInstance::data_[active_tab_];
  if (index.value() >= pkg.size()) {
    index = std::nullopt;
    return;
  }
  auto& itm = pkg[index.value()];
  if (itm->id.empty()) {
    index = std::nullopt;
    return;
  }
  if (!AddItem(itm)) {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kNoStorageSpace;
    NoticeUiSystem::Open();
    return;
  }
  if (itm->id.empty() || ItemGameInstance::LoadItemNum(itm) == 0) {
    ClearItem(itm);
    index = std::nullopt;
  }
}

void StorageUiSystem::EventButtonSort() {
  const auto cmp = [](const std::polymorphic<GameItem>& a,
                      const std::polymorphic<GameItem>& b) {
    bool a_empty = a->id.empty();
    bool b_empty = b->id.empty();
    if (a_empty != b_empty) {
      return b_empty;
    }
    return a->id < b->id;
  };
  std::ranges::sort(StorageGameInstance::storage_.data, cmp);
  active_item_[kActiveTrunk] = std::nullopt;
  page_[kActiveTrunk] = 0;
}

void StorageUiSystem::EventButtonExit() {
  EventClose();
}

bool StorageUiSystem::EventItem(SDL_Event* event) {
  (void)event;
  auto& trunk = StorageGameInstance::storage_.data;
  if (auto index =
          LoadMouseIndex(kTrunkGridLt, page_[kActiveTrunk], trunk.size());
      index.has_value()) {
    if (trunk[index.value()]->id.empty()) {
      active_item_[kActiveTrunk] = std::nullopt;
    } else if (active_item_[kActiveTrunk] == index.value()) {
      // 再次点击＝直接取出（与双击取出一致）
      EventButtonGet();
    } else {
      active_item_[kActiveTrunk] = static_cast<uint16_t>(index.value());
    }
    active_item_[kActivePkg] = std::nullopt;
    return true;
  }

  auto& pkg = PackageGameInstance::data_[active_tab_];
  if (auto index = LoadMouseIndex(kPkgGridLt, page_[kActivePkg], pkg.size());
      index.has_value()) {
    if (pkg[index.value()]->id.empty()) {
      active_item_[kActivePkg] = std::nullopt;
    } else if (active_item_[kActivePkg] == index.value()) {
      // 再次点击＝直接存入
      EventButtonPut();
    } else {
      active_item_[kActivePkg] = static_cast<uint16_t>(index.value());
    }
    active_item_[kActiveTrunk] = std::nullopt;
    return true;
  }
  return false;
}

bool StorageUiSystem::EventUI(SDL_Event* event) {
  if (EventItem(event)) {
    return true;
  }

  auto& components = GetComponent();
  int group = 0;
  for (auto& component : components) {
    auto* tab = std::get_if<Tab>(&component);
    if (tab == nullptr) {
      continue;
    }
    if (group == 0) {
      // 第一个 Tab 是背包页签
      if (auto i = tab->Click(); i.has_value()) {
        active_tab_ = *i;
        active_item_[kActivePkg] = std::nullopt;
        page_[kActivePkg] = 0;
        return true;
      }
    }
    group++;
  }

  if (DispatchButtonClick(components, true)) {
    return true;
  }

  group = 0;
  for (auto& component : components) {
    auto* scroll = std::get_if<Scroll>(&component);
    if (scroll == nullptr) {
      continue;
    }
    auto val = scroll->Click();
    if (group == 0) {
      auto& trunk = StorageGameInstance::storage_.data;
      const auto rows =
          static_cast<uint32_t>(std::ceil(trunk.size() / (float)kSlotsPerRow));
      const auto max_page = rows > kRowsPerPage ? rows - kRowsPerPage : 0;
      page_[kActiveTrunk] = static_cast<uint16_t>(std::min(val, max_page));
    } else {
      auto& pkg = PackageGameInstance::data_[active_tab_];
      const auto rows =
          static_cast<uint32_t>(std::ceil(pkg.size() / (float)kSlotsPerRow));
      const auto max_page = rows > kRowsPerPage ? rows - kRowsPerPage : 0;
      page_[kActivePkg] = static_cast<uint16_t>(std::min(val, max_page));
    }
    group++;
  }
  return false;
}

bool StorageUiSystem::Event(SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      if (event->key.scancode == SDL_SCANCODE_ESCAPE) {
        EventClose();
        return false;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT &&
          CursorGameInstance::cursor_ui_ == Render) {
        return false;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT &&
          CursorGameInstance::cursor_ui_ == Render) {
        EventUI(event);
        return false;
      }
      break;
    }
    default: {
      break;
    }
  }
  return true;
}

}  // namespace sdlms
