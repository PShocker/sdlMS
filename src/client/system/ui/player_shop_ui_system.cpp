#include "src/client/system/ui/player_shop_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/freetype/freetype.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_main.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>

namespace sdlms {

namespace {

std::u16string U16FromNum(uint64_t num) {
  auto s = std::to_string(num);
  return {s.begin(), s.end()};
}

// ===== PlayerShop.img/Store 布局（窗口 527x468）=====
constexpr float kWinW = 527;
constexpr float kWinH = 468;
// 标题（vector:title (21,9)）。
constexpr float kTitleX = 21;
constexpr float kTitleY = 9;
// 摊主：名牌 (13,134)、立绘 (55,128)、金币 (131,136)。
constexpr float kOwnerTagX = 13;
constexpr float kOwnerAvX = 55;
constexpr float kOwnerAvY = 128;
constexpr float kMesoX = 131;
constexpr float kMesoY = 136;
// 顾客位：名牌1 (242,134)、立绘1 (290,130)。
constexpr float kGuestTagX = 242;
constexpr float kGuestAvX = 290;
constexpr float kGuestAvY = 130;
// 商品列表：entitylist (6,157)-(202,398)，行高 35（ItemEntry）。
constexpr float kListX = 6;
constexpr float kListY = 157;
constexpr float kListW = 196;
constexpr float kRowH = 35;
constexpr int kRowsPerPage = 6;
constexpr float kScrollX = 211;
constexpr float kScrollLen = 237;
// 聊天区：chatlog (244,165)-(492,367)；输入 edit:chatinput (240,387)-(451,400)。
constexpr float kChatX = 244;
constexpr float kChatY = 165;
constexpr float kChatW = 248;
constexpr float kChatH = 202;
constexpr float kChatInX = 240;
constexpr float kChatInY = 387;
constexpr float kChatInW = 211;
constexpr float kChatInH = 13;
// 手续费/提示区 feeText (35,421)-(226,449)。
constexpr float kFeeX = 35;
constexpr float kFeeY = 421;
// 底部按钮（WZ 无坐标，自定一排）。
constexpr SDL_FRect kBtnMain = {240, 419, 80, 18};   // storeopen/buy/storeclose
constexpr SDL_FRect kBtnSold = {330, 419, 80, 18};
constexpr SDL_FRect kBtnLeave = {420, 419, 80, 18};  // leave/leave_host
// ToSell 上架弹窗（235x145）：数量/价格输入 + OK/Cancel。
constexpr float kSellW = 235;
constexpr float kSellH = 145;
constexpr float kSellNumX = 123;
constexpr float kSellNumY = 34;
constexpr float kSellPriceY = 54;
constexpr float kSellEditW = 101;
constexpr float kSellEditH = 12;
constexpr SDL_FRect kSellOk = {60, 113, 47, 18};
constexpr SDL_FRect kSellCancel = {128, 113, 47, 18};
// SoldItems 已售弹窗（235x278）：日志区 (5,31)-(207,264)。
constexpr float kSoldW = 235;
constexpr float kSoldH = 278;
constexpr float kSoldLogX = 5;
constexpr float kSoldLogY = 31;
constexpr float kSoldRowH = 49;
constexpr int kSoldRows = 4;

// 背包可上架物品列表（Etc/Consume/Install 栏，排除小游戏桌）。
std::vector<std::pair<std::u16string, uint32_t>> LoadSellableItems() {
  std::vector<std::pair<std::u16string, uint32_t>> items;
  for (int tab : {(int)ItemEnum::kConsume, (int)ItemEnum::kEtc,
                  (int)ItemEnum::kInstall}) {
    for (auto& itm : PackageGameInstance::data_[tab]) {
      if (itm->id.empty() || itm->id.starts_with(u"0408")) {
        continue;
      }
      items.push_back({itm->id, ItemGameInstance::LoadItemNum(itm)});
    }
  }
  return items;
}

// 通用立绘渲染（095 GetActionPosition：摊主/顾客站位）。
void RenderStandCharacter(GameCharacter g, float x, float y, bool flip) {
  CharacterLogicSystem::RunStandAction(g);
  g.face.action = u"default";
  g.face.index = 0;
  g.action_index = 0;
  g.flip = flip ? 1 : 0;
  const auto& camera = CameraGameInstance::camera_;
  g.pos = {x + camera.x, y + camera.y};
  CharacterRenderSystem::RenderCharacter(g);
}

}  // namespace

SDL_FPoint PlayerShopUiSystem::LoadWh() {
  return {kWinW, kWinH};
}

bool PlayerShopUiSystem::CursorIn() {
  SDL_FRect pos_rect{pos_.x, pos_.y, kWinW, kWinH};
  return SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect);
}

void PlayerShopUiSystem::AddChat(const std::u16string& line) {
  chat_log_.push_back(line);
  if (chat_log_.size() > 100) {
    chat_log_.erase(chat_log_.begin());
  }
}

