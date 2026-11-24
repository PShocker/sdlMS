#include "src/client/system/ui/cash_shop_ui_system.h"

#include "src/client/game/game_character.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/cursor_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/component/scroll.h"
#include "src/client/system/ui/tooltip_ui_system.h"
#include "src/client/system_instance/fade_system_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/freetype/freetype.h"
#include "src/common/physic/physic.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_heartbeat_instance.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>
#include <wz/Property.h>
#include <wz/Types.h>
#include <wz/Wz.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace sdlms {

namespace {

// 现金商店（CashShop）界面。
// 所有面板/按钮/文字位置都取自 UI/CashShopUI.img 各节点的 origin / lt / rb，
// 坐标系是 1366x768 的整张底图，绘制时统一加 g_cash_shop_pos。
//
// 主要结构：
//   - 布局常量：试穿窗、梯子、两个背包面板、分类 tab、商品网格、页码。
//   - 左侧：Preview（试穿小人，可走动 / 爬梯子）、Cash Inventory、Item
//   Inventory。
//   - 右侧：分类 tab、分类条、搜索框、3x5 商品卡片、页码翻页。
//   - 悬浮：商品卡片 / 背包格子显示 TooltipUiSystem 的道具说明。

SDL_FPoint g_cash_shop_pos = {0, 0};

bool g_cash_shop_visible = false;
bool g_cash_shop_fading = false;

SDL_FPoint g_return_pos = {0, 0};

bool g_on_ladder = false;

// 试穿角色的运动状态（和地图一样由 Physic 驱动）。
float g_hspeed = 0.0f;
float g_vspeed = 0.0f;
float g_hforce = 0.0f;
int32_t g_fh = 0;
uint64_t g_ladder_cooldown = 0;
uint8_t g_phys_page = 0;


constexpr float kSceneX = 189.0f;
constexpr float kSceneY = 121.0f;
constexpr float kSceneW = 243.0f;
constexpr float kSceneH = 171.0f;

constexpr float kStandFloor = 252.0f;
constexpr float kStandLeft = 236.0f;
constexpr float kStandRight = 400.0f;

constexpr float kLadderX = 374.0f;
constexpr float kLadderTop = 200.0f;
constexpr float kLadderBottom = kStandFloor;
// 和地图 RunClimb 一致：|pos.x - 梯子x| <= 15 才算在梯子范围内。
constexpr float kLadderGrab = 15.0f;

constexpr float kJobBoxL = 185.0f;
constexpr float kJobBoxR = 303.0f;
constexpr float kNameBoxL = 305.0f;
constexpr float kNameBoxR = 433.0f;
constexpr float kNameBoxT = 87.0f;
constexpr float kNameBoxB = 108.0f;

constexpr float kSlotW = 45.0f;
constexpr int kItemInvenCols = 5;
constexpr int kItemInvenRows = 3;
constexpr int kLockerCols = 5;
constexpr int kLockerRows = 2;

constexpr float kLockerScrollX = 418.0f;
constexpr float kLockerScrollY = 461.0f;
constexpr int kLockerScrollLength = 72;
constexpr float kItemInvenScrollX = 417.0f;
constexpr float kItemInvenScrollY = 639.0f;
constexpr int kItemInvenScrollLength = 112;
constexpr int kInvenSlotCount = 96;

constexpr float kTabX = 466.0f;
constexpr float kTabY = 86.0f;
constexpr float kTabRight = 1174.0f;
constexpr int kTabCount = 8;
constexpr float kTabStride = (kTabRight - kTabX) / kTabCount;

constexpr float kGridX = 463.0f;
constexpr float kGridY = 181.0f;
constexpr float kCardW = 235.0f;
constexpr float kCardH = 87.0f;
constexpr int kGridCols = 3;
constexpr int kGridRows = 5;
constexpr float kGridGapX = 5.0f;
constexpr float kGridGapY = 5.25f;
constexpr int kPageSize = kGridCols * kGridRows;

constexpr int64_t kNxBalance = 0;
constexpr int64_t kLeafPointBalance = 0;

const SDL_Color kTextDark = {32, 32, 32, 255};
const SDL_Color kTextWhite = {255, 255, 255, 255};

constexpr uint8_t kFontSize = 12;

// 试穿场景的假 foothold：一条从左到右的平地，让 Physic 能沿用地图的走/跳逻辑。
const std::flat_map<int32_t, GameFoothold>& PreviewFootholds() {
  static std::flat_map<int32_t, GameFoothold> data;
  if (data.empty()) {
    GameFoothold fh{};
    fh.id = 1;
    fh.page = 0;
    fh.zmass = 0;
    fh.prev = 0;
    fh.next = 0;
    fh.x1 = static_cast<int32_t>(kSceneX) - 600;
    fh.y1 = static_cast<int32_t>(kStandFloor);
    fh.x2 = static_cast<int32_t>(kSceneX + kSceneW) + 600;
    fh.y2 = static_cast<int32_t>(kStandFloor);
    fh.l = fh.x1;
    fh.r = fh.x2;
    fh.t = fh.y1;
    fh.b = fh.y1;
    fh.k = 0.0f;
    fh.intercept = kStandFloor;
    data.emplace(1, fh);
  }
  return data;
}

// 物理边界：整块屏幕（Physic 用它夹住角色，防止飞出去）。
// 试穿窗口的可视范围另外用 clamp 限制。
std::optional<SDL_FRect> PreviewBorder() {
  return SDL_FRect{0.0f, 0.0f, 1366.0f, 768.0f};
}

// 试穿：进商城时记下角色身上所有点装槽位，退出商城时还原。
struct DecoSnapshot {
  std::optional<GameDecoItem> weapon;
  std::optional<GameDecoItem> cap;
  std::optional<GameDecoItem> cape;
  std::optional<GameDecoItem> coat;
  std::optional<GameDecoItem> glove;
  std::optional<GameDecoItem> pant;
  std::optional<GameDecoItem> ring0;
  std::optional<GameDecoItem> ring1;
  std::optional<GameDecoItem> shield;
  std::optional<GameDecoItem> longcoat;
  std::optional<GameDecoItem> shoes;
  std::optional<GameDecoItem> accessory;
};

DecoSnapshot g_deco_snapshot;

void SaveDecoSnapshot() {
  const auto& s = CharacterGameInstance::self_;
  g_deco_snapshot = {s.weapon_deco,  s.cap_deco,      s.cape_deco,
                     s.coat_deco,    s.glove_deco,    s.pant_deco,
                     s.ring0_deco,   s.ring1_deco,    s.shield_deco,
                     s.longcoat_deco, s.shoes_deco,   s.accessory_deco};
}

void RestoreDecoSnapshot() {
  auto& s = CharacterGameInstance::self_;
  s.weapon_deco = g_deco_snapshot.weapon;
  s.cap_deco = g_deco_snapshot.cap;
  s.cape_deco = g_deco_snapshot.cape;
  s.coat_deco = g_deco_snapshot.coat;
  s.glove_deco = g_deco_snapshot.glove;
  s.pant_deco = g_deco_snapshot.pant;
  s.ring0_deco = g_deco_snapshot.ring0;
  s.ring1_deco = g_deco_snapshot.ring1;
  s.shield_deco = g_deco_snapshot.shield;
  s.longcoat_deco = g_deco_snapshot.longcoat;
  s.shoes_deco = g_deco_snapshot.shoes;
  s.accessory_deco = g_deco_snapshot.accessory;
}

// 试穿一件点装：按 Character.wz 的分类找到对应槽位套到小人身上；
// 同一件再点一次则还原成进入商城时的样子。
void TryOnItem(const std::u16string& id) {
  const auto type = EquipGameInstance::LoadEquipType(id);
  if (type.empty() || WzResource::character_ == nullptr) {
    return;
  }
  // 商城里有些商品在 Character.wz 里没有对应资源，直接跳过，
  // 否则 AddAvatarCache 会对空节点解引用导致崩溃。
  auto* node = WzResource::character_->find(type + u"/" + id + u".img");
  if (node == nullptr || node->find(u"info/islot") == nullptr ||
      node->find(u"info/vslot") == nullptr) {
    return;
  }
  // 帽子类饰品（vslot=Cp）等要套到对应槽位才有 z 序，按 vslot 再细分一次。
  auto vslot = std::u16string{};
  if (auto* v = node->find(u"info/vslot")) {
    vslot = static_cast<wz::Property<std::u16string>*>(v)->get();
  }
  auto vslot_has = [&](const char16_t* code) {
    return vslot.find(code) != std::u16string::npos;
  };
  auto& s = CharacterGameInstance::self_;
  auto toggle = [&](std::optional<GameDecoItem>& slot,
                    const std::optional<GameDecoItem>& origin,
                    void (*add)(GameCharacter&, const std::u16string&)) {
    if (slot.has_value() && slot->id == id) {
      slot = origin;
    } else {
      slot = std::nullopt;
      add(s, id);
    }
  };
  if (type == u"Cap") {
    toggle(s.cap_deco, g_deco_snapshot.cap, &CharacterGameInstance::AddCapDeco);
  } else if (type == u"Coat") {
    toggle(s.coat_deco, g_deco_snapshot.coat,
           &CharacterGameInstance::AddCoatDeco);
  } else if (type == u"Longcoat") {
    toggle(s.longcoat_deco, g_deco_snapshot.longcoat,
           &CharacterGameInstance::AddLongcoatDeco);
  } else if (type == u"Pants") {
    toggle(s.pant_deco, g_deco_snapshot.pant,
           &CharacterGameInstance::AddPantsDeco);
  } else if (type == u"Shoes") {
    toggle(s.shoes_deco, g_deco_snapshot.shoes,
           &CharacterGameInstance::AddShoesDeco);
  } else if (type == u"Glove") {
    toggle(s.glove_deco, g_deco_snapshot.glove,
           &CharacterGameInstance::AddGloveDeco);
  } else if (type == u"Cape") {
    toggle(s.cape_deco, g_deco_snapshot.cape,
           &CharacterGameInstance::AddCapeDeco);
  } else if (type == u"Weapon") {
    toggle(s.weapon_deco, g_deco_snapshot.weapon,
           &CharacterGameInstance::AddWeaponDeco);
  } else if (type == u"Shield") {
    toggle(s.shield_deco, g_deco_snapshot.shield,
           &CharacterGameInstance::AddShieldDeco);
  } else if (type == u"Ring") {
    toggle(s.ring0_deco, g_deco_snapshot.ring0,
           &CharacterGameInstance::AddRing0Deco);
  } else if (type == u"Accessory") {
    // 饰品按 vslot 落到对应的身体槽位：帽子(Cp)、披风(Sr)、鞋(So)……
    if (vslot_has(u"Cp")) {
      toggle(s.cap_deco, g_deco_snapshot.cap,
             &CharacterGameInstance::AddCapDeco);
    } else if (vslot_has(u"Sr")) {
      toggle(s.cape_deco, g_deco_snapshot.cape,
             &CharacterGameInstance::AddCapeDeco);
    } else if (vslot_has(u"Ma")) {
      toggle(s.coat_deco, g_deco_snapshot.coat,
             &CharacterGameInstance::AddCoatDeco);
    } else if (vslot_has(u"So")) {
      toggle(s.shoes_deco, g_deco_snapshot.shoes,
             &CharacterGameInstance::AddShoesDeco);
    } else if (vslot_has(u"Gl") || vslot_has(u"Gv")) {
      toggle(s.glove_deco, g_deco_snapshot.glove,
             &CharacterGameInstance::AddGloveDeco);
    } else if (vslot_has(u"Pn")) {
      toggle(s.pant_deco, g_deco_snapshot.pant,
             &CharacterGameInstance::AddPantsDeco);
    } else if (vslot_has(u"Wp")) {
      toggle(s.weapon_deco, g_deco_snapshot.weapon,
             &CharacterGameInstance::AddWeaponDeco);
    }
    // 其余饰品（脸/耳/戒指类）渲染层没有对应部件，试穿时直接忽略。
  }
}

SDL_FPoint FindNearestSp(SDL_FPoint from) {
  SDL_FPoint best = from;
  float best_dist = -1.0f;
  for (auto it = PortalGameInstance::data_.begin();
       it != PortalGameInstance::data_.end(); ++it) {
    if (it->second.pn != u"sp") {
      continue;
    }
    float dx = it->second.pos.x - from.x;
    float dy = it->second.pos.y - from.y;
    float d = dx * dx + dy * dy;
    if (best_dist < 0.0f || d < best_dist) {
      best_dist = d;
      best = it->second.pos;
    }
  }
  if (best_dist >= 0.0f) {
    best.y -= 5;
  }
  return best;
}

wz::Node* FindBackgrnd() {
  return WzResource::ui_->find(u"CashShopUI.img/backgrnd");
}

SDL_FPoint LoadWh() {
  static const auto backgrnd = WzResource::LoadTexture(FindBackgrnd());
  return SDL_FPoint{static_cast<float>(backgrnd->w),
                    static_cast<float>(backgrnd->h)};
}

SDL_FPoint LoadOrigin(wz::Node* node) {
  if (node == nullptr) {
    return {0, 0};
  }
  auto* origin = node->get_child(u"origin");
  if (origin != nullptr && origin->type == wz::Type::Vector2D) {
    auto v = static_cast<wz::Property<wz::WzVec2D>*>(origin)->get();
    return {static_cast<float>(-v.x), static_cast<float>(-v.y)};
  }
  return {0, 0};
}

wz::Node* FindUi(const std::u16string& path) {
  return WzResource::ui_->find(u"CashShopUI.img/" + path);
}

void DrawUi(const std::u16string& path, float dx = 0.0f, float dy = 0.0f) {
  auto* node = FindUi(path);
  if (node == nullptr) {
    return;
  }
  auto* texture = WzResource::LoadTexture(node);
  if (texture == nullptr) {
    return;
  }
  auto pos = LoadOrigin(node);
  SDL_FRect dst{g_cash_shop_pos.x + pos.x + dx, g_cash_shop_pos.y + pos.y + dy,
                static_cast<float>(texture->w), static_cast<float>(texture->h)};
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &dst);
}

