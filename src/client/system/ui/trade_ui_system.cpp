#include "src/client/system/ui/trade_ui_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/component/ui_component.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <ranges>
#include <string>
#include <vector>

namespace sdlms {

namespace {

// 交易窗口双方物品格（3x3，39x37），相对窗口左上角。
constexpr float kCellW = 39.0f;
constexpr float kCellH = 37.0f;
constexpr int kGridCols = 3;
constexpr int kGridRows = 3;
constexpr float kOtherGridX = 7.0f;
constexpr float kOtherGridY = 130.0f;
constexpr float kMyGridX = 170.0f;
constexpr float kMyGridY = 130.0f;

// 窗口底部：自己的背包条（点击放入/取出交易）。
constexpr float kStripX = 10.0f;
constexpr float kStripY = 285.0f;
constexpr float kStripCell = 26.0f;
constexpr int kStripCols = 20;
constexpr int kStripRows = 4;

std::u16string Id8(int v) {
  auto s = std::format("{:08d}", v);
  return {s.begin(), s.end()};
}

std::u16string ToU16(uint64_t v) {
  auto s = std::to_string(v);
  return {s.begin(), s.end()};
}

SDL_Texture* LoadItemIconById(const std::u16string& id) {
  auto type = ItemGameInstance::LoadItemType(id);
  if (type == u"Cash" || type == u"Consume" || type == u"Etc" ||
      type == u"Ins" || type == u"Pet") {
    auto* info = ItemGameInstance::LoadItemInfo(id, 0);
    if (info != nullptr) {
      for (const char16_t* key : {u"icon", u"iconRaw"}) {
        auto* c = info->get_child(key);
        if (c != nullptr && c->type == wz::Type::Canvas) {
          return WzResource::LoadTexture(c);
        }
      }
    }
  }
  auto* equip = EquipGameInstance::LoadEquipInfo(id);
  if (equip != nullptr) {
    auto* c = equip->get_child(u"icon");
    if (c != nullptr && c->type == wz::Type::Canvas) {
      return WzResource::LoadTexture(c);
    }
  }
  return nullptr;
}

// GameItem -> ItemUnionUnion（交易协议 payload）。
fbs::ItemUnionUnion ToUnion(std::polymorphic<GameItem>& item) {
  fbs::ItemUnionUnion u;
  if (item->id.empty()) {
    return u;
  }
  if (item->type == ItemEnum::kEquip) {
    fbs::EquipT et;
    et.equip_id = std::stoi(std::string{item->id.begin(), item->id.end()});
    auto& eq = static_cast<GameEquipItem&>(*item);
    for (auto& sc : eq.scroll) {
      fbs::EquipScrollT est;
      est.scroll_id = std::stoi(std::string{sc.id.begin(), sc.id.end()});
      est.success = sc.success;
      et.scroll.push_back(std::make_unique<fbs::EquipScrollT>(est));
    }
    u.Set(et);
  } else {
    fbs::ItemT it;
    it.item_id = std::stoi(std::string{item->id.begin(), item->id.end()});
    it.item_num = ItemGameInstance::LoadItemNum(item);
    u.Set(it);
  }
  return u;
}

// 包裹里所有非空物品（tab + 槽位）。
std::vector<TradeUiSystem::TradeOffer> InventoryList() {
  std::vector<TradeUiSystem::TradeOffer> list;
  for (uint8_t tab = 0; tab < PackageGameInstance::data_.size(); ++tab) {
    auto& vec = PackageGameInstance::data_[tab];
    for (uint32_t i = 0; i < vec.size(); ++i) {
      if (!vec[i]->id.empty()) {
        list.push_back({tab, i});
      }
    }
  }
  return list;
}

void SendOffer() {
  if (TradeUiSystem::other_id_ == 0) {
    return;
  }
  fbs::ClientCharacterTradeT ct;
  ct.to_id = TradeUiSystem::other_id_;
  ct.step = 10;  // 更新报价
  ct.meso = TradeUiSystem::my_meso_;
  for (auto& o : TradeUiSystem::my_offers_) {
    auto& vec = PackageGameInstance::data_[o.tab];
    if (o.index < vec.size() && !vec[o.index]->id.empty()) {
      ct.payload.push_back(ToUnion(vec[o.index]));
    }
  }
  ClientRequest::SendToHost(ct);
}

}  // namespace

SDL_FPoint TradeUiSystem::LoadWh() {
  return SDL_FPoint{567, 422};
}

bool TradeUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void TradeUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void TradeUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    CharacterLogicSystem::RunStandAction(other_);
    CharacterLogicSystem::RunFaceAction(other_, u"default");
    other_.flip = 0;

    my_offers_.clear();
    other_payload_.clear();
    my_meso_ = 0;
    other_meso_ = 0;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void TradeUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

void TradeUiSystem::RenderBackgrnd() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"Trade.img/TradingRoom/backgrnd"));
  SDL_FRect pos_rect{pos_.x, pos_.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);
}