void PlayerShopUiSystem::OpenSetup() {
  if (mode_ == Mode::kSelling) {
    return;
  }
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it == System::render_systems_.end()) {
    return;
  }
  auto& camera = CameraGameInstance::camera_;
  pos_.x = static_cast<float>((camera.w - kWinW) / 2);
  pos_.y = static_cast<float>((camera.h - kWinH) / 2);
  goods_.clear();
  my_goods_.clear();
  chat_log_.clear();
  sold_log_.clear();
  active_pkg_ = -1;
  active_goods_ = -1;
  goods_page_ = 0;
  pkg_page_ = 0;
  sold_open_ = false;
  sell_edit_active_ = false;
  income_ = 0;
  mode_ = Mode::kSetup;
  chat_input_.Close();
  chat_input_ = TextInput{
      .max_size = 40,
      .text = u"",
      .composition = u"",
      .disable = false,
      .active = false,
      .r = SDL_Rect{static_cast<int>(pos_.x + kChatInX),
                    static_cast<int>(pos_.y + kChatInY),
                    static_cast<int>(kChatInW),
                    static_cast<int>(kChatInH)},
      .pos = {0, 0},
      .font_color = {0, 0, 0, 255},
      .cur_color = {0, 0, 0, 255},
      .font_size = 11,
  };
  System::render_systems_.insert(it, Render);
  System::event_systems_.insert(System::event_systems_.begin(), Event);
}

void PlayerShopUiSystem::OpenBrowse(uint64_t owner) {
  if (owner == ServerMain::local_addr_) {
    OpenSetup();
    return;
  }
  SendStep(1, owner);
}

void PlayerShopUiSystem::Close() {
  if (mode_ == Mode::kBrowsing) {
    // 顾客离店（095 OnLeave）。
    SendStep(3, owner_id_);
  }
  chat_input_.Close();
  sell_num_input_.Close();
  sell_price_input_.Close();
  sell_edit_active_ = false;
  sold_open_ = false;
  mode_ = Mode::kIdle;
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  if (std::ranges::find(System::event_systems_, &KeyboardInputSystem::Event) ==
      System::event_systems_.end()) {
    System::event_systems_.push_back(KeyboardInputSystem::Event);
  }
}

void PlayerShopUiSystem::Toggle() {
  if (mode_ == Mode::kIdle) {
    OpenSetup();
  } else {
    EventButtonLeave();
  }
}

void PlayerShopUiSystem::SendStep(uint8_t step, uint64_t to_id, uint8_t x,
                                  uint32_t y) {
  fbs::ClientCharacterShopT m;
  m.step = step;
  m.to_id = to_id;
  m.x = x;
  m.y = y;
  ClientRequest::SendToHost(m);
}

