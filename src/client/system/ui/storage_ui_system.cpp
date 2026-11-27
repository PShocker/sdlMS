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
#include "src/client/system/input/keyboard_input_system.h"
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

// 改动后把当前页夹回有效范围，避免删空一页后停在空白页。
void ClampPage(const std::vector<std::polymorphic<GameItem>>& items,
               uint16_t& page) {
  const auto rows =
      static_cast<uint32_t>(std::ceil(items.size() / (float)kSlotsPerRow));
  const auto max_page = rows > kRowsPerPage ? rows - kRowsPerPage : 0u;
  if (page > max_page) {
    page = static_cast<uint16_t>(max_page);
  }
}

// 某一侧的行数（仓库 0 / 背包 1）。
uint32_t SideRows(int side, int tab) {
  const auto count =
      side == 0 ? StorageGameInstance::storage_.data.size()
                : PackageGameInstance::data_[tab].size();
  return static_cast<uint32_t>(std::ceil(count / (float)kSlotsPerRow));
}

uint32_t SideMaxPage(int side, int tab) {
  const uint32_t rows = SideRows(side, tab);
  return rows > kRowsPerPage ? rows - kRowsPerPage : 0;
}

// 输入数量超过可存取上限时的提示（复用 Notice 的通用文本弹窗）。
void NoticeStorageExceed() {
  NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kShopNotice;
  NoticeUiSystem::notice_text_ = u"输入的数量超过最大值！";
  NoticeUiSystem::Open();
}

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
  static auto get_node = WzResource::ui_->find(u"Storage.img/Trunk/tab:get");
  static auto get_tab =
      WzResource::LoadTexture(WzResource::ui_->find(
          u"Storage.img/Trunk/tab:get/selected/0"));
  SDL_FPoint get_lt{6.0f, 98.0f};
  if (get_node != nullptr) {
    if (auto* c = get_node->get_child(u"lt");
        c != nullptr && c->type == wz::Type::Vector2D) {
      auto v = static_cast<wz::Property<wz::WzVec2D>*>(c)->get();
      get_lt = {static_cast<float>(v.x), static_cast<float>(v.y)};
    }
  }
  SDL_FRect pos_rect{
      static_cast<float>((int)pos_.x + (int)get_lt.x),
      static_cast<float>((int)pos_.y + (int)get_lt.y),
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
        std::floor(x + (kSlotW - icon->w) / 2.0f),
        std::floor(y + (kSlotH - icon->h) / 2.0f),
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
  // 金币输入框在 backgrnd 里的范围是 y=343..356（高 14），数字要垂直居中：
  // DrawLine 的 y 是行框顶部，字形大约从 y+5 开始，所以这里减去半行高再微调。
  constexpr float kMesoBoxTop = 343.0f;
  constexpr float kMesoBoxH = 14.0f;
  const float line_h = Freetype::LoadLh();
  const float text_y =
      pos_.y + kMesoBoxTop + (kMesoBoxH - line_h) / 2.0f - 1.0f;

  auto trunk =
      TextGameInstance::FormatWithCommas(StorageGameInstance::storage_.meso);
  std::u16string trunk2{trunk.begin(), trunk.end()};
  Freetype::DrawLine(trunk2, pos_.x + kMesoTrunkLt.x - Freetype::LoadW(trunk2),
                     text_y);

  auto mine = TextGameInstance::FormatWithCommas(PackageGameInstance::meso_);
  std::u16string mine2{mine.begin(), mine.end()};
  Freetype::DrawLine(mine2, pos_.x + kMesoPkgLt.x - Freetype::LoadW(mine2),
                     text_y);
}