void RenderPanel(const std::u16string& path) {
  auto* node = FindUi(path);
  if (node == nullptr) {
    return;
  }
  auto* texture = WzResource::LoadTexture(node);
  auto panel_pos = LoadOrigin(node);
  SDL_FRect pos_rect = {
      g_cash_shop_pos.x + panel_pos.x,
      g_cash_shop_pos.y + panel_pos.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

bool MouseIn(float x, float y, float w, float h) {
  SDL_FRect r{x, y, w, h};
  SDL_FPoint mouse{Window::mouse_pos_.x, Window::mouse_pos_.y};
  return SDL_PointInRectFloat(&mouse, &r);
}

void DrawTextureAt(SDL_Texture* texture, float x, float y) {
  if (texture == nullptr) {
    return;
  }
  SDL_FRect dst{x, y, static_cast<float>(texture->w),
                static_cast<float>(texture->h)};
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &dst);
}

void DrawTextureAt(const std::u16string& path, float x, float y) {
  auto* node = FindUi(path);
  if (node == nullptr) {
    return;
  }
  DrawTextureAt(WzResource::LoadTexture(node), x, y);
}

void RenderButton(const std::u16string& base) {
  auto* normal = FindUi(base + u"/normal/0");
  if (normal == nullptr) {
    return;
  }
  auto npos = LoadOrigin(normal);
  auto* ntex = WzResource::LoadTexture(normal);
  if (ntex == nullptr) {
    return;
  }
  bool hover =
      MouseIn(g_cash_shop_pos.x + npos.x, g_cash_shop_pos.y + npos.y,
              static_cast<float>(ntex->w), static_cast<float>(ntex->h));
  auto* node = hover ? FindUi(base + u"/mouseOver/0") : normal;
  if (node == nullptr) {
    node = normal;
  }
  auto* t = WzResource::LoadTexture(node);
  if (t == nullptr) {
    t = ntex;
  }
  auto pos = LoadOrigin(node);
  SDL_FRect pos_rect{g_cash_shop_pos.x + pos.x, g_cash_shop_pos.y + pos.y,
                     static_cast<float>(t->w), static_cast<float>(t->h)};
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
}


std::u16string ToU16(const std::string& s) {
  return std::u16string(s.begin(), s.end());
}

std::u16string Money(int64_t value) {
  std::string s = std::to_string(value);
  std::string out;
  int n = 0;
  for (int i = static_cast<int>(s.size()) - 1; i >= 0; --i) {
    out.push_back(s[i]);
    ++n;
    if (n % 3 == 0 && i > 0) {
      out.push_back(',');
    }
  }
  std::reverse(out.begin(), out.end());
  return ToU16(out);
}

std::u16string Num(int64_t value) {
  return ToU16(std::to_string(value));
}

void LoadFont(uint8_t size, SDL_Color color) {
  Freetype::LoadSize(size);
  Freetype::LoadColor(color.r, color.g, color.b, color.a);
  Freetype::LoadAligned(true);
}

void TextCenterAt(const std::u16string& text, float cx, float cy, uint8_t size,
                  SDL_Color color) {
  if (text.empty()) {
    return;
  }
  LoadFont(size, color);
  float w = Freetype::LoadW(text);
  float lh = Freetype::LoadLh();
  Freetype::DrawLine(text, cx - w / 2.0f, cy - lh * 0.65f);
}

void TextCenterRect(const std::u16string& text, float x, float y, float w,
                    float h, uint8_t size, SDL_Color color) {
  if (text.empty()) {
    return;
  }
  LoadFont(size, color);
  float tw = Freetype::LoadW(text);
  float lh = Freetype::LoadLh();
  Freetype::DrawLine(text, x + (w - tw) / 2.0f, y + h / 2.0f - lh * 0.65f);
}

void TextLeft(const std::u16string& text, float x, float y, uint8_t size,
              SDL_Color color) {
  if (text.empty()) {
    return;
  }
  LoadFont(size, color);
  Freetype::DrawLine(text, x, y);
}

void TextRight(const std::u16string& text, float right, float y, uint8_t size,
               SDL_Color color) {
  if (text.empty()) {
    return;
  }
  LoadFont(size, color);
  float w = Freetype::LoadW(text);
  Freetype::DrawLine(text, right - w, y);
}

std::u16string Ellipsis(const std::u16string& text, float max_w) {
  if (Freetype::LoadW(text) <= max_w) {
    return text;
  }
  std::u16string out = text;
  while (!out.empty() && Freetype::LoadW(out + u"...") > max_w) {
    out.pop_back();
  }
  return out + u"...";
}

void TextCenterBox(const std::u16string& text, float left, float right,
                   float top, float bottom, uint8_t size, SDL_Color color) {
  if (text.empty()) {
    return;
  }
  LoadFont(size, color);
  auto fit = Ellipsis(text, (right - left) - 10.0f);
  float tw = Freetype::LoadW(fit);
  float lh = Freetype::LoadLh();
  Freetype::DrawLine(fit, left + ((right - left) - tw) / 2.0f,
                     (top + bottom) / 2.0f - lh * 0.65f);
}


enum class CsCategory {
  kSeasonal,
  kSpecial,
  kFashion,
  kBeauty,
  kPets,
  kUtility,
  kOther,
};

struct CashCommodity {
  std::u16string id;
  std::u16string name;
  int price = 0;
  int count = 1;
  CsCategory cat = CsCategory::kOther;
};

std::vector<CashCommodity> g_commodities;
std::vector<SDL_Texture*> g_commodity_icons;
bool g_commodities_loaded = false;

int g_active_tab = 0;
int g_page = 0;

int g_item_inven_tab = 0;

int g_locker_scroll = 0;
int g_item_inven_scroll = 0;

int ReadIntChild(wz::Node* node, const char16_t* key, int def) {
  if (node == nullptr) {
    return def;
  }
  auto* child = node->get_child(key);
  if (child == nullptr || child->type != wz::Type::Int) {
    return def;
  }
  return static_cast<wz::Property<int>*>(child)->get();
}

std::u16string ReadStringChild(wz::Node* node, const char16_t* key) {
  if (node == nullptr) {
    return {};
  }
  auto* child = node->get_child(key);
  if (child == nullptr || child->type != wz::Type::String) {
    return {};
  }
  return static_cast<wz::Property<std::u16string>*>(child)->get();
}

std::u16string PadItemId(int item_id) {
  std::u16string u = ToU16(std::to_string(item_id));
  if (u.size() < 8) {
    u.insert(0, 8 - u.size(), u'0');
  }
  return u;
}

std::u16string LoadItemName(const std::u16string& id8) {
  if (WzResource::string_ == nullptr) {
    return {};
  }
  std::u16string raw = id8;
  auto first = raw.find_first_not_of(u'0');
  if (first == std::u16string::npos) {
    return {};
  }
  raw = raw.substr(first);
  for (const char16_t* img : {u"Cash.img/", u"Pet.img/", u"Ins.img/",
                              u"Consume.img/", u"Etc.img/Etc/", u"Etc.img/"}) {
    auto name = ReadStringChild(
        WzResource::string_->find(std::u16string(img) + raw), u"name");
    if (!name.empty()) {
      return name;
    }
  }
  auto* eqp = WzResource::string_->find(u"Eqp.img/ClassicWorld");
  if (eqp != nullptr) {
    for (const auto& pair : *eqp->get_children()) {
      auto name = ReadStringChild(eqp->find(pair.first + u"/" + raw), u"name");
      if (!name.empty()) {
        return name;
      }
    }
  }
  return {};
}

CsCategory CategorizeCommodity(int item_id, const std::u16string& id8,
                               int cls) {
  if (cls == 0) {
    return CsCategory::kSeasonal;
  }
  if (cls == 2) {
    return CsCategory::kSpecial;
  }
  if (item_id >= 5000000 && item_id < 5020000) {
    return CsCategory::kPets;
  }
  auto etype = EquipGameInstance::LoadEquipType(id8);
  if (etype == u"PetEquip") {
    return CsCategory::kPets;
  }
  if (etype == u"Hair" || etype == u"Face" || etype == u"Skin" ||
      etype == u"Body") {
    return CsCategory::kBeauty;
  }
  if (!etype.empty()) {
    return CsCategory::kFashion;
  }
  auto itype = ItemGameInstance::LoadItemType(id8);
  if (itype == u"Cash" || itype == u"Consume" || itype == u"Etc" ||
      itype == u"Ins") {
    return CsCategory::kUtility;
  }
  return CsCategory::kOther;
}

SDL_Texture* LoadCommodityIcon(const std::u16string& id) {
  auto type = ItemGameInstance::LoadItemType(id);
  if (type == u"Cash" || type == u"Consume" || type == u"Etc" ||
      type == u"Ins" || type == u"Pet") {
    auto* info = ItemGameInstance::LoadItemInfo(id, 0);
    if (info != nullptr) {
      for (const char16_t* key : {u"icon", u"iconRaw"}) {
        auto* canvas = info->get_child(key);
        if (canvas != nullptr && canvas->type == wz::Type::Canvas) {
          return WzResource::LoadTexture(canvas);
        }
      }
    }
  }
  auto* equip = EquipGameInstance::LoadEquipInfo(id);
  if (equip != nullptr) {
    auto* canvas = equip->get_child(u"icon");
    if (canvas != nullptr && canvas->type == wz::Type::Canvas) {
      return WzResource::LoadTexture(canvas);
    }
  }
  return nullptr;
}

void LoadCommodities() {
  if (g_commodities_loaded) {
    return;
  }
  g_commodities_loaded = true;
  auto* root = WzResource::etc_->find(u"Commodity.img");
  if (root == nullptr) {
    return;
  }
  auto* children = root->get_children();
  if (children == nullptr) {
    return;
  }
  for (auto it = children->begin(); it != children->end(); ++it) {
    for (auto* node : it->second) {
      int item_id = ReadIntChild(node, u"ItemId", 0);
      if (item_id == 0 || ReadIntChild(node, u"OnSale", 1) != 1) {
        continue;
      }
      int cls = ReadIntChild(node, u"Class", -1);
      CashCommodity c;
      c.id = PadItemId(item_id);
      c.name = LoadItemName(c.id);
      c.price = 0;
      c.count = ReadIntChild(node, u"Count", 1);
      if (c.count <= 0) {
        c.count = 1;
      }
      c.cat = CategorizeCommodity(item_id, c.id, cls);
      g_commodities.push_back(std::move(c));
    }
  }
  g_commodity_icons.reserve(g_commodities.size());
  for (auto& c : g_commodities) {
    g_commodity_icons.push_back(LoadCommodityIcon(c.id));
  }
}

std::vector<int> FilterCommodities() {
  std::vector<int> result;
  for (int i = 0; i < static_cast<int>(g_commodities.size()); ++i) {
    const auto& c = g_commodities[i];
    bool keep = false;
    switch (g_active_tab) {
      case 0:
        keep = true;
        break;
      case 1:
        keep = c.cat == CsCategory::kSeasonal;
        break;
      case 2:
        keep = c.cat == CsCategory::kSpecial;
        break;
      case 3:
        keep = c.cat == CsCategory::kFashion;
        break;
      case 4:
        keep = c.cat == CsCategory::kBeauty;
        break;
      case 5:
        keep = c.cat == CsCategory::kPets;
        break;
      case 6:
        keep = c.cat == CsCategory::kUtility;
        break;
      default:
        keep = false;
        break;
    }
    if (keep) {
      result.push_back(i);
    }
  }
  return result;
}

int PageCount() {
  int total = static_cast<int>(FilterCommodities().size());
  return std::max(1, (total + kPageSize - 1) / kPageSize);
}

SDL_FPoint CardPos(int slot) {
  int col = slot % kGridCols;
  int row = slot / kGridCols;
  return {g_cash_shop_pos.x + kGridX +
              static_cast<float>(col) * (kCardW + kGridGapX),
          g_cash_shop_pos.y + kGridY +
              static_cast<float>(row) * (kCardH + kGridGapY)};
}

std::u16string LoadJobName() {
  auto& self = CharacterGameInstance::self_;
  if (self.job.empty() || WzResource::ms_ == nullptr) {
    return {};
  }
  auto* node = WzResource::ms_->get_root()->find(u"String.img/" + self.job);
  if (node == nullptr || node->type != wz::Type::String) {
    return {};
  }
  return static_cast<wz::Property<std::u16string>*>(node)->get();
}

std::u16string LoadSubCategoryText() {
  switch (g_active_tab) {
    case 5:
      return u"Pets     |     Pet Equip     |     Pet Care";
    default:
      return {};
  }
}


void RenderSceneBackdrop() {
  auto* node = FindUi(u"preview/layer:preview_back0");
  if (node == nullptr) {
    return;
  }
  auto* texture = WzResource::LoadTexture(node);
  if (texture == nullptr) {
    return;
  }
  SDL_Rect clip{static_cast<int>(g_cash_shop_pos.x + kSceneX),
                static_cast<int>(g_cash_shop_pos.y + kSceneY),
                static_cast<int>(kSceneW), static_cast<int>(kSceneH)};
  SDL_SetRenderClipRect(Window::renderer_, &clip);
  SDL_FRect dst{g_cash_shop_pos.x + kSceneX, g_cash_shop_pos.y + kSceneY,
                kSceneW, kSceneH};
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &dst);
  SDL_SetRenderClipRect(Window::renderer_, nullptr);
}