void PlayerShopUiSystem::RenderBackgrnd() {
  static auto texture = WzResource::LoadTexture(
      WzResource::ui_->find(u"PlayerShop.img/Store/backgrnd"));
  SDL_FRect pos_rect{pos_.x, pos_.y, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

void PlayerShopUiSystem::RenderTitle() {
  Freetype::LoadSize(12);
  Freetype::LoadAligned(true);
  Freetype::LoadBold(false);
  Freetype::LoadColor(255, 255, 255, 255);
  std::u16string title;
  if (mode_ == Mode::kSetup) {
    title = u"个人商店 - 设置货架";
  } else {
    title = stall_title_;
  }
  Freetype::DrawLine(u"[" + title + u"]", pos_.x + kTitleX, pos_.y + kTitleY);
}

void PlayerShopUiSystem::RenderOwner() {
  static auto name_tag = WzResource::LoadTexture(
      WzResource::ui_->find(u"PlayerShop.img/Store/canvas:nameTag0"));
  // 名牌（nameTagPos0）。
  std::u16string name;
  if (mode_ == Mode::kBrowsing) {
    if (CharacterGameInstance::others_.contains(owner_id_)) {
      name = CharacterGameInstance::others_[owner_id_].g_character.name;
    }
  } else {
    name = CharacterGameInstance::self_.name;
  }
  SDL_FRect r{pos_.x + kOwnerTagX, pos_.y + 134,
              static_cast<float>(name_tag->w),
              static_cast<float>(name_tag->h)};
  SDL_RenderTexture(Window::renderer_, name_tag, nullptr, &r);
  Freetype::LoadSize(11);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(name, r.x + 6, r.y + 3);

  // 立绘（avatar0，摊主朝右面向顾客）。
  if (mode_ == Mode::kBrowsing) {
    if (CharacterGameInstance::others_.contains(owner_id_)) {
      RenderStandCharacter(
          CharacterGameInstance::others_[owner_id_].g_character,
          pos_.x + kOwnerAvX, pos_.y + kOwnerAvY, false);
    }
  } else {
    RenderStandCharacter(CharacterGameInstance::self_, pos_.x + kOwnerAvX,
                         pos_.y + kOwnerAvY, false);
  }

  // 金币（mesoText：显示自己余额，与商店一致）。
  Freetype::LoadSize(11);
  Freetype::LoadColor(255, 255, 255, 255);
  auto meso =
      TextGameInstance::FormatWithCommas(PackageGameInstance::meso_);
  std::u16string meso2{meso.begin(), meso.end()};
  Freetype::DrawLine(meso2 + u" mesos", pos_.x + kMesoX, pos_.y + kMesoY);
}

void PlayerShopUiSystem::RenderVisitor() {
  // 顾客位 1（avatar1/nameTag1）：浏览者自己（仅浏览模式）。
  if (mode_ != Mode::kBrowsing) {
    return;
  }
  static auto name_tag = WzResource::LoadTexture(
      WzResource::ui_->find(u"PlayerShop.img/Store/canvas:nameTag1"));
  SDL_FRect r{pos_.x + kGuestTagX, pos_.y + 134,
              static_cast<float>(name_tag->w),
              static_cast<float>(name_tag->h)};
  SDL_RenderTexture(Window::renderer_, name_tag, nullptr, &r);
  Freetype::LoadSize(11);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(CharacterGameInstance::self_.name, r.x + 6, r.y + 3);
  // 顾客朝左面向摊主。
  RenderStandCharacter(CharacterGameInstance::self_, pos_.x + kGuestAvX,
                      pos_.y + kGuestAvY, true);
}

void PlayerShopUiSystem::RenderGoodsList() {
  static auto selected = WzResource::LoadTexture(
      WzResource::ui_->find(u"PlayerShop.img/Store/ItemEntry/selected"));
  static auto soldout = WzResource::LoadTexture(
      WzResource::ui_->find(u"PlayerShop.img/Store/ItemEntry/soldout"));
  static auto meso_icon = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIShop.img/Shop/meso"));
  const auto& list = (mode_ == Mode::kSelling) ? my_goods_ : goods_;

  Freetype::LoadSize(11);
  Freetype::LoadAligned(true);
  Freetype::LoadBold(false);

  for (int i = 0; i < kRowsPerPage; ++i) {
    int index = goods_page_ + i;
    if (index >= static_cast<int>(list.size())) {
      break;
    }
    const auto& g = list[index];
    float y = pos_.y + kListY + i * kRowH;
    // 选中高亮（ItemEntry/selected 162x35）。
    if (active_goods_ == index) {
      SDL_FRect r{pos_.x + 4, y, static_cast<float>(selected->w),
                  static_cast<float>(selected->h)};
      SDL_RenderTexture(Window::renderer_, selected, nullptr, &r);
    }
    // 售罄盖图（ItemEntry/soldout 199x35）。
    if (g.num == 0) {
      SDL_FRect r{pos_.x + 4, y, static_cast<float>(soldout->w),
                  static_cast<float>(soldout->h)};
      SDL_RenderTexture(Window::renderer_, soldout, nullptr, &r);
    }
    // 图标。
    auto info = ItemGameInstance::LoadItemInfo(g.item_id, 0);
    if (info != nullptr) {
      auto* texture = WzResource::LoadTexture(info->get_child(u"icon"));
      if (texture != nullptr) {
        SDL_FRect r{pos_.x + 12 + (32 - texture->w) / 2,
                    y + (kRowH - texture->h) / 2,
                    static_cast<float>(texture->w),
                    static_cast<float>(texture->h)};
        SDL_RenderTexture(Window::renderer_, texture, nullptr, &r);
      }
    }
    // 名字（ItemEntry posItemName）。
    Freetype::LoadColor(0, 0, 0, 255);
    std::u16string name = ItemGameInstance::LoadItemText(g.item_id, u"name");
    if (name.size() > 8) {
      name = name.substr(0, 8) + u"..";
    }
    Freetype::DrawLine(name, pos_.x + 48, y + 4);
    // 价格 x 数量（ItemEntry posPrice）。
    SDL_FRect mr{pos_.x + 48, y + 21, static_cast<float>(meso_icon->w),
                 static_cast<float>(meso_icon->h)};
    SDL_RenderTexture(Window::renderer_, meso_icon, nullptr, &mr);
    Freetype::DrawLine(U16FromNum(g.price) + u" x" + U16FromNum(g.num),
                       mr.x + 14, mr.y - 3);
  }
}

void PlayerShopUiSystem::RenderChatPane() {
  Freetype::LoadSize(11);
  Freetype::LoadAligned(true);
  Freetype::LoadBold(false);

  if (mode_ == Mode::kSetup) {
    // 设置模式：聊天区显示背包可售物品（点击弹 ToSell 上架）。
    auto items = LoadSellableItems();
    static auto meso_icon = WzResource::LoadTexture(
        WzResource::ui_->find(u"UIShop.img/Shop/meso"));
    for (int i = 0; i < kRowsPerPage; ++i) {
      int index = pkg_page_ + i;
      if (index >= static_cast<int>(items.size())) {
        break;
      }
      const auto& [id, num] = items[index];
      float y = pos_.y + kChatY + i * (kRowH + 5);
      if (active_pkg_ == index) {
        SDL_SetRenderDrawColor(Window::renderer_, 255, 240, 180, 90);
        SDL_FRect hl{pos_.x + kChatX, y - 2, kChatW, kRowH};
        SDL_RenderFillRect(Window::renderer_, &hl);
      }
      auto info = ItemGameInstance::LoadItemInfo(id, 0);
      if (info != nullptr) {
        auto* texture = WzResource::LoadTexture(info->get_child(u"icon"));
        if (texture != nullptr) {
          SDL_FRect r{pos_.x + kChatX + 6 + (32 - texture->w) / 2,
                      y + (kRowH - texture->h) / 2,
                      static_cast<float>(texture->w),
                      static_cast<float>(texture->h)};
          SDL_RenderTexture(Window::renderer_, texture, nullptr, &r);
        }
      }
      Freetype::LoadColor(0, 0, 0, 255);
      std::u16string name = ItemGameInstance::LoadItemText(id, u"name");
      if (name.size() > 8) {
        name = name.substr(0, 8) + u"..";
      }
      Freetype::DrawLine(name, pos_.x + kChatX + 44, y + 4);
      Freetype::DrawLine(u"x" + U16FromNum(num), pos_.x + kChatX + 44,
                         y + 21);
      // 已加入货架的显示当前价格。
      for (const auto& g : goods_) {
        if (g.item_id == id) {
          SDL_FRect mr{pos_.x + kChatX + 110, y + 21,
                       static_cast<float>(meso_icon->w),
                       static_cast<float>(meso_icon->h)};
          SDL_RenderTexture(Window::renderer_, meso_icon, nullptr, &mr);
          Freetype::DrawLine(U16FromNum(g.price), mr.x + 14, mr.y - 3);
          break;
        }
      }
    }
    return;
  }

  // 浏览/开张：店内消息（最近 12 行）。
  Freetype::LoadColor(0, 0, 0, 255);
  constexpr int kChatMaxLines = 13;
  int begin = std::max(0, static_cast<int>(chat_log_.size()) - kChatMaxLines);
  for (int i = begin; i < static_cast<int>(chat_log_.size()); ++i) {
    Freetype::DrawLine(chat_log_[i], pos_.x + kChatX + 4,
                       pos_.y + kChatY + (i - begin) * 15);
  }
}

void PlayerShopUiSystem::RenderFeeText() {
  // feeText 区（095：手续费说明位）：摊主显示收入，顾客显示提示。
  Freetype::LoadSize(11);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(0, 0, 0, 255);
  if (mode_ == Mode::kSelling) {
    auto income = TextGameInstance::FormatWithCommas(income_);
    std::u16string inc{income.begin(), income.end()};
    Freetype::DrawLine(u"本次摆摊收入: " + inc + u" mesos",
                       pos_.x + kFeeX, pos_.y + kFeeY + 4);
  } else if (mode_ == Mode::kBrowsing) {
    Freetype::DrawLine(u"选中商品后点击购买按钮。", pos_.x + kFeeX,
                       pos_.y + kFeeY + 4);
  } else {
    Freetype::DrawLine(u"点击右侧物品上架，然后开店。", pos_.x + kFeeX,
                       pos_.y + kFeeY + 4);
  }
}

void PlayerShopUiSystem::RenderSellEdit() {
  if (!sell_edit_active_) {
    return;
  }
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"PlayerShop.img/ToSell/backgrnd"));
  // 居中于主窗。
  float x = pos_.x + (kWinW - kSellW) / 2;
  float y = pos_.y + (kWinH - kSellH) / 2;
  SDL_FRect r{x, y, kSellW, kSellH};
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &r);

  Freetype::LoadSize(11);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(0, 0, 0, 255);
  // 物品图标与名字。
  auto info = ItemGameInstance::LoadItemInfo(sell_item_id_, 0);
  if (info != nullptr) {
    auto* texture = WzResource::LoadTexture(info->get_child(u"icon"));
    if (texture != nullptr) {
      SDL_FRect ir{x + 18, y + 12, static_cast<float>(texture->w),
                   static_cast<float>(texture->h)};
      SDL_RenderTexture(Window::renderer_, texture, nullptr, &ir);
    }
  }
  std::u16string name = ItemGameInstance::LoadItemText(sell_item_id_, u"name");
  if (name.size() > 8) {
    name = name.substr(0, 8) + u"..";
  }
  Freetype::DrawLine(name, x + 56, y + 16);
  // 输入标签。
  Freetype::DrawLine(u"数量:", x + 68, y + kSellNumY + 2);
  Freetype::DrawLine(u"单价:", x + 68, y + kSellPriceY + 2);
  // 输入框。
  SDL_SetRenderDrawColor(Window::renderer_, 255, 255, 255, 255);
  SDL_FRect nr{x + kSellNumX, y + kSellNumY, kSellEditW, kSellEditH};
  SDL_RenderFillRect(Window::renderer_, &nr);
  SDL_FRect pr{x + kSellNumX, y + kSellPriceY, kSellEditW, kSellEditH};
  SDL_RenderFillRect(Window::renderer_, &pr);
  sell_num_input_.Render();
  sell_price_input_.Render();
}