void TradeUiSystem::RenderGrids() {
  // 自己放入的物品（右侧）
  for (int i = 0;
       i < static_cast<int>(my_offers_.size()) && i < kGridCols * kGridRows;
       ++i) {
    const auto& o = my_offers_[i];
    auto& vec = PackageGameInstance::data_[o.tab];
    if (o.index >= vec.size() || vec[o.index]->id.empty()) {
      continue;
    }
    auto* tex = LoadItemIconById(vec[o.index]->id);
    if (tex == nullptr) {
      continue;
    }
    float x = pos_.x + kMyGridX + (i % kGridCols) * kCellW;
    float y = pos_.y + kMyGridY + (i / kGridCols) * kCellH;
    SDL_FRect r{x + (kCellW - tex->w) / 2, y + (kCellH - tex->h) / 2,
                static_cast<float>(tex->w), static_cast<float>(tex->h)};
    SDL_RenderTexture(Window::renderer_, tex, nullptr, &r);
  }
  // 对方放入的物品（左侧）
  for (int i = 0;
       i < static_cast<int>(other_payload_.size()) && i < kGridCols * kGridRows;
       ++i) {
    const auto& u = other_payload_[i];
    std::u16string id;
    if (u.type == fbs::ItemUnion_Item) {
      auto* it = u.AsItem();
      if (it != nullptr) {
        id = Id8(it->item_id);
      }
    } else if (u.type == fbs::ItemUnion_Equip) {
      auto* et = u.AsEquip();
      if (et != nullptr) {
        id = Id8(et->equip_id);
      }
    }
    if (id.empty()) {
      continue;
    }
    auto* tex = LoadItemIconById(id);
    if (tex == nullptr) {
      continue;
    }
    float x = pos_.x + kOtherGridX + (i % kGridCols) * kCellW;
    float y = pos_.y + kOtherGridY + (i / kGridCols) * kCellH;
    SDL_FRect r{x + (kCellW - tex->w) / 2, y + (kCellH - tex->h) / 2,
                static_cast<float>(tex->w), static_cast<float>(tex->h)};
    SDL_RenderTexture(Window::renderer_, tex, nullptr, &r);
  }
}

void TradeUiSystem::RenderInventoryStrip() {
  auto list = InventoryList();
  for (int i = 0;
       i < static_cast<int>(list.size()) && i < kStripCols * kStripRows; ++i) {
    const auto& o = list[i];
    auto& vec = PackageGameInstance::data_[o.tab];
    auto* tex = LoadItemIconById(vec[o.index]->id);
    if (tex == nullptr) {
      continue;
    }
    float x = pos_.x + kStripX + (i % kStripCols) * kStripCell;
    float y = pos_.y + kStripY + (i / kStripCols) * kStripCell;
    float scale = 1.0f;
    float max_wh = static_cast<float>(std::max(tex->w, tex->h));
    if (max_wh > kStripCell - 4) {
      scale = (kStripCell - 4) / max_wh;
    }
    float w = static_cast<float>(tex->w) * scale;
    float h = static_cast<float>(tex->h) * scale;
    SDL_FRect r{x + (kStripCell - w) / 2, y + (kStripCell - h) / 2, w, h};
    SDL_RenderTexture(Window::renderer_, tex, nullptr, &r);
  }
}

void TradeUiSystem::OfferItem(uint8_t tab, uint32_t index) {
  auto it = std::ranges::find_if(my_offers_, [tab, index](const TradeOffer& o) {
    return o.tab == tab && o.index == index;
  });
  if (it != my_offers_.end()) {
    my_offers_.erase(it);
    SendOffer();
    return;
  }
  if (my_offers_.size() >= static_cast<size_t>(kGridCols * kGridRows)) {
    return;
  }
  my_offers_.push_back({tab, index});
  SendOffer();
}

void TradeUiSystem::ApplyTrade() {
  // 移除自己放出的物品
  for (auto& o : my_offers_) {
    if (o.tab >= PackageGameInstance::data_.size()) {
      continue;
    }
    auto& vec = PackageGameInstance::data_[o.tab];
    if (o.index < vec.size()) {
      vec[o.index]->id.clear();
    }
  }
  // 加入对方给出的物品
  for (auto& u : other_payload_) {
    if (u.type == fbs::ItemUnion_Equip) {
      auto* et = u.AsEquip();
      if (et == nullptr) {
        continue;
      }
      GameEquipItem e;
      e.id = Id8(et->equip_id);
      for (auto& sc : et->scroll) {
        if (sc != nullptr) {
          e.scroll.push_back({Id8(sc->scroll_id), sc->success});
        }
      }
      std::polymorphic<GameItem> item(std::in_place_type<GameEquipItem>, e);
      PackageGameInstance::AddItem(item);
    } else if (u.type == fbs::ItemUnion_Item) {
      auto* it = u.AsItem();
      if (it == nullptr) {
        continue;
      }
      auto item = ItemGameInstance::LoadItem(Id8(it->item_id), it->item_num);
      PackageGameInstance::AddItem(item);
    }
  }
  my_offers_.clear();
  other_payload_.clear();
  // 金币结算：扣除自己放出的，加入对方给出的。
  if (my_meso_ > 0 && PackageGameInstance::meso_ >= my_meso_) {
    PackageGameInstance::meso_ -= my_meso_;
  }
  PackageGameInstance::meso_ += other_meso_;
  my_meso_ = 0;
  other_meso_ = 0;
}