void StorageUiSystem::RenderItemInfo() {
  // 仓库自己是模态窗口（modal_overlay_ == &Render），此时仍然要显示物品
  // tooltip；只有被更上层的模态（比如数量询问框）盖住时才隐藏。
  if (CursorGameInstance::cursor_ui_ != Render ||
      (CursorGameInstance::modal_overlay_ != nullptr &&
       CursorGameInstance::modal_overlay_ != &Render)) {
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
  const static std::array<Button::Callback, 6> kCallbacks = {
      EventButtonGet,      EventButtonPut,     EventButtonSort,
      EventButtonExit,     EventButtonOutCoin, EventButtonInCoin,
  };

  components_.clear();
  const static std::array buttons_node = {
      WzResource::ui_->find(u"Storage.img/Trunk/button:get"),
      WzResource::ui_->find(u"Storage.img/Trunk/button:put"),
      WzResource::ui_->find(u"Storage.img/Trunk/button:sort"),
      WzResource::ui_->find(u"Storage.img/Trunk/button:exit"),
      WzResource::ui_->find(u"Storage.img/Trunk/button:outCoin"),
      WzResource::ui_->find(u"Storage.img/Trunk/button:InCoin"),
  };
  // 位置取 Storage.img/Trunk/button:<name>/normal/0 的 origin 取反：
  // get(132,31) put(340,11) sort(132,51) exit(132,11)
  // outCoin(8,341) InCoin(216,341)
  const std::array buttons_rect = {
      SDL_FRect{132, 31, 70, 18},   // get
      SDL_FRect{340, 11, 70, 18},   // put
      SDL_FRect{132, 51, 70, 19},   // sort
      SDL_FRect{132, 11, 70, 18},   // exit
      SDL_FRect{8, 341, 36, 18},    // outCoin
      SDL_FRect{216, 341, 36, 18},  // InCoin
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x += (int)pos_.x;
    rect.y += (int)pos_.y;
    Button button{.node = buttons_node[i],
                  .rect = rect,
                  .enable = true,
                  .focused = CursorGameInstance::cursor_ui_ == Render &&
                             (CursorGameInstance::modal_overlay_ == nullptr ||
                              CursorGameInstance::modal_overlay_ == &Render)};
    components_.push_back(button);
    if (i < kCallbacks.size()) {
      std::get<Button>(components_.back()).callback = kCallbacks[i];
    }
  }

  // 背包一侧的四个页签：位置/尺寸全部取自 Storage.img/Trunk/tab:put
  // （lt、各页签画布宽高、tabSpace），不再用写死的间距。
  std::vector<SDL_FRect> tab_rects;
  {
    auto lv = [](wz::Node* n, const char16_t* name) {
      SDL_FPoint p{0.0f, 0.0f};
      auto* c = n == nullptr ? nullptr : n->get_child(name);
      if (c != nullptr && c->type == wz::Type::Vector2D) {
        auto v = static_cast<wz::Property<wz::WzVec2D>*>(c)->get();
        p = {static_cast<float>(v.x), static_cast<float>(v.y)};
      }
      return p;
    };
    auto* put = WzResource::ui_->find(u"Storage.img/Trunk/tab:put");
    const SDL_FPoint lt = lv(put, u"lt");
    int tab_space = 0;
    if (auto* c = put == nullptr ? nullptr : put->get_child(u"tabSpace")) {
      if (c->type == wz::Type::Int) {
        tab_space = static_cast<wz::Property<int32_t>*>(c)->get();
      }
    }
    float tab_w = static_cast<float>(kTabW);
    float tab_h = static_cast<float>(kTabH);
    if (auto* c = put == nullptr ? nullptr
                                 : put->get_child(u"selected/0")) {
      if (auto* tex = WzResource::LoadTexture(c)) {
        tab_h = static_cast<float>(tex->h);
      }
    }
    float x = static_cast<float>((int)(pos_.x + lt.x));
    const float y = static_cast<float>((int)(pos_.y + lt.y));
    for (int i = 0; i < kTabCount; ++i) {
      float w = tab_w;
      std::u16string frame = u"normal/";
      frame += static_cast<char16_t>(u'0' + i);
      if (auto* c = put == nullptr ? nullptr : put->get_child(frame)) {
        if (auto* tex = WzResource::LoadTexture(c)) {
          w = static_cast<float>(tex->w);
        }
      }
      tab_rects.push_back(SDL_FRect{x, y, w, tab_h});
      x += w + static_cast<float>(tab_space);
    }
  }
  components_.push_back(Tab{WzResource::ui_->find(u"Storage.img/Trunk/tab:put"),
                            u"selected", u"normal", tab_rects,
                            static_cast<uint8_t>(active_tab_)});

  // 仓库窗口是模态的：modal_overlay_ 指向自己时仍然可以操作自己的组件。
  const bool self_modal = CursorGameInstance::modal_overlay_ == &Render;
  const bool top = CursorGameInstance::cursor_ui_ == Render &&
                   (CursorGameInstance::modal_overlay_ == nullptr || self_modal);
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
    StorageGameInstance::NormalizeTypes();
    // 打开时清掉按键状态：否则走路中打开仓库，之前按下的方向键不会收到
    // key-up（仓库把键盘事件都吞掉了），人物会一直朝那个方向走。
    KeyboardInputSystem::Reset();

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void StorageUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  active_item_ = {};
  drag_scroll_ = -1;
  // 关闭时同样清空按键，避免模态期间残留的方向键让角色继续移动。
  KeyboardInputSystem::Reset();
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

bool StorageUiSystem::AddItem(std::polymorphic<GameItem>& item,
                             uint64_t limit) {
  if (item->id.empty() || limit == 0) {
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
    const int available = ItemGameInstance::LoadItemNum(item);
    // 只存入 limit 件（堆叠道具可以只存一部分）。
    int remain = static_cast<int>(
        std::min<uint64_t>(static_cast<uint64_t>(std::max(available, 0)),
                           limit));
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
      // 整份复制（连同 type）：空槽是 GameEtcItem 占位，只写 id/num 会让
      // 存进去的可叠加道具保留 Etc 类型，取回时跑到 Etc 页签。
      slot = item;
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

// 从仓库第 slot_index 格取回 limit 件：先并进背包同 id 未满堆叠，再放空槽，
// 然后按实际取出的数量扣减/清空仓库那一格。
void StorageUiSystem::TakeOutItem(int slot_index, uint64_t limit) {
  StorageGameInstance::NormalizeTypes();
  auto& slots = StorageGameInstance::storage_.data;
  if (slot_index < 0 || slot_index >= static_cast<int>(slots.size())) {
    active_item_[kActiveTrunk] = std::nullopt;
    return;
  }
  auto& itm = slots[slot_index];
  if (itm->id.empty() || limit == 0) {
    active_item_[kActiveTrunk] = std::nullopt;
    return;
  }

  const ItemEnum item_type = itm->type;
  const int bucket = static_cast<int>(item_type);
  if (bucket < 0 ||
      bucket >= static_cast<int>(PackageGameInstance::data_.size())) {
    NoticeUiSystem::OpenNoSpace(item_type);
    return;
  }
  const bool stackable =
      item_type == ItemEnum::kConsume || item_type == ItemEnum::kEtc;
  const int available =
      stackable ? std::max(ItemGameInstance::LoadItemNum(itm), 0) : 1;
  if (limit != UINT64_MAX &&
      limit > static_cast<uint64_t>(available) && available > 0) {
    NoticeStorageExceed();
    return;
  }
  int remain = static_cast<int>(std::min<uint64_t>(
      static_cast<uint64_t>(available),
      limit == UINT64_MAX ? static_cast<uint64_t>(available) : limit));
  if (remain <= 0) {
    return;
  }
  const int want = remain;
  auto& pkg = PackageGameInstance::data_[bucket];
  const int slot_max = std::max(ItemGameInstance::LoadSlotMax(itm->id), 1);

  if (stackable) {
    // 1) 并入背包里已有的同 id 堆叠
    for (int i = 0; i < static_cast<int>(pkg.size()) && remain > 0; ++i) {
      if (pkg[i]->id != itm->id) {
        continue;
      }
      const int cur = static_cast<int>(ItemGameInstance::LoadItemNum(pkg[i]));
      if (cur >= slot_max) {
        continue;
      }
      const int add = std::min(remain, slot_max - cur);
      ItemGameInstance::AddItemNum(pkg[i], add);
      remain -= add;
    }
    // 2) 剩余放进空槽
    for (int i = 0; i < static_cast<int>(pkg.size()) && remain > 0; ++i) {
      if (!pkg[i]->id.empty()) {
        continue;
      }
      const int add = std::min(remain, slot_max);
      pkg[i] = itm;  // 整份复制（保留 type/expire）
      ItemGameInstance::SetItemNum(pkg[i], add);
      remain -= add;
    }
  } else {
    // 装备一件：找一个空槽整体放入
    for (int i = 0; i < static_cast<int>(pkg.size()); ++i) {
      if (!pkg[i]->id.empty()) {
        continue;
      }
      pkg[i] = itm;
      remain = 0;
      break;
    }
  }

  const int moved = want - remain;
  if (moved <= 0) {
    NoticeUiSystem::OpenNoSpace(item_type);
    return;
  }
  // 3) 扣减仓库源：拿光了就清空格子，取回后格子应该消失。
  if (stackable) {
    ItemGameInstance::DecItemNum(itm, moved);
  }
  if (!stackable || itm->id.empty() || ItemGameInstance::LoadItemNum(itm) == 0) {
    ClearItem(itm);
    active_item_[kActiveTrunk] = std::nullopt;
  }
  ClampPage(slots, page_[kActiveTrunk]);

  // 4) 右侧背包切到该道具对应的页签并回到第 1 页，方便立刻看到。
  const int tab = std::clamp(static_cast<int>(item_type), 0, kTabCount - 1);
  if (tab != active_tab_) {
    active_tab_ = tab;
    page_[kActivePkg] = 0;
    active_item_[kActivePkg] = std::nullopt;
  }
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
  StorageGameInstance::NormalizeTypes();
  auto& itm = slots[index.value()];
  if (itm->id.empty()) {
    index = std::nullopt;
    return;
  }

  const ItemEnum item_type = itm->type;
  const bool stackable =
      item_type == ItemEnum::kConsume || item_type == ItemEnum::kEtc;
  // 飞镖（0207xxxx）整组取回，不询问数量。
  const bool whole_stack = itm->id.starts_with(u"0207");
  const int num = std::max(ItemGameInstance::LoadItemNum(itm), 0);
  if (stackable && num > 1 && !whole_stack) {
    // 可叠加道具：先问要取回多少件（和存入同样的数量框）。
    const int idx = static_cast<int>(index.value());
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kStorageCount;
    NoticeUiSystem::notice_text_ = u"请输入要取出的数量：";
    NoticeUiSystem::number_value_ = static_cast<uint64_t>(num);
    NoticeUiSystem::on_confirm_ = [idx]() {
      TakeOutItem(idx, NoticeUiSystem::LoadInputNumber());
    };
    NoticeUiSystem::Open();
    return;
  }
  TakeOutItem(static_cast<int>(index.value()),
              whole_stack ? UINT64_MAX : 1);
}

void StorageUiSystem::PutItemCount(int tab, int index, uint64_t limit) {
  if (tab < 0 || tab >= static_cast<int>(PackageGameInstance::data_.size())) {
    return;
  }
  auto& pkg = PackageGameInstance::data_[tab];
  if (index < 0 || index >= static_cast<int>(pkg.size())) {
    return;
  }
  auto& itm = pkg[index];
  if (itm->id.empty()) {
    return;
  }
  const bool stackable =
      itm->type == ItemEnum::kConsume || itm->type == ItemEnum::kEtc;
  const uint64_t max_count =
      stackable ? static_cast<uint64_t>(
                      std::max(ItemGameInstance::LoadItemNum(itm), 0))
                : 1u;
  // 超过可存入的最大数量：给出提示，不静默截断。
  if (limit != UINT64_MAX && limit > max_count) {
    NoticeStorageExceed();
    return;
  }
  const uint64_t want =
      std::clamp<uint64_t>(limit == UINT64_MAX ? max_count : limit, 1u,
                           std::max<uint64_t>(max_count, 1u));
  if (!AddItem(itm, want)) {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kNoStorageSpace;
    NoticeUiSystem::Open();
    return;
  }
  if (itm->id.empty() || ItemGameInstance::LoadItemNum(itm) == 0) {
    ClearItem(itm);
    active_item_[kActivePkg] = std::nullopt;
  }
  ClampPage(pkg, page_[kActivePkg]);
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

  const bool stackable =
      itm->type == ItemEnum::kConsume || itm->type == ItemEnum::kEtc;
  // 飞镖（0207xxxx）按一整组存取，不询问数量。
  const bool whole_stack = itm->id.starts_with(u"0207");
  const int num = ItemGameInstance::LoadItemNum(itm);
  if (stackable && num > 1 && !whole_stack) {
    // 可堆叠道具：先问要存多少件。
    const int tab = active_tab_;
    const int idx = static_cast<int>(index.value());
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kStorageCount;
    NoticeUiSystem::notice_text_ = u"请输入要存入的数量：";
    NoticeUiSystem::number_value_ = static_cast<uint64_t>(num);
    NoticeUiSystem::on_confirm_ = [tab, idx]() {
      PutItemCount(tab, idx, NoticeUiSystem::LoadInputNumber());
    };
    NoticeUiSystem::Open();
    return;
  }
  PutItemCount(active_tab_, static_cast<int>(index.value()),
               whole_stack ? UINT64_MAX : 1);
}

void StorageUiSystem::EventButtonInCoin() {
  // 存入金币：弹数量框，默认 1000，上限为身上持有的金币。
  if (PackageGameInstance::meso_ == 0) {
    return;
  }
  NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kStorageCount;
  NoticeUiSystem::notice_text_ = u"请输入要存入的金币数量：";
  NoticeUiSystem::number_value_ =
      std::min<uint64_t>(1000, PackageGameInstance::meso_);
  NoticeUiSystem::on_confirm_ = []() {
    const uint64_t input = NoticeUiSystem::LoadInputNumber();
    if (input > PackageGameInstance::meso_) {
      NoticeStorageExceed();
      return;
    }
    if (input == 0) {
      return;
    }
    PackageGameInstance::meso_ -= input;
    StorageGameInstance::storage_.meso += input;
  };
  NoticeUiSystem::Open();
}

void StorageUiSystem::EventButtonOutCoin() {
  // 取出金币：弹数量框，默认 1000，上限为仓库里的金币。
  auto& storage = StorageGameInstance::storage_;
  if (storage.meso == 0) {
    return;
  }
  NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kStorageCount;
  NoticeUiSystem::notice_text_ = u"请输入要取出的金币数量：";
  NoticeUiSystem::number_value_ = std::min<uint64_t>(1000, storage.meso);
  NoticeUiSystem::on_confirm_ = []() {
    auto& storage = StorageGameInstance::storage_;
    const uint64_t input = NoticeUiSystem::LoadInputNumber();
    if (input > storage.meso) {
      NoticeStorageExceed();
      return;
    }
    if (input == 0) {
      return;
    }
    storage.meso -= input;
    PackageGameInstance::meso_ += input;
  };
  NoticeUiSystem::Open();
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

// 鼠标所在的滚动条：0 = 仓库侧，1 = 背包侧，-1 = 都不在。
int StorageUiSystem::ScrollSideAt(SDL_FPoint mouse) {
  auto in_bar = [&](SDL_FPoint lt) {
    SDL_FRect bar{pos_.x + lt.x, pos_.y + lt.y, 15.0f,
                  static_cast<float>(kScrollLength)};
    return SDL_PointInRectFloat(&mouse, &bar);
  };
  if (in_bar(kTrunkScrollLt)) {
    return 0;
  }
  if (in_bar(kPkgScrollLt)) {
    return 1;
  }
  return -1;
}

// 鼠标所在的滚动区域（格子或滚动条）：0 = 仓库侧，1 = 背包侧，-1 = 都不在。
int StorageUiSystem::WheelSideAt(SDL_FPoint mouse) {
  auto in_grid = [&](SDL_FPoint lt) {
    SDL_FRect r{pos_.x + lt.x, pos_.y + lt.y,
                static_cast<float>(kSlotW * kSlotsPerRow),
                static_cast<float>(kSlotH * kRowsPerPage)};
    return SDL_PointInRectFloat(&mouse, &r);
  };
  if (in_grid(kTrunkGridLt)) {
    return 0;
  }
  if (in_grid(kPkgGridLt)) {
    return 1;
  }
  return ScrollSideAt(mouse);
}

void StorageUiSystem::ApplyScrollDrag(int side) {
  if (side < 0) {
    return;
  }
  const bool is_trunk = side == 0;
  const auto rows = SideRows(side, active_tab_);
  Scroll scroll(is_trunk ? page_[kActiveTrunk] : page_[kActivePkg], rows,
                kScrollLength, kRowsPerPage);
  scroll.pos = {pos_.x + (is_trunk ? kTrunkScrollLt.x : kPkgScrollLt.x),
                pos_.y + (is_trunk ? kTrunkScrollLt.y : kPkgScrollLt.y)};
  scroll.top = CursorGameInstance::cursor_ui_ == Render &&
               (CursorGameInstance::modal_overlay_ == nullptr ||
                CursorGameInstance::modal_overlay_ == &Render);
  const auto page = scroll.Drag(Window::mouse_pos_);
  const auto max_page = SideMaxPage(side, active_tab_);
  auto& target = is_trunk ? page_[kActiveTrunk] : page_[kActivePkg];
  target = static_cast<uint16_t>(std::min(page, max_page));
}

bool StorageUiSystem::Event(SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
      // 模态窗口：键盘事件不再传给下层的键盘输入系统（会驱动人物移动）。
      if (event->type == SDL_EVENT_KEY_DOWN &&
          event->key.scancode == SDL_SCANCODE_ESCAPE) {
        EventClose();
      }
      return false;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT &&
          CursorGameInstance::cursor_ui_ == Render) {
        // 按住滚动条轨道开始拖动（点在上下箭头上仍然走点击单步）
        const int side = ScrollSideAt(Window::mouse_pos_);
        if (side >= 0) {
          const bool is_trunk = side == 0;
          SDL_FRect track{
              pos_.x + (is_trunk ? kTrunkScrollLt.x : kPkgScrollLt.x),
              pos_.y + (is_trunk ? kTrunkScrollLt.y : kPkgScrollLt.y) + 13.0f,
              15.0f,
              static_cast<float>(kScrollLength - 26),
          };
          if (SDL_PointInRectFloat(&Window::mouse_pos_, &track)) {
            drag_scroll_ = side;
            ApplyScrollDrag(side);
          }
        }
        return false;
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      if (drag_scroll_ >= 0) {
        ApplyScrollDrag(drag_scroll_);
        return false;
      }
      break;
    }
    case SDL_EVENT_MOUSE_WHEEL: {
      // 仓库自身是模态（modal_overlay_ == &Render）时滚轮仍要生效，
      // 只有被更上层的模态（数量询问框等）盖住时才忽略。
      if (CursorGameInstance::cursor_ui_ != Render ||
          (CursorGameInstance::modal_overlay_ != nullptr &&
           CursorGameInstance::modal_overlay_ != &Render)) {
        break;
      }
      // 滚轮按鼠标所在区域翻页（格子或滚动条都算）
      const int side = WheelSideAt(Window::mouse_pos_);
      if (side < 0 || event->wheel.y == 0.0f) {
        break;
      }
      const int step = event->wheel.y > 0.0f ? -1 : 1;
      auto& target = side == 0 ? page_[kActiveTrunk] : page_[kActivePkg];
      const int max_page = static_cast<int>(SideMaxPage(side, active_tab_));
      target = static_cast<uint16_t>(
          std::clamp(static_cast<int>(target) + step, 0, max_page));
      return false;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        const bool was_dragging = drag_scroll_ >= 0;
        drag_scroll_ = -1;
        if (!was_dragging && CursorGameInstance::cursor_ui_ == Render) {
          EventUI(event);
          return false;
        }
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