void PlayerShopUiSystem::RenderSoldList() {
  if (!sold_open_) {
    return;
  }
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"PlayerShop.img/SoldItems/backgrnd"));
  static auto meso_icon = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIShop.img/Shop/meso"));
  float x = pos_.x + (kWinW - kSoldW) / 2;
  float y = pos_.y + (kWinH - kSoldH) / 2;
  SDL_FRect r{x, y, kSoldW, kSoldH};
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &r);

  Freetype::LoadSize(11);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(0, 0, 0, 255);
  Freetype::DrawLine(u"已售列表", x + kSoldLogX + 8, y + 8);
  // 记录行（entity：icon / line0 名 / line1 价格 / line2 买家）。
  int begin = sold_page_ * kSoldRows;
  for (int i = 0; i < kSoldRows; ++i) {
    int index = begin + i;
    if (index >= static_cast<int>(sold_log_.size())) {
      break;
    }
    const auto& s = sold_log_[index];
    float ry = y + kSoldLogY + 30 + i * kSoldRowH;
    auto info = ItemGameInstance::LoadItemInfo(s.item_id, 0);
    if (info != nullptr) {
      auto* texture = WzResource::LoadTexture(info->get_child(u"icon"));
      if (texture != nullptr) {
        SDL_FRect ir{x + 20, ry + 4, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
        SDL_RenderTexture(Window::renderer_, texture, nullptr, &ir);
      }
    }
    std::u16string name = ItemGameInstance::LoadItemText(s.item_id, u"name");
    if (name.size() > 8) {
      name = name.substr(0, 8) + u"..";
    }
    Freetype::DrawLine(name, x + 44, ry);
    SDL_FRect mr{x + 44, ry + 17, static_cast<float>(meso_icon->w),
                 static_cast<float>(meso_icon->h)};
    SDL_RenderTexture(Window::renderer_, meso_icon, nullptr, &mr);
    Freetype::DrawLine(U16FromNum(s.price) + u" x" + U16FromNum(s.num),
                       mr.x + 14, mr.y - 3);
    Freetype::LoadColor(90, 90, 90, 255);
    Freetype::DrawLine(s.buyer, x + 44, ry + 36);
    Freetype::LoadColor(0, 0, 0, 255);
  }
}