UIComponents& TradeUiSystem::GetComponent() {
  const static std::array<Button::Callback, 5> kCallbacks = {
      EventButtonNoop,   // claim
      EventButtonTrade,  // trade
      EventButtonNoop,   // enter
      EventButtonCoin,   // coin
      EventButtonReset,  // reset
  };

  components_.clear();
  const static std::array buttons_node = {
      WzResource::ui_->find(u"MapleChat.img/MapleChat/button:claim"),
      WzResource::ui_->find(u"Trade.img/TradingRoom/button:trade"),
      WzResource::ui_->find(u"MapleChat.img/MapleChat/button:enter"),
      WzResource::ui_->find(u"Trade.img/TradingRoom/button:coin"),
      WzResource::ui_->find(u"Trade.img/TradingRoom/button:reset"),
  };
  const std::array buttons_rect = {
      SDL_FRect{541, 250, 20, 19},  // claim
      SDL_FRect{116, 48, 60, 18},   // trade
      SDL_FRect{500, 251, 39, 17},  // enter
      SDL_FRect{151, 243, 36, 18},  // coin
      SDL_FRect{116, 70, 60, 18},   // reset
  };
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x += pos_.x;
    rect.y += pos_.y;
    Button button{.node = buttons_node[i],
                  .rect = rect,
                  .enable = true,
                  .focused = CursorIn()};
    components_.push_back(button);
    if (i < kCallbacks.size()) {
      std::get<Button>(components_.back()).callback = kCallbacks[i];
    }
  }
  return components_;
}

bool TradeUiSystem::Render() {
  RenderBackgrnd();
  Freetype::LoadAligned(true);
  Freetype::LoadSize(12);
  Freetype::LoadBold(false);
  Freetype::LoadColor(20, 20, 20, 255);
  Freetype::DrawLine(other_.name, pos_.x + 63, pos_.y + 106);
  Freetype::DrawLine(CharacterGameInstance::self_.name, pos_.x + 228,
                     pos_.y + 106);
  {
    auto t = ToU16(other_meso_);
    Freetype::DrawLine(t, pos_.x + 104 - Freetype::LoadW(t), pos_.y + 245);
  }
  {
    auto t = ToU16(my_meso_);
    Freetype::DrawLine(t, pos_.x + 283 - Freetype::LoadW(t), pos_.y + 245);
  }
  RenderGrids();
  RenderInventoryStrip();
  RenderUI();
  return true;
}

void TradeUiSystem::EventButtonTrade() {
  if (other_id_ == 0) {
    return;
  }
  fbs::ClientCharacterTradeT ct;
  ct.to_id = other_id_;
  ct.step = 2;  // 确认/锁定
  ClientRequest::SendToHost(ct);
}

void TradeUiSystem::EventButtonReset() {
  my_offers_.clear();
  my_meso_ = 0;
  SendOffer();
}

void TradeUiSystem::EventButtonCoin() {
  // 每次点击加 1000 金币，上限为自己持有的金币。
  my_meso_ = std::min<uint64_t>(my_meso_ + 1000, PackageGameInstance::meso_);
  SendOffer();
}

void TradeUiSystem::EventButtonNoop() {}

void TradeUiSystem::EventButtonClose() {
  Close();
}

bool TradeUiSystem::EventUI(SDL_Event* event) {
  return DispatchButtonClick(GetComponent(), true);
}

bool TradeUiSystem::Event(SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      if (event->key.scancode == SDL_SCANCODE_ESCAPE) {
        if (other_id_ != 0) {
          fbs::ClientCharacterTradeT ct;
          ct.to_id = other_id_;
          ct.step = 3;  // 取消交易
          ClientRequest::SendToHost(ct);
        }
        Close();
        return true;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT && CursorIn()) {
        return true;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT && CursorIn()) {
        // 先判背包条（放入/取出物品），再判按钮。
        auto list = InventoryList();
        auto& mouse = Window::mouse_pos_;
        for (int i = 0;
             i < static_cast<int>(list.size()) && i < kStripCols * kStripRows;
             ++i) {
          SDL_FRect r{pos_.x + kStripX + (i % kStripCols) * kStripCell,
                      pos_.y + kStripY + (i / kStripCols) * kStripCell,
                      kStripCell, kStripCell};
          if (SDL_PointInRectFloat(&mouse, &r)) {
            OfferItem(list[i].tab, list[i].index);
            return true;
          }
        }
        EventUI(event);
        return true;
      }
      break;
    }
    default: {
      break;
    }
  }
  return false;
}

}  // namespace sdlms