void RenderPreviewPanel() {
  RenderSceneBackdrop();

  TextCenterBox(LoadJobName(), g_cash_shop_pos.x + kJobBoxL,
                g_cash_shop_pos.x + kJobBoxR, g_cash_shop_pos.y + kNameBoxT,
                g_cash_shop_pos.y + kNameBoxB, kFontSize, kTextDark);
  TextCenterBox(CharacterGameInstance::self_.name,
                g_cash_shop_pos.x + kNameBoxL, g_cash_shop_pos.x + kNameBoxR,
                g_cash_shop_pos.y + kNameBoxT, g_cash_shop_pos.y + kNameBoxB,
                kFontSize, kTextDark);

  RenderButton(u"preview/button:preview_back");
  RenderButton(u"preview/button:zoom");
  RenderButton(u"preview/button:petOff");
  RenderButton(u"preview/button:playAction");
  RenderButton(u"preview/button:chat");
  RenderButton(u"preview/button:prev");
  RenderButton(u"preview/button:next");
  RenderButton(u"preview/button:default");
  RenderButton(u"preview/button:takeoff");
  RenderButton(u"preview/button:buyall");
}


Scroll MakeScroll(int val, int count, int visible, float x, float y,
                  int length) {
  Scroll scroll(static_cast<uint32_t>(std::max(0, val)),
                static_cast<uint32_t>(std::max(0, count)), length,
                static_cast<uint32_t>(std::max(0, visible)));
  scroll.pos = {g_cash_shop_pos.x + x, g_cash_shop_pos.y + y};
  scroll.top = true;
  return scroll;
}