void PlayerShopUiSystem::RenderInputs() {
  // 店内聊天输入（设置模式不显示）。
  if (mode_ != Mode::kSetup && !sell_edit_active_) {
    chat_input_.Render();
  }
}

bool PlayerShopUiSystem::Render() {
  RenderBackgrnd();
  RenderTitle();
  RenderOwner();
  RenderVisitor();
  RenderGoodsList();
  RenderChatPane();
  RenderFeeText();
  RenderInputs();
  RenderUI();
  // 弹窗层最后画。
  RenderSoldList();
  RenderSellEdit();
  return true;
}

bool PlayerShopUiSystem::RenderStalls() {
  // 同图摊位招牌（095：摆摊玩家头顶店名招牌）。
  static auto sign = WzResource::LoadTexture(
      WzResource::ui_->find(u"Minigame.img/Popup/icon_public"));
  const auto& camera = CameraGameInstance::camera_;
  Freetype::LoadSize(11);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(255, 255, 255, 255);
  for (const auto& [client_id, title] : stalls_) {
    const GameCharacter* g = nullptr;
    if (client_id == ServerMain::local_addr_) {
      g = &CharacterGameInstance::self_;
    } else if (CharacterGameInstance::others_.contains(client_id)) {
      g = &CharacterGameInstance::others_[client_id].g_character;
    }
    if (g == nullptr) {
      continue;
    }
    SDL_FRect r{g->pos.x - camera.x - sign->w / 2,
                g->pos.y - camera.y - sign->h - 26,
                static_cast<float>(sign->w), static_cast<float>(sign->h)};
    SDL_RenderTexture(Window::renderer_, sign, nullptr, &r);
    auto w = Freetype::LoadW(title);
    Freetype::DrawLine(title, r.x + r.w / 2 - w / 2, r.y - 3);
  }
  return true;
}

UIComponents& PlayerShopUiSystem::GetComponent() {
  components_.clear();
  auto top = CursorGameInstance::cursor_ui_ == Render &&
             (CursorGameInstance::modal_overlay_ == nullptr ||
              CursorGameInstance::modal_overlay_ == Render);
  auto add_button = [&](const char16_t* path, SDL_FRect rect, bool enable,
                        void (*callback)()) {
    Button b{.node = WzResource::ui_->find(path),
             .rect = rect,
             .enable = enable,
             .focused = top};
    b.rect.x += pos_.x;
    b.rect.y += pos_.y;
    b.callback = callback;
    components_.push_back(b);
  };

  if (sell_edit_active_) {
    // ToSell 弹窗：OK / Cancel。
    add_button(u"PlayerShop.img/ToSell/button:ok", kSellOk, true,
               EventButtonSellOk);
    add_button(u"PlayerShop.img/ToSell/button:cancel", kSellCancel, true,
               EventButtonSellCancel);
    return components_;
  }

  switch (mode_) {
    case Mode::kSetup: {
      add_button(u"PlayerShop.img/Store/button:storeopen", kBtnMain,
                 !goods_.empty(), EventButtonOpen);
      break;
    }
    case Mode::kBrowsing: {
      add_button(u"PlayerShop.img/Store/button:buy", kBtnMain,
                 active_goods_ >= 0, EventButtonBuy);
      add_button(u"PlayerShop.img/Store/button:leave", kBtnLeave, true,
                 EventButtonLeave);
      break;
    }
    case Mode::kSelling: {
      add_button(u"PlayerShop.img/Store/button:storeclose", kBtnMain, true,
                 EventButtonClose);
      add_button(u"PlayerShop.img/Store/button:sold", kBtnSold, true,
                 EventButtonSold);
      add_button(u"PlayerShop.img/Store/button:leave_host", kBtnLeave, true,
                 EventButtonLeave);
      break;
    }
    default: {
      break;
    }
  }
  if (mode_ == Mode::kSelling && !sold_open_) {
    // 已售按钮已加。
  }
  if (mode_ == Mode::kSetup) {
    // 设置模式也给 leave（直接关设置窗）。
    add_button(u"PlayerShop.img/Store/button:leave", kBtnLeave, true,
               EventButtonLeave);
  }
  return components_;
}

bool PlayerShopUiSystem::EventUI(SDL_Event* event) {
  return DispatchButtonClick(GetComponent(), true);
}

void PlayerShopUiSystem::OpenSellEdit(const std::u16string& item_id,
                                      uint32_t max_num) {
  sell_edit_active_ = true;
  sell_item_id_ = item_id;
  sell_max_num_ = max_num;
  sell_edit_focus_ = 0;
  float x = pos_.x + (kWinW - kSellW) / 2;
  float y = pos_.y + (kWinH - kSellH) / 2;
  sell_num_input_.Close();
  sell_num_input_ = TextInput{
      .max_size = 3,
      .text = U16FromNum(max_num),
      .composition = u"",
      .disable = false,
      .active = false,
      .r = SDL_Rect{static_cast<int>(x + kSellNumX),
                    static_cast<int>(y + kSellNumY),
                    static_cast<int>(kSellEditW),
                    static_cast<int>(kSellEditH)},
      .pos = {0, 0},
      .font_color = {0, 0, 0, 255},
      .cur_color = {0, 0, 0, 255},
      .font_size = 11,
  };
  sell_num_input_.type.set(TextInput::kDigit);
  sell_price_input_.Close();
  sell_price_input_ = TextInput{
      .max_size = 9,
      .text = u"100",
      .composition = u"",
      .disable = false,
      .active = false,
      .r = SDL_Rect{static_cast<int>(x + kSellNumX),
                    static_cast<int>(y + kSellPriceY),
                    static_cast<int>(kSellEditW),
                    static_cast<int>(kSellEditH)},
      .pos = {0, 0},
      .font_color = {0, 0, 0, 255},
      .cur_color = {0, 0, 0, 255},
      .font_size = 11,
  };
  sell_price_input_.type.set(TextInput::kDigit);
}

