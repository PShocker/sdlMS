#include "src/client/system/ui/cash_shop_ui_system.h"

#include "src/client/game/game_character.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/cursor_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system_instance/fade_system_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/freetype/freetype.h"
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
#include <cstdint>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace sdlms {

namespace {

// 现金商店整体（1366x768 全屏底图）在当前屏幕上的左上角位置。
SDL_FPoint g_cash_shop_pos = {0, 0};

// 是否已进入现金商店。
bool g_cash_shop_visible = false;
// 是否正在做淡入/淡出切换，防止重复触发。
bool g_cash_shop_fading = false;

// 进入商城前角色在地图上的位置；退出时按这个位置重新进入地图。
SDL_FPoint g_return_pos = {0, 0};

// 找离 from 最近的 sp（出生点）传送门位置，与 LoadPreparePos 一样 y-5。
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

// 试穿移动范围：取自 CashShopPreview.img/foothold 最宽的一段水平地面。
float g_preview_left = 184.0f;
float g_preview_right = 436.0f;
float g_preview_floor = 250.0f;

// 真实素材路径：UI/CashShopUI.img，底图是 outlink 画布（LoadTexture 会解析）。
wz::Node* FindBackgrnd() {
  return WzResource::ui_->find(u"CashShopUI.img/backgrnd");
}

// 底图真实尺寸：解析 outlink 后为 1366x768。
SDL_FPoint LoadWh() {
  static const auto backgrnd = WzResource::LoadTexture(FindBackgrnd());
  return SDL_FPoint{static_cast<float>(backgrnd->w),
                    static_cast<float>(backgrnd->h)};
}

// 面板位置由画布的 origin 推算：绘制左上角 = -origin。
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

// 绘制 CashShopUI.img 里的子面板（试穿窗/衣帽间/背包格）。
void RenderPanel(const std::u16string& path) {
  auto node = WzResource::ui_->find(path);
  if (node == nullptr) {
    return;
  }
  auto texture = WzResource::LoadTexture(node);
  auto panel_pos = LoadOrigin(node);
  SDL_FRect pos_rect = {
      g_cash_shop_pos.x + panel_pos.x,
      g_cash_shop_pos.y + panel_pos.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

// 按钮按鼠标悬停切换 normal / mouseOver 状态（context 下的按钮用）。
void RenderButton(const std::u16string& base) {
  auto* normal = WzResource::ui_->find(base + u"/normal/0");
  if (normal == nullptr) {
    return;
  }
  auto npos = LoadOrigin(normal);
  auto* ntex = WzResource::LoadTexture(normal);
  SDL_FRect hit{g_cash_shop_pos.x + npos.x, g_cash_shop_pos.y + npos.y,
                static_cast<float>(ntex->w), static_cast<float>(ntex->h)};
  SDL_FPoint mouse{Window::mouse_pos_.x, Window::mouse_pos_.y};
  bool hover = SDL_PointInRectFloat(&mouse, &hit);
  auto* node = hover ? WzResource::ui_->find(base + u"/mouseOver/0") : normal;
  if (node == nullptr) {
    node = normal;
  }
  auto* t = WzResource::LoadTexture(node);
  auto pos = LoadOrigin(node);
  SDL_FRect pos_rect{g_cash_shop_pos.x + pos.x, g_cash_shop_pos.y + pos.y,
                     static_cast<float>(t->w), static_cast<float>(t->h)};
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
}

// 商品卡右侧购买按钮（相对卡片 x,y 定位）的 hover 效果。
void RenderCardBuyButton(float x, float y) {
  auto* normal =
      WzResource::ui_->find(u"CashShopUI.img/saleEntry/btBuy/normal/0");
  if (normal == nullptr) {
    return;
  }
  auto* ntex = WzResource::LoadTexture(normal);
  SDL_FRect hit{x + 90.0f, y + 55.0f, static_cast<float>(ntex->w),
                static_cast<float>(ntex->h)};
  SDL_FPoint mouse{Window::mouse_pos_.x, Window::mouse_pos_.y};
  bool hover = SDL_PointInRectFloat(&mouse, &hit);
  auto* node =
      hover
          ? WzResource::ui_->find(u"CashShopUI.img/saleEntry/btBuy/mouseOver/0")
          : normal;
  if (node == nullptr) {
    node = normal;
  }
  auto* t = WzResource::LoadTexture(node);
  SDL_FRect r{x + 90.0f, y + 55.0f, static_cast<float>(t->w),
              static_cast<float>(t->h)};
  SDL_RenderTexture(Window::renderer_, t, nullptr, &r);
}

// 从 CashShopPreview.img/foothold 取最宽的一段水平线作为走动范围。
void LoadPreviewRange() {
  auto* node = WzResource::ui_->find(u"CashShopPreview.img");
  if (node == nullptr) {
    return;
  }
  auto data = FootholdGameInstance::Load(node);
  float best = -1.0f;
  for (auto it = data.begin(); it != data.end(); ++it) {
    const auto& fh = it->second;
    // 只取水平地面（y1 == y2）。
    if (fh.t != fh.b) {
      continue;
    }
    float width = static_cast<float>(fh.r - fh.l);
    if (width > best) {
      best = width;
      g_preview_left = static_cast<float>(fh.l);
      g_preview_right = static_cast<float>(fh.r);
      g_preview_floor = static_cast<float>(fh.t);
    }
  }
}

// ---- 现金商店商品数据（Etc.wz/Commodity.img） ----

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
  std::u16string id;  // 8 位补零道具 id
  int price = 0;
  int count = 1;
  CsCategory cat = CsCategory::kOther;
};

std::vector<CashCommodity> g_commodities;
std::vector<SDL_Texture*> g_commodity_icons;
std::vector<bool> g_commodity_selected;
bool g_commodities_loaded = false;

int g_active_tab = 0;
int g_page = 0;
int g_cart_count = 0;
int g_cart_price = 0;

constexpr float kTabX = 466.0f;
constexpr float kTabY = 86.0f;
constexpr int kTabCount = 8;
constexpr float kTabStride = (1174.0f - kTabX) / kTabCount;

// saleEntry 卡片 235x87，3 列 x 5 行 = 15 张/页，铺在 entry
// 区(463,181)~(1178,637)。
constexpr float kEntryX = 465.0f;
constexpr float kEntryY = 183.0f;
constexpr float kCardW = 235.0f;
constexpr float kCardH = 87.0f;
constexpr float kCardStrideX = 238.0f;
constexpr float kCardStrideY = 91.0f;
constexpr int kCardCols = 3;
constexpr int kCardRows = 5;
constexpr int kPageSize = kCardCols * kCardRows;

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

std::u16string PadItemId(int item_id) {
  std::string s = std::to_string(item_id);
  std::u16string u(s.begin(), s.end());
  if (u.size() < 8) {
    u.insert(0, 8 - u.size(), u'0');
  }
  return u;
}

std::u16string NumToU16(int value) {
  std::string s = std::to_string(value);
  return std::u16string(s.begin(), s.end());
}

// 商品分类：Class 标记优先（0=季节 2=特别），否则按道具/装备槽位推断。
CsCategory CategorizeCommodity(int item_id, const std::u16string& id8,
                               int cls) {
  if (cls == 0) {
    return CsCategory::kSeasonal;
  }
  if (cls == 2) {
    return CsCategory::kSpecial;
  }
  if (item_id >= 5000000 && item_id <= 5009999) {
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

// 防御式加载道具图标：普通/现金道具走 Item，装备（含点装）走 Equip。
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
      c.price = ReadIntChild(node, u"Price", 0);
      c.count = ReadIntChild(node, u"Count", 1);
      c.cat = CategorizeCommodity(item_id, c.id, cls);
      g_commodities.push_back(std::move(c));
    }
  }
  g_commodity_icons.reserve(g_commodities.size());
  for (auto& c : g_commodities) {
    g_commodity_icons.push_back(LoadCommodityIcon(c.id));
  }
  g_commodity_selected.assign(g_commodities.size(), false);
}

// 右侧 context 面板顶部的 8 个分类 tab，带选中态。
void RenderTabs() {
  for (int i = 0; i < kTabCount; ++i) {
    std::u16string suffix(1, static_cast<char16_t>(u'0' + i));
    const char16_t* state = (i == g_active_tab) ? u"selected" : u"normal";
    auto* node = WzResource::ui_->find(u"CashShopUI.img/context/tab:category/" +
                                       std::u16string(state) + u"/" + suffix);
    if (node == nullptr) {
      continue;
    }
    auto* texture = WzResource::LoadTexture(node);
    SDL_FRect pos_rect = {
        g_cash_shop_pos.x + kTabX + static_cast<float>(i) * kTabStride,
        g_cash_shop_pos.y + kTabY,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  }
}

// 当前 tab 过滤后的商品下标。
std::vector<int> FilterCommodities() {
  std::vector<int> result;
  for (int i = 0; i < static_cast<int>(g_commodities.size()); ++i) {
    const auto& c = g_commodities[i];
    bool keep = false;
    switch (g_active_tab) {
      case 0:
        keep = true;  // MAIN：全部
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
        keep = c.cat == CsCategory::kOther;
        break;
    }
    if (keep) {
      result.push_back(i);
    }
  }
  return result;
}

void RecomputeCart() {
  g_cart_count = 0;
  g_cart_price = 0;
  for (int i = 0; i < static_cast<int>(g_commodities.size()); ++i) {
    if (i < static_cast<int>(g_commodity_selected.size()) &&
        g_commodity_selected[i]) {
      g_cart_count += g_commodities[i].count;
      g_cart_price += g_commodities[i].price;
    }
  }
}

// 单张 saleEntry 卡片：底框 + 图标 + 价格 + 购买按钮 + 加入购物车勾选框。
void RenderSaleCard(float x, float y, int index) {
  auto* bg = WzResource::ui_->find(u"CashShopUI.img/saleEntry/backgrnd");
  if (bg != nullptr) {
    auto* t = WzResource::LoadTexture(bg);
    SDL_FRect r{x, y, static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(Window::renderer_, t, nullptr, &r);
  }
  auto* icon = g_commodity_icons[index];
  if (icon != nullptr) {
    // 图标放大 2 倍，居中在卡片左侧图标区（x 6..86）。
    constexpr float kIconBoxX = 6.0f;
    constexpr float kIconBoxW = 80.0f;
    constexpr float kIconBoxH = kCardH;
    float w = static_cast<float>(icon->w) * 2.0f;
    float h = static_cast<float>(icon->h) * 2.0f;
    // 超出图标区时等比收缩到刚好放下。
    float fit = std::min({1.0f, kIconBoxW / w, kIconBoxH / h});
    w *= fit;
    h *= fit;
    SDL_FRect r{x + kIconBoxX + (kIconBoxW - w) / 2, y + (kIconBoxH - h) / 2, w,
                h};
    SDL_RenderTexture(Window::renderer_, icon, nullptr, &r);
  }
  // 价格
  Freetype::LoadAligned(true);
  Freetype::LoadSize(12);
  Freetype::LoadColor(30, 30, 30, 255);
  Freetype::DrawLine(NumToU16(g_commodities[index].price), x + 92, y + 40);
  // 加入购物车勾选框
  auto* chk = WzResource::ui_->find(
      g_commodity_selected[index]
          ? u"CashShopUI.img/saleEntry/chkToCart/checked"
          : u"CashShopUI.img/saleEntry/chkToCart/unchecked");
  if (chk != nullptr) {
    auto* t = WzResource::LoadTexture(chk);
    SDL_FRect r{x + 62, y + 9, static_cast<float>(t->w),
                static_cast<float>(t->h)};
    SDL_RenderTexture(Window::renderer_, t, nullptr, &r);
  }
  // 购买按钮（带 hover）
  RenderCardBuyButton(x, y);
}

// 把当前 tab 的商品以 saleEntry 卡片铺进右侧 entry 区，带分页。
void RenderSaleList() {
  LoadCommodities();
  auto filtered = FilterCommodities();
  int total = static_cast<int>(filtered.size());
  int pages = std::max(1, (total + kPageSize - 1) / kPageSize);
  g_page = std::clamp(g_page, 0, pages - 1);
  int start = g_page * kPageSize;
  for (int slot = 0; slot < kPageSize; ++slot) {
    int idx = start + slot;
    if (idx >= total) {
      break;
    }
    int col = slot % kCardCols;
    int row = slot / kCardCols;
    float x =
        g_cash_shop_pos.x + kEntryX + static_cast<float>(col) * kCardStrideX;
    float y =
        g_cash_shop_pos.y + kEntryY + static_cast<float>(row) * kCardStrideY;
    RenderSaleCard(x, y, filtered[idx]);
  }
  // 页码
  Freetype::LoadAligned(true);
  Freetype::LoadSize(12);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(NumToU16(g_page + 1) + u"/" + NumToU16(pages),
                     g_cash_shop_pos.x + 800, g_cash_shop_pos.y + 642);
}

// 购物车摘要（数量 / 合计），画在右侧列表区下方。
void RenderCartInfo() {
  Freetype::LoadAligned(true);
  Freetype::LoadSize(16);
  Freetype::LoadColor(255, 255, 255, 255);
  std::u16string text =
      u"CART " + NumToU16(g_cart_count) + u"   TOTAL " + NumToU16(g_cart_price);
  Freetype::DrawLine(text, g_cash_shop_pos.x + 480, g_cash_shop_pos.y + 600);
}

// 点击：切 tab / 翻页 / 勾选商品加入购物车。
void HandleCashShopClick(float mx, float my) {
  SDL_FPoint p{mx, my};
  // 右下角 Exit：退出商城（恢复心跳 + 回到原地图）。
  SDL_FRect exit_rect{g_cash_shop_pos.x + 1032.0f, g_cash_shop_pos.y + 707.0f,
                      67.0f, 36.0f};
  if (SDL_PointInRectFloat(&p, &exit_rect)) {
    CashShopUiSystem::Close();
    return;
  }
  // 分类 tab
  for (int i = 0; i < kTabCount; ++i) {
    SDL_FRect r{g_cash_shop_pos.x + kTabX + static_cast<float>(i) * kTabStride,
                g_cash_shop_pos.y + kTabY, kTabStride, 47.0f};
    if (SDL_PointInRectFloat(&p, &r)) {
      g_active_tab = i;
      g_page = 0;
      return;
    }
  }
  // 翻页：page 区左右两半
  SDL_FRect prev_rect{g_cash_shop_pos.x + 752, g_cash_shop_pos.y + 636, 80, 30};
  SDL_FRect next_rect{g_cash_shop_pos.x + 831, g_cash_shop_pos.y + 636, 80, 30};
  if (SDL_PointInRectFloat(&p, &prev_rect)) {
    if (g_page > 0) {
      g_page--;
    }
    return;
  }
  if (SDL_PointInRectFloat(&p, &next_rect)) {
    g_page++;
    return;
  }
  // 商品卡片：点选/取消加入购物车
  auto filtered = FilterCommodities();
  int total = static_cast<int>(filtered.size());
  int start = g_page * kPageSize;
  for (int slot = 0; slot < kPageSize; ++slot) {
    int idx = start + slot;
    if (idx >= total) {
      break;
    }
    int col = slot % kCardCols;
    int row = slot / kCardCols;
    SDL_FRect r{
        g_cash_shop_pos.x + kEntryX + static_cast<float>(col) * kCardStrideX,
        g_cash_shop_pos.y + kEntryY + static_cast<float>(row) * kCardStrideY,
        kCardW, kCardH};
    if (SDL_PointInRectFloat(&p, &r)) {
      int ci = filtered[idx];
      if (ci < static_cast<int>(g_commodity_selected.size())) {
        g_commodity_selected[ci] = !g_commodity_selected[ci];
      }
      RecomputeCart();
      return;
    }
  }
}

// 右侧主内容面板：底图 + 分类 tab + 分类条 + 搜索框 + 购物车结算条 + 功能按钮。
void RenderContextPanel() {
  RenderPanel(u"CashShopUI.img/context/backgrnd");
  RenderTabs();
  // 分类条随当前 tab 切换（0..6，超出回退 0）。
  RenderPanel(u"CashShopUI.img/context/categoryBar/" +
              NumToU16(std::min(g_active_tab, 6)));
  RenderPanel(u"CashShopUI.img/context/layer:searchBackgrnd");
  RenderPanel(u"CashShopUI.img/context/canvas:cartCheckOut");
  RenderButton(u"CashShopUI.img/context/button:cartBuy");
  RenderButton(u"CashShopUI.img/context/button:cartDelete");
  RenderButton(u"CashShopUI.img/context/button:chargeCash");
  RenderButton(u"CashShopUI.img/context/button:checkCash");
  RenderButton(u"CashShopUI.img/context/button:coupon");
  RenderButton(u"CashShopUI.img/context/button:exit");
  RenderButton(u"CashShopUI.img/context/button:help");
  RenderButton(u"CashShopUI.img/context/button:probability");
}

// 淡出到黑屏后真正切到现金商店场景。
void OpenImmediate() {
  // 进入商城后断开心跳包。
  ServerHeartbeatInstance::client_started = false;

  // 通知服务端把人物移出当前地图：map_id=0 + fade=false 会走
  // ServerSceneInstance::CleanClient，把本客户端移出地图场景并广播
  // ServerCharacterOut（其他人不再看到这个角色）。
  fbs::ClientSceneT leave_scene;
  leave_scene.fade = false;
  leave_scene.map_id = 0;
  ClientRequest::SendToHost(leave_scene);

  // 预览区用屏幕坐标，相机归零，避免角色被地图相机偏移。
  CameraGameInstance::camera_.x = 0;
  CameraGameInstance::camera_.y = 0;

  // 把角色放到预览脚点中央，站立待机。
  auto& self = CharacterGameInstance::self_;
  self.pos.x = (g_preview_left + g_preview_right) / 2;
  self.pos.y = g_preview_floor;
  self.flip = 0;
  self.face.action = u"default";
  self.face.index = 0;
  self.action_animate = true;
  // 预览里不显示椅子/墓碑/技能残留/聊天气泡。
  self.chair = std::nullopt;
  self.tomb = std::nullopt;
  self.skill = std::nullopt;
  self.effect.clear();
  self.chatballoon = std::nullopt;
  CharacterLogicSystem::RunStandAction(self);

  // 暂停地图逻辑，只跑试穿走位 + 光标逻辑（动画/按下反馈）。
  System::logic_systems_ = {CashShopUiSystem::Run, CursorLogicSystem::Run};
  System::render_systems_ = {CashShopUiSystem::Render,
                             CursorRenderSystem::Render};
  System::event_systems_ = {CashShopUiSystem::Event,
                            KeyboardInputSystem::Event};
  KeyboardInputSystem::Reset();

  // 每次进入重置分类/页码/购物车。
  g_active_tab = 0;
  g_page = 0;
  g_cart_count = 0;
  g_cart_price = 0;
  g_commodity_selected.assign(g_commodities.size(), false);

  g_cash_shop_visible = true;
  g_cash_shop_fading = false;

  // 本次回调在 FadeSystemInstance::Run 内执行，随后会移除一对 fade Run/Render，
  // 因此这里推两次，保证淡入仍有一对生效。
  FadeSystemInstance::EnterOut();
  FadeSystemInstance::EnterOut();
}

}  // namespace

bool CashShopUiSystem::Run() {
  auto& self = CharacterGameInstance::self_;
  const float speed =
      140.0f * static_cast<float>(Window::delta_time_) / 1000.0f;
  const bool left =
      CharacterLogicSystem::character_action_input_.contains("left");
  const bool right =
      CharacterLogicSystem::character_action_input_.contains("right");
  if (left && !right) {
    self.pos.x -= speed;
    self.flip = 0;
    CharacterLogicSystem::RunWalkAction(self);
  } else if (right && !left) {
    self.pos.x += speed;
    self.flip = 1;
    CharacterLogicSystem::RunWalkAction(self);
  } else {
    CharacterLogicSystem::RunStandAction(self);
  }
  self.pos.x =
      std::clamp(self.pos.x, g_preview_left + 20.0f, g_preview_right - 20.0f);
  self.pos.y = g_preview_floor;
  self.action_animate = true;
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
  // 直接渲染真实角色（已在预览脚点上随输入移动）。
  CharacterRenderSystem::RenderCharacter(CharacterGameInstance::self_);
}

bool CashShopUiSystem::Render() {
  RenderBackgrnd();
  // 左侧纵向布局：试穿窗 -> 衣帽间 -> 背包格。
  RenderPanel(u"CashShopUI.img/preview/backgrnd");
  RenderPanel(u"CashShopUI.img/cashLocker/backgrnd");
  RenderPanel(u"CashShopUI.img/itemInven/backgrnd");
  RenderContextPanel();
  RenderSaleList();
  RenderCartInfo();
  RenderAvatar();
  return true;
}

bool CashShopUiSystem::Event(SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      if (event->key.scancode == SDL_SCANCODE_ESCAPE) {
        Close();
        return true;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      // 底图全屏覆盖，吞掉点击避免误触后面的地图 UI。
      return true;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        HandleCashShopClick(Window::mouse_pos_.x, Window::mouse_pos_.y);
      }
      return true;
    }
    default: {
      break;
    }
  }
  return false;
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

  // 记录离人物最近的 sp 出生点，退出时回到该位置。
  g_return_pos = FindNearestSp(CharacterGameInstance::self_.pos);

  auto wh = LoadWh();
  const auto& camera = CameraGameInstance::camera_;
  g_cash_shop_pos.x = (camera.w - wh.x) / 2;
  g_cash_shop_pos.y = (camera.h - wh.y) / 2;
  LoadPreviewRange();

  g_cash_shop_fading = true;
  FadeSystemInstance::EnterIn([] { OpenImmediate(); });
}

void CashShopUiSystem::Close() {
  if (!g_cash_shop_visible) {
    return;
  }
  g_cash_shop_visible = false;
  // 走正常地图重进握手：客户端重新登记到服务端场景（怪物/其它玩家恢复），
  // 并回到进入商城前的地图位置。心跳由 SceneSystemInstance::Enter 恢复。
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