bool InScrollBar(float x, float y, int length) {
  return MouseIn(g_cash_shop_pos.x + x, g_cash_shop_pos.y + y, 15.0f,
                 static_cast<float>(length));
}

void RenderCombo(float x, float y, const std::u16string& label) {
  auto* mid = FindUi(u"cashLocker/combo:sort/button/normal/1");
  if (mid != nullptr) {
    auto* tex = WzResource::LoadTexture(mid);
    if (tex != nullptr) {
      SDL_FRect dst{g_cash_shop_pos.x + x + 1.0f, g_cash_shop_pos.y + y + 2.0f,
                    static_cast<float>(tex->w), static_cast<float>(tex->h)};
      SDL_RenderTexture(Window::renderer_, tex, nullptr, &dst);
    }
  }
  TextCenterRect(label, g_cash_shop_pos.x + x, g_cash_shop_pos.y + y + 2.0f,
                 88.0f, 18.0f, kFontSize, kTextWhite);
}

void RenderCashLocker() {
  DrawUi(u"cashLocker/layer:searchBack");
  RenderCombo(300.0f, 436.0f, u"Newest");
  RenderButton(u"cashLocker/button:moveToInven");
  RenderButton(u"cashLocker/button:destroy");
  DrawUi(u"cashLocker/canvas:itemSlot");
  MakeScroll(g_locker_scroll, kInvenSlotCount, kLockerCols * kLockerRows,
             kLockerScrollX, kLockerScrollY, kLockerScrollLength)
      .Render();
}