void PlayerShopUiSystem::ApplyOpen() {
  if (goods_.empty()) {
    return;
  }
  fbs::ClientCharacterShopT m;
  m.step = 0;
  auto title = stall_title_.empty() ? u"商店" : stall_title_;
  // 设置模式没输入店名的地方（Store 布局无输入框），默认用角色名+商店。
  title = CharacterGameInstance::self_.name + u"的商店";
  m.chat = {title.begin(), title.end()};
  for (const auto& g : goods_) {
    fbs::ShopGoodsT gg;
    gg.item_id =
        std::stoull(std::string{g.item_id.begin(), g.item_id.end()});
    gg.num = g.num;
    gg.price = g.price;
    m.goods.push_back(std::make_unique<fbs::ShopGoodsT>(gg));
  }
  ClientRequest::SendToHost(m);
}

void PlayerShopUiSystem::EventButtonSellOk() {
  uint32_t num = 0;
  uint64_t price = 0;
  try {
    num = static_cast<uint32_t>(std::stoul(
        std::string{sell_num_input_.text.begin(), sell_num_input_.text.end()}));
    price = std::stoull(std::string{sell_price_input_.text.begin(),
                                    sell_price_input_.text.end()});
  } catch (...) {
    return;
  }
  if (num == 0 || price == 0 || num > sell_max_num_) {
    return;
  }
  // 同物品重复上架：合并覆盖。
  for (auto& g : goods_) {
    if (g.item_id == sell_item_id_) {
      g.num = num;
      g.price = price;
      sell_edit_active_ = false;
      AudioGameInstance::PlayNow(u"UI.img/BtMouseClick");
      return;
    }
  }
  Goods g;
  g.item_id = sell_item_id_;
  g.num = num;
  g.price = price;
  goods_.push_back(g);
  sell_edit_active_ = false;
  AudioGameInstance::PlayNow(u"UI.img/BtMouseClick");
}

void PlayerShopUiSystem::EventButtonSellCancel() {
  sell_edit_active_ = false;
}

void PlayerShopUiSystem::EventButtonOpen() {
  ApplyOpen();
}

void PlayerShopUiSystem::EventButtonBuy() {
  if (mode_ != Mode::kBrowsing || active_goods_ < 0 ||
      active_goods_ >= static_cast<int>(goods_.size())) {
    return;
  }
  const auto& g = goods_[active_goods_];
  if (g.num == 0) {
    NoticeUiSystem::notice_text_ = u"商品已售完。";
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kShopNotice;
    NoticeUiSystem::Open();
    return;
  }
  // 本地校验（095 OnBuyResult 前置）：钱、背包格。
  auto tmp = ItemGameInstance::LoadItem(g.item_id, 1);
  if (PackageGameInstance::meso_ < g.price) {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kShopbuyNoMeso;
    NoticeUiSystem::Open();
    return;
  }
  if (PackageGameInstance::LoadEmptyIndex(tmp).empty()) {
    NoticeUiSystem::OpenNoSpace(tmp->type);
    return;
  }
  // 单件购买（095 BuyItem）。
  SendStep(2, owner_id_, static_cast<uint8_t>(active_goods_), 1);
}