void RenderInventoryItems(float origin_x, float origin_y, int col_count,
                          int row_count, int tab_index, int scroll) {
  if (tab_index < 0 || tab_index > 1) {
    return;
  }
  int data_index = (tab_index == 0) ? static_cast<int>(ItemEnum::kCash)
                                    : static_cast<int>(ItemEnum::kDeco);
  auto& items = PackageGameInstance::data_[data_index];
  int capacity = col_count * row_count;
  for (int i = 0; i < capacity; ++i) {
    int slot_index = scroll + i;
    if (slot_index < 0 || slot_index >= static_cast<int>(items.size())) {
      continue;
    }
    if (items[slot_index]->id.empty()) {
      continue;
    }
    const std::u16string& id = items[slot_index]->id;
    wz::Node* info = (tab_index == 1) ? EquipGameInstance::LoadEquipInfo(id)
                                      : ItemGameInstance::LoadItemInfo(id, 0);
    if (info == nullptr) {
      continue;
    }
    auto* canvas = info->get_child(u"icon");
    if (canvas == nullptr) {
      continue;
    }
    auto* icon = WzResource::LoadTexture(canvas);
    if (icon == nullptr) {
      continue;
    }
    int col = i % col_count;
    int row = i / col_count;
    float cell_x =
        g_cash_shop_pos.x + origin_x + static_cast<float>(col) * kSlotW;
    float cell_y =
        g_cash_shop_pos.y + origin_y + static_cast<float>(row) * kSlotW;
    SDL_FRect dst{std::floor(cell_x + (kSlotW - icon->w) / 2.0f),
                  std::floor(cell_y + (kSlotW - icon->h) / 2.0f),
                  static_cast<float>(icon->w), static_cast<float>(icon->h)};
    SDL_RenderTexture(Window::renderer_, icon, nullptr, &dst);
  }
}

void RenderItemInvenPanel() {
  for (int i = 0; i < 2; ++i) {
    const char16_t* state = (g_item_inven_tab == i) ? u"selected" : u"normal";
    std::u16string suffix(1, static_cast<char16_t>(u'0' + i));
    auto* node =
        FindUi(u"itemInven/tab:inven/" + std::u16string(state) + u"/" + suffix);
    if (node == nullptr) {
      continue;
    }
    auto* texture = WzResource::LoadTexture(node);
    float x = g_cash_shop_pos.x + 189.0f + static_cast<float>(i) * 39.0f;
    float y = g_cash_shop_pos.y + 586.0f;
    if (texture != nullptr) {
      y += 20.0f - static_cast<float>(texture->h);
      SDL_FRect dst{x, y, static_cast<float>(texture->w),
                    static_cast<float>(texture->h)};
      SDL_RenderTexture(Window::renderer_, texture, nullptr, &dst);
    }
  }
  DrawUi(u"itemInven/layer:searchBack");
  RenderCombo(322.0f, 616.0f, u"Default");
  RenderButton(u"itemInven/button:moveToLocker");
  DrawUi(u"itemInven/canvas:itemSlot");
  RenderInventoryItems(189.0f, 640.0f, kItemInvenCols, kItemInvenRows,
                       g_item_inven_tab, g_item_inven_scroll);
  MakeScroll(g_item_inven_scroll, kInvenSlotCount,
             kItemInvenCols * kItemInvenRows, kItemInvenScrollX,
             kItemInvenScrollY, kItemInvenScrollLength)
      .Render();
}