void PlayerShopUiSystem::EventButtonClose() {
  // 关摊（095 storeclose）。
  SendStep(3);
  mode_ = Mode::kIdle;
  stalls_.erase(ServerMain::local_addr_);
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void PlayerShopUiSystem::EventButtonSold() {
  sold_open_ = !sold_open_;
  sold_page_ = 0;
}

void PlayerShopUiSystem::EventButtonLeave() {
  if (mode_ == Mode::kSelling) {
    EventButtonClose();
    return;
  }
  Close();
}

bool PlayerShopUiSystem::Event(SDL_Event* event) {
  if (mode_ == Mode::kIdle) {
    return true;
  }
  bool input_cr = false;
  if (sell_edit_active_) {
    input_cr = sell_num_input_.Event(event) || sell_price_input_.Event(event);
  } else {
    input_cr = chat_input_.Event(event);
  }
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      auto scan_code = event->key.scancode;
      if (scan_code == SDL_SCANCODE_ESCAPE) {
        AudioGameInstance::PlayNow(u"UI.img/BtMouseClick");
        if (sold_open_) {
          sold_open_ = false;
        } else if (sell_edit_active_) {
          sell_edit_active_ = false;
        } else {
          EventButtonLeave();
        }
        r = false;
      } else if (scan_code == SDL_SCANCODE_RETURN) {
        if (sell_edit_active_) {
          // 回车在数量/价格间切换，价格框回车确认。
          if (sell_edit_focus_ == 0) {
            sell_num_input_.Close();
            sell_price_input_.Active();
            sell_edit_focus_ = 1;
          } else {
            EventButtonSellOk();
          }
        } else if (mode_ != Mode::kSetup) {
          // 店内聊天（095 CP 145 同款语义：回车发送）。
          if (chat_input_.active) {
            if (!chat_input_.text.empty()) {
              fbs::ClientCharacterShopT m;
              m.step = 5;
              m.to_id = owner_id_;
              m.chat = {chat_input_.text.begin(), chat_input_.text.end()};
              ClientRequest::SendToHost(m);
              chat_input_.text.clear();
              chat_input_.cur = 0;
              chat_input_.composition.clear();
            }
          } else {
            chat_input_.Active();
            std::erase(System::event_systems_, KeyboardInputSystem::Event);
          }
        } else {
          // 设置模式回车：不处理。
        }
        r = false;
      } else if (scan_code == SDL_SCANCODE_TAB && sell_edit_active_) {
        if (sell_edit_focus_ == 0) {
          sell_num_input_.Close();
          sell_price_input_.Active();
          sell_edit_focus_ = 1;
        } else {
          sell_price_input_.Close();
          sell_num_input_.Active();
          sell_edit_focus_ = 0;
        }
        r = false;
      } else if (sell_edit_active_) {
        r = false;
      } else if (chat_input_.active) {
        r = false;
      }
      break;
    }
    case SDL_EVENT_TEXT_INPUT:
    case SDL_EVENT_TEXT_EDITING: {
      r = false;
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        // 点击输入框外恢复键盘（聊天或上架弹窗）。
        if (input_cr &&
            std::ranges::find(System::event_systems_,
                              &KeyboardInputSystem::Event) ==
                System::event_systems_.end()) {
          System::event_systems_.push_back(KeyboardInputSystem::Event);
        }
        if (sell_edit_active_) {
          // 点击 ToSell 输入框聚焦。
          if (sell_edit_focus_ == 0) {
            sell_price_input_.Close();
            sell_num_input_.Active();
          } else {
            sell_num_input_.Close();
            sell_price_input_.Active();
          }
        }
        if (CursorIn()) {
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT && CursorIn()) {
        if (!EventUI(event)) {
          auto& mouse = Window::mouse_pos_;
          // 货架选中（entitylist 区，095 GetItemIndexFromPoint）。
          SDL_FRect rect{pos_.x + kListX, pos_.y + kListY, kListW,
                         kRowH * kRowsPerPage};
          if (SDL_PointInRectFloat(&mouse, &rect)) {
            int row = static_cast<int>((mouse.y - rect.y) / kRowH);
            int index = goods_page_ + row;
            const auto& list = (mode_ == Mode::kSelling) ? my_goods_ : goods_;
            if (index < static_cast<int>(list.size())) {
              active_goods_ = index;
            }
          }
          // 设置模式：点背包物品弹 ToSell 上架（095 PutItem）。
          if (mode_ == Mode::kSetup && !sell_edit_active_) {
            SDL_FRect prect{pos_.x + kChatX, pos_.y + kChatY, kChatW,
                           kRowH * kRowsPerPage};
            if (SDL_PointInRectFloat(&mouse, &prect)) {
              int row = static_cast<int>((mouse.y - prect.y) / (kRowH + 5));
              int index = pkg_page_ + row;
              auto items = LoadSellableItems();
              if (index < static_cast<int>(items.size()) &&
                  items[index].second > 0) {
                OpenSellEdit(items[index].first, items[index].second);
              }
            }
          }
        }
        r = false;
      }
      break;
    }
    case SDL_EVENT_MOUSE_WHEEL: {
      // 商品列表滚轮（scroll:itemListScroll wheelRange 210）。
      if (CursorIn() && !sell_edit_active_ && !sold_open_) {
        auto& list = (mode_ == Mode::kSelling) ? my_goods_ : goods_;
        int max_page =
            std::max(0, (static_cast<int>(list.size()) - kRowsPerPage));
        if (event->wheel.y > 0 && goods_page_ > 0) {
          --goods_page_;
        } else if (event->wheel.y < 0 && goods_page_ < max_page) {
          ++goods_page_;
        }
      }
      break;
    }
    default: {
      break;
    }
  }
  return r;
}