void RenderTabs() {
  for (int i = 0; i < kTabCount; ++i) {
    std::u16string suffix(1, static_cast<char16_t>(u'0' + i));
    const char16_t* state = (i == g_active_tab) ? u"selected" : u"normal";
    auto* node = FindUi(u"context/tab:category/" + std::u16string(state) +
                        u"/" + suffix);
    if (node == nullptr) {
      continue;
    }
    auto* texture = WzResource::LoadTexture(node);
    if (texture == nullptr) {
      continue;
    }
    SDL_FRect pos_rect = {
        g_cash_shop_pos.x + kTabX + static_cast<float>(i) * kTabStride,
        g_cash_shop_pos.y + kTabY,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  }
}

void RenderSaleCard(float x, float y, int index) {
  const auto& commodity = g_commodities[index];

  DrawTextureAt(u"saleEntry/backgrnd", x, y);

  auto* icon = g_commodity_icons[index];
  if (icon != nullptr && icon->w > 0 && icon->h > 0) {
    constexpr float kBoxX = 6.0f;
    constexpr float kBoxY = 6.0f;
    constexpr float kBoxW = 76.0f;
    constexpr float kBoxH = 76.0f;
    float w = static_cast<float>(icon->w) * 2.0f;
    float h = static_cast<float>(icon->h) * 2.0f;
    float fit = std::min({1.0f, kBoxW / w, kBoxH / h});
    w *= fit;
    h *= fit;
    SDL_FRect dst{x + kBoxX + (kBoxW - w) / 2.0f,
                  y + kBoxY + (kBoxH - h) / 2.0f, w, h};
    SDL_RenderTexture(Window::renderer_, icon, nullptr, &dst);
  }

  bool has_quantity = commodity.count > 1;
  LoadFont(kFontSize, kTextDark);
  auto name = Ellipsis(commodity.name, has_quantity ? 112.0f : 136.0f);
  Freetype::DrawLine(name, x + 90.0f, y + 11.0f);
  if (has_quantity) {
    TextRight(u"x" + Num(commodity.count), x + 225.0f, y + 11.0f, kFontSize,
              SDL_Color{96, 96, 96, 255});
  }

  auto* normal = FindUi(u"saleEntry/btBuy/normal/0");
  if (normal != nullptr) {
    auto* ntex = WzResource::LoadTexture(normal);
    if (ntex != nullptr) {
      bool hover = MouseIn(x + 90.0f, y + 55.0f, static_cast<float>(ntex->w),
                           static_cast<float>(ntex->h));
      auto* node = hover ? FindUi(u"saleEntry/btBuy/mouseOver/0") : normal;
      if (node == nullptr) {
        node = normal;
      }
      auto* t = WzResource::LoadTexture(node);
      if (t == nullptr) {
        t = ntex;
      }
      SDL_FRect r{x + 90.0f, y + 55.0f, static_cast<float>(t->w),
                  static_cast<float>(t->h)};
      SDL_RenderTexture(Window::renderer_, t, nullptr, &r);
    }
    std::u16string price = Money(commodity.price) + u" NX";
    TextCenterRect(price, x + 90.0f, y + 55.0f, 134.0f, 21.0f, kFontSize,
                   kTextDark);
  }
}

void DrawPageArrow(float x, float y, bool right, SDL_Color color) {
  constexpr int kRows = 10;
  constexpr float kWidth = 8.0f;
  const float half = kRows / 2.0f;
  SDL_SetRenderDrawColor(Window::renderer_, color.r, color.g, color.b, color.a);
  SDL_FRect rects[kRows];
  for (int i = 0; i < kRows; ++i) {
    float t = 1.0f - std::fabs(static_cast<float>(i) + 0.5f - half) / half;
    float w = std::max(1.0f, std::round(kWidth * t));
    rects[i] = SDL_FRect{right ? x : x + (kWidth - w),
                         y + static_cast<float>(i), w, 1.0f};
  }
  SDL_RenderFillRects(Window::renderer_, rects, kRows);
}

void RenderPageBar(int page, int pages) {
  const float left = g_cash_shop_pos.x + 752.0f;
  const float top = g_cash_shop_pos.y + 640.0f;
  const bool can_prev = page > 0;
  const bool can_next = page + 1 < pages;
  const bool hover_prev = MouseIn(left, top, 30.0f, 25.0f);
  const bool hover_next = MouseIn(left + 129.0f, top, 30.0f, 25.0f);
  auto color = [](bool enabled, bool hover) {
    if (!enabled) {
      return SDL_Color{112, 132, 152, 255};
    }
    if (hover) {
      return SDL_Color{255, 176, 214, 255};
    }
    return SDL_Color{255, 255, 255, 255};
  };
  DrawPageArrow(left + 11.0f, top + 7.0f, false, color(can_prev, hover_prev));
  DrawPageArrow(left + 140.0f, top + 7.0f, true, color(can_next, hover_next));
  TextCenterRect(Num(page + 1) + u"/" + Num(pages), left + 30.0f, top, 99.0f,
                 25.0f, kFontSize, kTextWhite);
}

void RenderSaleList() {
  LoadCommodities();
  auto filtered = FilterCommodities();
  int total = static_cast<int>(filtered.size());
  int pages = std::max(1, (total + kPageSize - 1) / kPageSize);
  g_page = std::clamp(g_page, 0, pages - 1);
  int start = g_page * kPageSize;

  if (total > 0) {
    for (int slot = 0; slot < kPageSize; ++slot) {
      int idx = start + slot;
      auto pos = CardPos(slot);
      if (idx >= total) {
        DrawTextureAt(u"saleEntry/empty", pos.x, pos.y);
        continue;
      }
      RenderSaleCard(pos.x, pos.y, filtered[idx]);
    }

    RenderPageBar(g_page, pages);
    return;
  }

  DrawUi(u"context/layer:noResultsFound");
  TextCenterAt(u"No results found.", g_cash_shop_pos.x + 818.0f,
               g_cash_shop_pos.y + 446.0f, kFontSize, kTextWhite);
}

void RenderWallet() {
  TextLeft(u"NX", g_cash_shop_pos.x + 509.0f, g_cash_shop_pos.y + 705.0f,
           kFontSize, kTextDark);
  TextRight(Money(kNxBalance), g_cash_shop_pos.x + 685.0f,
            g_cash_shop_pos.y + 705.0f, kFontSize, kTextDark);
  TextLeft(u"Leaf Points", g_cash_shop_pos.x + 490.0f,
           g_cash_shop_pos.y + 733.0f, kFontSize, kTextDark);
  TextRight(Money(kLeafPointBalance), g_cash_shop_pos.x + 685.0f,
            g_cash_shop_pos.y + 733.0f, kFontSize, kTextDark);
}

void RenderContextPanel() {
  RenderPanel(u"context/backgrnd");
  RenderPanel(u"context/categoryBar/" + Num(std::clamp(g_active_tab, 0, 7)));
  RenderTabs();
  TextLeft(LoadSubCategoryText(), g_cash_shop_pos.x + 477.0f,
           g_cash_shop_pos.y + 130.0f, kFontSize, kTextWhite);
  LoadCommodities();
  if (!FilterCommodities().empty()) {
    RenderPanel(u"context/layer:searchBackgrnd");
  }
  RenderButton(u"context/button:checkCash");
  RenderButton(u"context/button:coupon");
  RenderButton(u"context/button:probability");
  RenderButton(u"context/button:help");
  RenderButton(u"context/button:exit");
}


void HandleCashShopClick(float mx, float my) {
  SDL_FPoint p{mx, my};

  SDL_FRect exit_rect{g_cash_shop_pos.x + 1032.0f, g_cash_shop_pos.y + 707.0f,
                      67.0f, 36.0f};
  if (SDL_PointInRectFloat(&p, &exit_rect)) {
    CashShopUiSystem::Close();
    return;
  }

  for (int i = 0; i < kTabCount; ++i) {
    SDL_FRect r{g_cash_shop_pos.x + kTabX + static_cast<float>(i) * kTabStride,
                g_cash_shop_pos.y + kTabY, kTabStride, 47.0f};
    if (SDL_PointInRectFloat(&p, &r)) {
      g_active_tab = i;
      g_page = 0;
      return;
    }
  }

  SDL_FRect prev_rect{g_cash_shop_pos.x + 752.0f, g_cash_shop_pos.y + 640.0f,
                      30.0f, 25.0f};
  SDL_FRect next_rect{g_cash_shop_pos.x + 881.0f, g_cash_shop_pos.y + 640.0f,
                      30.0f, 25.0f};
  if (SDL_PointInRectFloat(&p, &prev_rect)) {
    g_page = std::max(0, g_page - 1);
    return;
  }
  if (SDL_PointInRectFloat(&p, &next_rect)) {
    g_page = std::min(PageCount() - 1, g_page + 1);
    return;
  }

  if (InScrollBar(kLockerScrollX, kLockerScrollY, kLockerScrollLength)) {
    g_locker_scroll = static_cast<int>(
        MakeScroll(g_locker_scroll, kInvenSlotCount, kLockerCols * kLockerRows,
                   kLockerScrollX, kLockerScrollY, kLockerScrollLength)
            .Click(p));
    return;
  }
  if (InScrollBar(kItemInvenScrollX, kItemInvenScrollY,
                  kItemInvenScrollLength)) {
    g_item_inven_scroll = static_cast<int>(
        MakeScroll(g_item_inven_scroll, kInvenSlotCount,
                   kItemInvenCols * kItemInvenRows, kItemInvenScrollX,
                   kItemInvenScrollY, kItemInvenScrollLength)
            .Click(p));
    return;
  }

  if (MouseIn(g_cash_shop_pos.x + 392.0f, g_cash_shop_pos.y + 436.0f, 19.0f,
              18.0f) ||
      MouseIn(g_cash_shop_pos.x + 414.0f, g_cash_shop_pos.y + 436.0f, 19.0f,
              18.0f) ||
      MouseIn(g_cash_shop_pos.x + 414.0f, g_cash_shop_pos.y + 616.0f, 19.0f,
              18.0f)) {
    return;
  }

  for (int i = 0; i < 2; ++i) {
    SDL_FRect r{g_cash_shop_pos.x + 189.0f + static_cast<float>(i) * 39.0f,
                g_cash_shop_pos.y + 586.0f, 37.0f, 22.0f};
    if (SDL_PointInRectFloat(&p, &r)) {
      g_item_inven_tab = i;
      return;
    }
  }

  // 商品卡片：点一下试穿，再点一次取消。
  auto filtered = FilterCommodities();
  for (int slot = 0; slot < kPageSize; ++slot) {
    int idx = g_page * kPageSize + slot;
    if (idx >= static_cast<int>(filtered.size())) {
      break;
    }
    auto pos = CardPos(slot);
    if (MouseIn(pos.x, pos.y, kCardW, kCardH)) {
      TryOnItem(g_commodities[filtered[idx]].id);
      return;
    }
  }
}

void OpenImmediate() {
  ServerHeartbeatInstance::client_started = false;

  fbs::ClientSceneT leave_scene;
  leave_scene.fade = false;
  leave_scene.map_id = 0;
  ClientRequest::SendToHost(leave_scene);

  CameraGameInstance::camera_.x = 0;
  CameraGameInstance::camera_.y = 0;

  auto& self = CharacterGameInstance::self_;
  g_on_ladder = false;
  g_hspeed = 0.0f;
  g_vspeed = 0.0f;
  g_hforce = 0.0f;
  g_phys_page = 0;
  g_fh = 1;
  self.pos.x = (kStandLeft + kStandRight) / 2.0f;
  self.pos.y = kStandFloor;
  self.flip = 0;
  // 试穿角色固定用默认混合（不透明度/颜色 255），避免带出地图上的受击闪白等状态。
  self.color = SDL_Color{255, 255, 255, 255};
  self.face.action = u"default";
  self.face.index = 0;
  self.action_animate = true;
  self.chair = std::nullopt;
  self.tomb = std::nullopt;
  self.skill = std::nullopt;
  self.effect.clear();
  self.chatballoon = std::nullopt;
  CharacterLogicSystem::RunStandAction(self);

  System::logic_systems_ = {CashShopUiSystem::Run, CursorLogicSystem::Run};
  System::render_systems_ = {CashShopUiSystem::Render,
                             CursorRenderSystem::Render};
  System::event_systems_ = {CashShopUiSystem::Event,
                            KeyboardInputSystem::Event};
  KeyboardInputSystem::Reset();

  // 记下进入商城时的点装，退出时还原试穿效果。
  SaveDecoSnapshot();

  g_active_tab = 0;
  g_page = 0;
  g_item_inven_tab = 0;
  g_locker_scroll = 0;
  g_item_inven_scroll = 0;

  g_cash_shop_visible = true;
  g_cash_shop_fading = false;

  FadeSystemInstance::EnterOut();
  FadeSystemInstance::EnterOut();
}


bool HasItemString(const std::u16string& id) {
  if (WzResource::string_ == nullptr) {
    return false;
  }
  auto type = ItemGameInstance::LoadItemType(id);
  auto* img = WzResource::string_->find(type + u".img");
  if (img == nullptr) {
    return false;
  }
  if (type == u"Etc") {
    img = img->get_child(u"Etc");
    if (img == nullptr) {
      return false;
    }
  }
  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });
  std::u16string raw(view.begin(), view.end());
  auto* entry = img->get_child(raw);
  if (entry == nullptr) {
    return false;
  }
  auto* name = entry->get_child(u"name");
  return name != nullptr && name->type == wz::Type::String;
}

void RenderCommodityTooltip(const CashCommodity& commodity) {
  const SDL_FPoint pos{Window::mouse_pos_.x + 15.0f,
                       Window::mouse_pos_.y + 15.0f};
  const auto& id = commodity.id;
  if (!EquipGameInstance::LoadEquipType(id).empty()) {
    auto* info = EquipGameInstance::LoadEquipInfo(id);
    auto* icon = info == nullptr ? nullptr : info->get_child(u"icon");
    if (icon != nullptr && icon->type == wz::Type::Canvas) {
      GameDecoItem deco;
      deco.id = id;
      TooltipUiSystem::RenderDeco(deco, pos.x, pos.y);
      return;
    }
    TooltipUiSystem::RenderStr(commodity.name.empty() ? id : commodity.name,
                               pos.x, pos.y);
    return;
  }
  if (!HasItemString(id)) {
    return;
  }
  GameItem item;
  item.id = id;
  item.type = ItemEnum::kCash;
  TooltipUiSystem::RenderItem(item, pos.x, pos.y);
}

void RenderHoverTooltip() {
  auto filtered = FilterCommodities();
  int total = static_cast<int>(filtered.size());
  int start = g_page * kPageSize;
  for (int slot = 0; slot < kPageSize && start + slot < total; ++slot) {
    auto pos = CardPos(slot);
    if (MouseIn(pos.x, pos.y, kCardW, kCardH)) {
      RenderCommodityTooltip(g_commodities[filtered[start + slot]]);
      return;
    }
  }

  int data_index = (g_item_inven_tab == 0) ? static_cast<int>(ItemEnum::kCash)
                                           : static_cast<int>(ItemEnum::kDeco);
  auto& items = PackageGameInstance::data_[data_index];
  for (int i = 0; i < kItemInvenCols * kItemInvenRows; ++i) {
    float cell_x = g_cash_shop_pos.x + 189.0f +
                   static_cast<float>(i % kItemInvenCols) * kSlotW;
    float cell_y = g_cash_shop_pos.y + 640.0f +
                   static_cast<float>(i / kItemInvenCols) * kSlotW;
    if (!MouseIn(cell_x, cell_y, kSlotW, kSlotW)) {
      continue;
    }
    int slot_index = g_item_inven_scroll + i;
    if (slot_index < 0 || slot_index >= static_cast<int>(items.size()) ||
        items[slot_index]->id.empty()) {
      return;
    }
    const SDL_FPoint pos{Window::mouse_pos_.x + 15.0f,
                         Window::mouse_pos_.y + 15.0f};
    if (g_item_inven_tab == 1) {
      TooltipUiSystem::RenderDeco(
          static_cast<GameDecoItem&>(*items[slot_index]), pos.x, pos.y);
    } else if (HasItemString(items[slot_index]->id)) {
      TooltipUiSystem::RenderItem(*items[slot_index], pos.x, pos.y);
    }
    return;
  }
}

}  // namespace