void PlayerShopUiSystem::HandleServer(uint64_t client_id,
                                      fbs::ServerCharacterShopT& r) {
  switch (r.step) {
    // 开摊成功。
    case 0: {
      mode_ = Mode::kSelling;
      stall_title_ = {r.chat.begin(), r.chat.end()};
      my_goods_.clear();
      for (const auto& g : r.goods) {
        Goods gg;
        auto tmp = std::to_string(g->item_id);
        gg.item_id = {tmp.begin(), tmp.end()};
        gg.num = g->num;
        gg.price = g->price;
        my_goods_.push_back(gg);
      }
      goods_ = my_goods_;
      income_ = 0;
      sold_log_.clear();
      active_goods_ = -1;
      goods_page_ = 0;
      sold_open_ = false;
      chat_log_.clear();
      chat_log_.push_back(u"商店已开张。");
      if (std::ranges::find(System::render_systems_, &Render) ==
          System::render_systems_.end()) {
        auto it = std::ranges::find(System::render_systems_,
                                    &CursorRenderSystem::Render);
        if (it != System::render_systems_.end()) {
          System::render_systems_.insert(it, Render);
          System::event_systems_.insert(System::event_systems_.begin(),
                                        Event);
        }
      }
      stalls_[ServerMain::local_addr_] = stall_title_;
      break;
    }
    // 进店成功。
    case 1: {
      mode_ = Mode::kBrowsing;
      owner_id_ = r.to_id;
      stall_title_ = {r.chat.begin(), r.chat.end()};
      if (r.player != nullptr) {
        owner_char_ = ServerCharacterInstance::LoadGCharacter(r.player->character);
      }
      goods_.clear();
      for (const auto& g : r.goods) {
        Goods gg;
        auto tmp = std::to_string(g->item_id);
        gg.item_id = {tmp.begin(), tmp.end()};
        gg.num = g->num;
        gg.price = g->price;
        goods_.push_back(gg);
      }
      active_goods_ = -1;
      goods_page_ = 0;
      chat_log_.clear();
      chat_log_.push_back(u"欢迎光临 [" + stall_title_ + u"]");
      if (std::ranges::find(System::render_systems_, &Render) ==
          System::render_systems_.end()) {
        auto it = std::ranges::find(System::render_systems_,
                                    &CursorRenderSystem::Render);
        if (it != System::render_systems_.end()) {
          auto& camera = CameraGameInstance::camera_;
          pos_.x = static_cast<float>((camera.w - kWinW) / 2);
          pos_.y = static_cast<float>((camera.h - kWinH) / 2);
          chat_input_.Close();
          chat_input_ = TextInput{
              .max_size = 40,
              .text = u"",
              .composition = u"",
              .disable = false,
              .active = false,
              .r = SDL_Rect{static_cast<int>(pos_.x + kChatInX),
                            static_cast<int>(pos_.y + kChatInY),
                            static_cast<int>(kChatInW),
                            static_cast<int>(kChatInH)},
              .pos = {0, 0},
              .font_color = {0, 0, 0, 255},
              .cur_color = {0, 0, 0, 255},
              .font_size = 11,
          };
          System::render_systems_.insert(it, Render);
          System::event_systems_.insert(System::event_systems_.begin(),
                                        Event);
        }
      }
      break;
    }
    // 购买结果（买家）。
    case 2: {
      if (r.x == 1) {
        int index = static_cast<int>(r.y);
        if (index >= 0 && index < static_cast<int>(goods_.size())) {
          const auto& g = goods_[index];
          auto item = ItemGameInstance::LoadItem(g.item_id, 1);
          if (!item->id.empty()) {
            PackageGameInstance::AddItem(item);
          }
          PackageGameInstance::meso_ -= g.price;
          goods_[index].num = goods_[index].num > 0 ? goods_[index].num - 1 : 0;
          AudioGameInstance::PlayNow(u"UI.img/BtMouseClick");
        }
      } else {
        std::u16string text = (r.y == 0)   ? u"金币不足。"
                              : (r.y == 2) ? u"背包空间不足。"
                                           : u"商品已售完。";
        NoticeUiSystem::notice_text_ = text;
        NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kShopNotice;
        NoticeUiSystem::Open();
      }
      break;
    }
    // 撤摊 / 离店。
    case 3: {
      if (mode_ == Mode::kSelling || mode_ == Mode::kBrowsing) {
        if (mode_ == Mode::kBrowsing) {
          NoticeUiSystem::notice_text_ = u"商店已关闭。";
          NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kShopNotice;
          NoticeUiSystem::Open();
        }
        mode_ = Mode::kIdle;
        stalls_.erase(ServerMain::local_addr_);
        std::erase(System::render_systems_, Render);
        std::erase(System::event_systems_, Event);
      }
      break;
    }
    // 售出通知（摊主，095 OnSoldItemResult）。
    case 4: {
      int index = static_cast<int>(r.x);
      if (index < 0 || index >= static_cast<int>(my_goods_.size())) {
        break;
      }
      if (r.y > 0) {
        my_goods_[index].num =
            my_goods_[index].num >= r.y ? my_goods_[index].num - r.y : 0;
        PackageGameInstance::meso_ += r.meso;
        income_ += r.meso;
        // 已售记录（095 SoldItems）。
        SoldRecord s;
        s.item_id = my_goods_[index].item_id;
        s.num = r.y;
        s.price = my_goods_[index].price;
        s.buyer = u"顾客";
        if (r.to_id != 0 && CharacterGameInstance::others_.contains(r.to_id)) {
          s.buyer = CharacterGameInstance::others_[r.to_id].g_character.name;
        }
        sold_log_.push_back(s);
        AddChat(s.buyer + u" 购买了商品。");
        // 本地扣物品：找背包里的实际槽位。
        auto* slot = PackageGameInstance::LoadItem(my_goods_[index].item_id);
        if (slot != nullptr) {
          ItemGameInstance::DecItemNum(*slot, r.y);
        }
        if (mode_ == Mode::kSelling) {
          goods_ = my_goods_;
        }
        AudioGameInstance::PlayNow(u"UI.img/BtMouseClick");
      }
      break;
    }
    // 摊位广播（同图玩家，095 摆摊招牌可见性）。
    case 5: {
      uint64_t owner = r.to_id;
      if (r.player != nullptr) {
        owner = r.player->client_id;
      }
      if (r.x == 1) {
        stalls_[owner] = {r.chat.begin(), r.chat.end()};
      } else {
        stalls_.erase(owner);
      }
      break;
    }
    // 提示。
    case 6: {
      NoticeUiSystem::notice_text_ = {r.chat.begin(), r.chat.end()};
      NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kShopNotice;
      NoticeUiSystem::Open();
      break;
    }
    // 店内聊天。
    case 7: {
      std::u16string name = u"???";
      if (r.player != nullptr && r.player->character != nullptr) {
        name = {r.player->character->name.begin(),
                r.player->character->name.end()};
      }
      AddChat(name + u" : " +
              std::u16string{r.chat.begin(), r.chat.end()});
      break;
    }
    default: {
      break;
    }
  }
}

}  // namespace sdlms