bool CashShopUiSystem::Run() {
  auto& self = CharacterGameInstance::self_;
  const float dt = static_cast<float>(Window::delta_time_) / 1000.0f;
  // Physic 内部会除以 delta_time，dt 为 0 会算出 NaN（人物会整个消失）。
  if (dt <= 0.0f) {
    CharacterLogicSystem::RunAnimate(self);
    return true;
  }
  // 试穿窗口里的角色始终按默认颜色/透明度（255）绘制。
  self.color = SDL_Color{255, 255, 255, 255};

  const auto& input = CharacterLogicSystem::character_action_input_;
  const bool left = input.contains("left");
  const bool right = input.contains("right");
  const bool up = input.contains("up");
  const bool down = input.contains("down");
  const bool jump = input.contains("jump");

  auto& hspeed = g_hspeed;
  auto& vspeed = g_vspeed;
  auto& hforce = g_hforce;
  auto& fh = g_fh;
  auto& page = g_phys_page;
  const auto& fhs = PreviewFootholds();
  const bool near_ladder = std::abs(self.pos.x - kLadderX) <= kLadderGrab;

  if (g_on_ladder) {
    // 和地图 RunClimbing / RunJump(kClimb) 一致：
    //   - 上下键爬（100px/s），只有「跳跃 + 方向键」才能跳离梯子；
    //   - 在梯子最高点继续按上：不再上升，动画静止；
    //   - 在梯子最下方按下：自动脱离梯子，落回地面。
    const bool jump_off = jump && (left || right);
    if (jump_off) {
      g_on_ladder = false;
      fh = 0;
      g_ladder_cooldown = Window::dt_now_ + 120;
      vspeed = -310.0f;
      hspeed = left ? -140.0f : 140.0f;
      self.flip = hspeed < 0.0f ? 0 : 1;
      self.action_animate = true;
      CharacterLogicSystem::RunAction(self, u"jump");
    } else {
      if (up) {
        vspeed = -100.0f;
      } else if (down) {
        vspeed = 100.0f;
      } else {
        vspeed = 0.0f;
      }
      self.pos.x = kLadderX;
      self.pos.y += vspeed * dt;
      self.flip = 0;
      CharacterLogicSystem::RunAction(self, u"ladder");
      self.action_animate = vspeed != 0.0f;
      if (self.pos.y < kLadderTop) {
        // 梯子最高点：停在顶端，继续按上动画不再播放。
        self.pos.y = kLadderTop;
        vspeed = 0.0f;
        self.action_animate = false;
      } else if (self.pos.y > kLadderBottom) {
        // 梯子最下方：按下后脱离梯子回到地面。
        self.pos.y = kLadderBottom;
        g_on_ladder = false;
        g_ladder_cooldown = Window::dt_now_ + 80;
        fh = 1;
        hspeed = 0.0f;
        vspeed = 0.0f;
        self.action_animate = true;
        CharacterLogicSystem::RunStandAction(self);
      }
    }
  } else if (fh == 0) {
    vspeed += 2000.0f * dt;
    if ((up || down) && near_ladder && self.pos.y > kLadderTop &&
        self.pos.y <= kLadderBottom + 5.0f &&
        Window::dt_now_ >= g_ladder_cooldown) {
      // 和地图一样：空中（跳跃中）靠到梯子范围内按上下都能抓住。
      g_on_ladder = true;
      self.pos.x = kLadderX;
      self.pos.y = std::clamp(self.pos.y, kLadderTop, kLadderBottom);
      self.flip = 0;
      vspeed = 0.0f;
      hspeed = 0.0f;
      CharacterLogicSystem::RunAction(self, u"ladder");
    } else {
      Physic::Fall(self.pos, dt, hspeed, vspeed,
                   CharacterStatGameInstance::GetVspeedMin(),
                   CharacterStatGameInstance::GetVspeedMax(), PreviewBorder(),
                   true, false, fh, page, fhs);
      self.pos.x = std::clamp(self.pos.x, kSceneX, kSceneX + kSceneW);
      CharacterLogicSystem::RunAction(self, u"jump");
      self.action_animate = true;
    }
  } else {
    if (left && !right) {
      hforce = -1400.0f;
    } else if (right && !left) {
      hforce = 1400.0f;
    } else {
      hforce = 0.0f;
    }
    if (jump) {
      vspeed = CharacterStatGameInstance::GetVspeedMin() * 0.1f;
      fh = 0;
      CharacterLogicSystem::RunAction(self, u"jump");
    } else if (near_ladder && Window::dt_now_ >= g_ladder_cooldown &&
               ((up && std::abs(self.pos.y - kLadderBottom) <= 5.0f) ||
                (down && self.pos.y >= kLadderTop - 10.0f &&
                 self.pos.y <= kLadderTop))) {
      // 和地图 RunClimb 一样：站在梯子底端按上键才能抓；
      // 站在梯子顶端（有平台）按下键才能抓。地面上按↓不会抓梯子。
      g_on_ladder = true;
      self.pos.x = kLadderX;
      hspeed = 0.0f;
      vspeed = 0.0f;
      CharacterLogicSystem::RunAction(self, u"ladder");
    } else if (down && !up) {
      // 和地图 RunProne 一样：站在地面上按↓趴下。
      hforce = 0.0f;
      hspeed = 0.0f;
      vspeed = 0.0f;
      CharacterLogicSystem::RunAction(self, u"prone");
      self.action_animate = true;
    } else {
      Physic::Walk(self.pos, dt, hspeed, vspeed, hforce,
                   CharacterStatGameInstance::GetHspeedMin(),
                   CharacterStatGameInstance::GetHspeedMax(), 800.0f, true, fh,
                   PreviewBorder(), fhs);
      self.pos.x = std::clamp(self.pos.x, kSceneX, kSceneX + kSceneW);
      if (hspeed < 0.0f) {
        self.flip = 0;
        CharacterLogicSystem::RunWalkAction(self);
      } else if (hspeed > 0.0f) {
        self.flip = 1;
        CharacterLogicSystem::RunWalkAction(self);
      } else {
        CharacterLogicSystem::RunStandAction(self);
      }
      self.action_animate = true;
    }
  }

  CharacterLogicSystem::RunAnimate(self);
  return true;
}

void CashShopUiSystem::RenderBackgrnd() {
  auto* node = FindBackgrnd();
  if (node == nullptr) {
    return;
  }
  auto texture = WzResource::LoadTexture(node);
  SDL_FRect pos_rect = {
      g_cash_shop_pos.x,
      g_cash_shop_pos.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

void CashShopUiSystem::RenderAvatar() {
  SDL_Rect clip{static_cast<int>(g_cash_shop_pos.x + kSceneX),
                static_cast<int>(g_cash_shop_pos.y + kSceneY),
                static_cast<int>(kSceneW), static_cast<int>(kSceneH)};
  SDL_SetRenderClipRect(Window::renderer_, &clip);
  CharacterRenderSystem::RenderCharacter(CharacterGameInstance::self_);
  SDL_SetRenderClipRect(Window::renderer_, nullptr);
}

bool CashShopUiSystem::Render() {
  RenderBackgrnd();
  RenderPanel(u"preview/backgrnd");
  RenderPanel(u"cashLocker/backgrnd");
  RenderPanel(u"itemInven/backgrnd");
  RenderPreviewPanel();
  RenderCashLocker();
  RenderItemInvenPanel();
  RenderContextPanel();
  RenderSaleList();
  RenderWallet();
  RenderAvatar();
  RenderHoverTooltip();
  return true;
}

bool CashShopUiSystem::Event(SDL_Event* event) {
  // 商城里屏蔽其它 UI 的快捷键（角色面板 / 背包 / 任务 / 好友 / 菜单……），
  // 只放行移动、跳跃这类 action 键。返回 false 会中断事件派发，
  // 后面的 KeyboardInputSystem 就不会把它当成 UI 键去 Toggle 面板。
  auto is_ui_key = [](SDL_Scancode scancode) {
    auto it = KeyboardGameInstance::data_.find(scancode);
    return it != KeyboardGameInstance::data_.end() && it->second.type == "ui";
  };
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      if (event->key.scancode == SDL_SCANCODE_ESCAPE) {
        Close();
        return false;
      }
      if (is_ui_key(event->key.scancode)) {
        return false;
      }
      return true;
    }
    case SDL_EVENT_KEY_UP: {
      if (is_ui_key(event->key.scancode)) {
        return false;
      }
      return true;
    }
    case SDL_EVENT_MOUSE_WHEEL: {
      const int step = event->wheel.y > 0.0f ? -kItemInvenCols : kItemInvenCols;
      auto scroll = [step](int val) {
        return std::clamp(val + step, 0,
                          kInvenSlotCount - kItemInvenCols * kItemInvenRows);
      };
      if (MouseIn(g_cash_shop_pos.x + 176.0f, g_cash_shop_pos.y + 400.0f,
                  269.0f, 146.0f)) {
        g_locker_scroll = scroll(g_locker_scroll);
        return false;
      }
      if (MouseIn(g_cash_shop_pos.x + 176.0f, g_cash_shop_pos.y + 550.0f,
                  269.0f, 214.0f)) {
        g_item_inven_scroll = scroll(g_item_inven_scroll);
        return false;
      }
      return true;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      return false;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        HandleCashShopClick(Window::mouse_pos_.x, Window::mouse_pos_.y);
      }
      return false;
    }
    default: {
      break;
    }
  }
  return true;
}

void CashShopUiSystem::Open() {
  if (g_cash_shop_visible || g_cash_shop_fading) {
    return;
  }
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it == System::render_systems_.end()) {
    return;
  }

  g_return_pos = FindNearestSp(CharacterGameInstance::self_.pos);

  auto wh = LoadWh();
  const auto& camera = CameraGameInstance::camera_;
  g_cash_shop_pos.x = (camera.w - wh.x) / 2;
  g_cash_shop_pos.y = (camera.h - wh.y) / 2;

  g_cash_shop_fading = true;
  FadeSystemInstance::EnterIn([] { OpenImmediate(); });
}

void CashShopUiSystem::Close() {
  if (!g_cash_shop_visible) {
    return;
  }
  g_cash_shop_visible = false;
  // 退出商城前把试穿的点装还原，地图上的角色恢复原样。
  RestoreDecoSnapshot();
  SceneSystemInstance::EnterCashShopReturn(SceneSystemInstance::map_id_,
                                           g_return_pos);
}

void CashShopUiSystem::Toggle() {
  if (g_cash_shop_visible) {
    Close();
  } else {
    Open();
  }
}

}  // namespace sdlms
