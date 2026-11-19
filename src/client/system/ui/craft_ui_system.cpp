#include "src/client/system/ui/craft_ui_system.h"

#include "src/client/game/game_item.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/craft_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/component/scroll.h"
#include "src/client/system/ui/component/tab.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system/ui/tooltip_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Property.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <flat_map>
#include <format>
#include <string>
#include <vector>

namespace sdlms {

namespace {

constexpr float kRecipeX = 9.0f;
constexpr float kRecipeY = 124.0f;
constexpr float kRecipeW = 160.0f;
constexpr float kRecipeH = 34.0f;
constexpr float kRecipeScroll =
    269.0f;  // UI/Crafting.img/scroll:recipeScroll/length
constexpr float kRecipeSpan = 235.0f;  // kRecipeScroll - kRecipeH
constexpr float kRecipeRows = 7.0f;

std::u16string Num(uint32_t n) {
  auto s = std::to_string(n);
  return {s.begin(), s.end()};
}

SDL_Texture* GrayIcon(SDL_Texture* source) {
  if (source == nullptr) {
    return nullptr;
  }
  static std::flat_map<SDL_Texture*, SDL_Texture*> cache;
  if (auto it = cache.find(source); it != cache.end()) {
    return it->second;
  }

  float fw = 0;
  float fh = 0;
  if (!SDL_GetTextureSize(source, &fw, &fh)) {
    return source;
  }
  const int w = static_cast<int>(fw);
  const int h = static_cast<int>(fh);
  if (w <= 0 || h <= 0) {
    return source;
  }

  SDL_Texture* target =
      SDL_CreateTexture(Window::renderer_, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_TARGET, w, h);
  if (target == nullptr) {
    return source;
  }
  SDL_SetRenderTarget(Window::renderer_, target);
  SDL_SetRenderDrawColor(Window::renderer_, 0, 0, 0, 0);
  SDL_RenderClear(Window::renderer_);
  SDL_RenderTexture(Window::renderer_, source, nullptr, nullptr);
  SDL_Surface* raw = SDL_RenderReadPixels(Window::renderer_, nullptr);
  SDL_SetRenderTarget(Window::renderer_, nullptr);
  SDL_DestroyTexture(target);
  if (raw == nullptr) {
    return source;
  }

  SDL_Surface* surface = SDL_ConvertSurface(raw, SDL_PIXELFORMAT_ARGB8888);
  SDL_DestroySurface(raw);
  if (surface == nullptr) {
    return source;
  }
  SDL_LockSurface(surface);
  auto* pixels = static_cast<uint32_t*>(surface->pixels);
  const int pitch = surface->pitch / static_cast<int>(sizeof(uint32_t));
  for (int y = 0; y < surface->h; ++y) {
    for (int x = 0; x < surface->w; ++x) {
      uint32_t& p = pixels[y * pitch + x];
      const uint32_t a = p & 0xff000000u;
      const uint32_t r = (p >> 16) & 0xffu;
      const uint32_t g = (p >> 8) & 0xffu;
      const uint32_t b = p & 0xffu;
      const uint32_t gray = (r * 77 + g * 150 + b * 29) >> 8;
      p = a | (gray << 16) | (gray << 8) | gray;
    }
  }
  SDL_UnlockSurface(surface);

  SDL_Texture* result =
      SDL_CreateTextureFromSurface(Window::renderer_, surface);
  SDL_DestroySurface(surface);
  if (result == nullptr) {
    return source;
  }
  cache[source] = result;
  return result;
}
std::u16string ItemId(uint32_t id) {
  auto s = std::format("{:08d}", id);
  return {s.begin(), s.end()};
}

SDL_Texture* ItemIcon(uint32_t id, std::u16string& name, uint32_t& have) {
  auto sid = ItemId(id);
  have = PackageGameInstance::LoadItemNum(sid);
  if (ItemGameInstance::CheckItem(sid)) {
    auto* info = ItemGameInstance::LoadItemInfo(sid, 0);
    name = ItemGameInstance::LoadItemText(sid, u"name");
    return info == nullptr ? nullptr
                           : WzResource::LoadTexture(info->get_child(u"icon"));
  }
  auto* info = EquipGameInstance::LoadEquipInfo(sid);
  name = EquipGameInstance::LoadEquipName(sid);
  return info == nullptr ? nullptr
                         : WzResource::LoadTexture(info->get_child(u"icon"));
}

void Text(const std::u16string& s, float x, float y, int size = 12,
          uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, bool bold = false) {
  Freetype::LoadAligned(true);
  Freetype::LoadSize(size);
  Freetype::LoadBold(bold);
  Freetype::LoadColor(r, g, b, 255);
  Freetype::DrawLine(s, x, y);
}

std::u16string CraftCategoryName(uint8_t tab) {
  static const std::array<const char16_t*, 6> names = {
      u"Smithing",     u"Weaponcrafting", u"Tailoring",
      u"Woodcrafting", u"Leatherworking", u"Arcanforge"};
  return names[std::min<size_t>(tab, names.size() - 1)];
}

std::u16string CraftToolName(uint8_t tab) {
  static const std::array<const char16_t*, 6> names = {
      u"Forge", u"Forge",        u"Tailor Bench",
      u"Saw",   u"Tanning Rack", u"Arcane Station"};
  return names[std::min<size_t>(tab, names.size() - 1)];
}

std::u16string CraftHint(uint8_t tab) {
  if (tab == 5) {
    return u"You can craft them near the Arcane Station in Ellinia and "
           u"Sleepywood.";
  }
  return u"Craft items using the materials in your inventory";
}

float ButtonW(wz::Node* node) {
  auto* texture = WzResource::LoadTexture(node->find(u"normal/0"));
  return texture == nullptr ? 60.0f : static_cast<float>(texture->w);
}

float ButtonH(wz::Node* node) {
  auto* texture = WzResource::LoadTexture(node->find(u"normal/0"));
  return texture == nullptr ? 18.0f : static_cast<float>(texture->h);
}

}  // namespace

SDL_FPoint CraftUiSystem::LoadWh() {
  return {545, 409};
}

void CraftUiSystem::RenderBackgrnd() {
  static auto texture =
      WzResource::LoadTexture(WzResource::ui_->find(u"Crafting.img/backgrnd"));
  SDL_FRect pos_rect{pos_.x, pos_.y, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

std::vector<CraftGameInstance::CraftRecipe> CraftUiSystem::LoadRecipes() {
  static bool recipe_loaded = false;
  if (!recipe_loaded) {
    CraftGameInstance::InitRecipe();
    recipe_loaded = true;
  }
  const uint32_t skill_id = 92000000u + selected_tab_;
  auto recipes = CraftGameInstance::LoadSkillRecipes(skill_id);
  if (recipes.empty()) {
    for (const auto& r : CraftGameInstance::recipes_) {
      if (static_cast<uint8_t>(r.type) == selected_tab_) {
        recipes.push_back(r);
      }
    }
  }
  // 按制作等级要求从低到高排序，同等级保持 WZ 原顺序。
  std::ranges::stable_sort(
      recipes, [](const auto& a, const auto& b) { return a.level < b.level; });
  return recipes;
}

bool CraftUiSystem::LoadRecipeItem(uint32_t item_id, SDL_Texture*& texture,
                                   std::u16string& name, uint32_t& have) {
  texture = ItemIcon(item_id, name, have);
  return texture != nullptr;
}

void CraftUiSystem::RenderLevel() {
  auto& craft = CraftGameInstance::datas_[selected_tab_];
  const int level = std::max(1, static_cast<int>(craft.level));
  const float percent =
      std::clamp(static_cast<float>(craft.exp) / 100.0f, 0.0f, 100.0f);

  static auto gauge0 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Crafting.img/gaugeBar:skill/0"));
  static auto gauge1 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Crafting.img/gaugeBar:skill/1"));
  const int gx = static_cast<int>(pos_.x) + 107;
  const int gy = static_cast<int>(pos_.y) + 60;
  SDL_FRect bar{static_cast<float>(gx), static_cast<float>(gy),
                gauge0 ? static_cast<float>(gauge0->w) : 422.0f,
                gauge0 ? static_cast<float>(gauge0->h) : 14.0f};
  if (gauge0) {
    SDL_RenderTexture(Window::renderer_, gauge0, nullptr, &bar);
  }
  if (gauge1) {
    const int fill_w = static_cast<int>(gauge1->w * percent / 100.0f);
    SDL_FRect src{0, 0, static_cast<float>(fill_w),
                  static_cast<float>(gauge1->h)};
    SDL_FRect dst{static_cast<float>(gx), static_cast<float>(gy),
                  static_cast<float>(fill_w), static_cast<float>(gauge1->h)};
    SDL_RenderTexture(Window::renderer_, gauge1, &src, &dst);
  }
  Text(u"LEVEL " + Num(level), pos_.x + 44, pos_.y + 60, 12);
  auto pct = std::format("{:.2f}%", percent);
  Text({pct.begin(), pct.end()}, pos_.x + 295, pos_.y + 61, 12);
}

void CraftUiSystem::RenderCategory() {
  static const std::array<const char16_t*, 6> icons = {
      u"smithing",     u"weaponcrafting", u"tailoring",
      u"woodcrafting", u"leatherworking", u"arcforge"};
  SDL_Texture* texture = nullptr;
  if (selected_tab_ == 5) {
    // Arcanforge 等级图标使用 UI/_Canvas 下的原始素材。
    if (auto* node =
            WzResource::ui_->find(u"_Canvas/Crafting.img/Icon/arcforge");
        node != nullptr) {
      texture = WzResource::LoadTexture(node);
    }
  }
  if (texture == nullptr) {
    auto* node = WzResource::ui_->find(std::u16string(u"Crafting.img/Icon/") +
                                       icons[selected_tab_]);
    if (node != nullptr) {
      texture = WzResource::LoadTexture(node);
    }
  }
  if (texture != nullptr) {
    SDL_FRect rect{pos_.x + 19, pos_.y + 57, static_cast<float>(texture->w),
                   static_cast<float>(texture->h)};
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &rect);
  }
  Text(CraftHint(selected_tab_), pos_.x + 15, pos_.y + 88, 11, 64, 64, 64);
}

void CraftUiSystem::RenderRecipes() {
  const auto recipes = LoadRecipes();
  const auto start = std::min<uint32_t>(recipe_scroll_, recipes.size());
  const auto rows = static_cast<uint32_t>(kRecipeRows);
  const auto end = std::min<uint32_t>(start + rows, recipes.size());

  static auto enabled = WzResource::LoadTexture(
      WzResource::ui_->find(u"Crafting.img/ItemEntry/enabled"));
  static auto selected = WzResource::LoadTexture(
      WzResource::ui_->find(u"Crafting.img/ItemEntry/selected"));
  static auto disabled = WzResource::LoadTexture(
      WzResource::ui_->find(u"Crafting.img/ItemEntry/disabled"));
  static auto unlock = WzResource::LoadTexture(
      WzResource::ui_->find(u"Crafting.img/Icon/unlock"));

  hover_recipe_ = std::nullopt;
  for (uint32_t i = start; i < end; ++i) {
    const auto row = i - start;
    const int x = pos_.x + kRecipeX;
    const int y =
        pos_.y + kRecipeY + static_cast<float>(static_cast<int>(row) * 235 / 6);
    auto& recipe = recipes[i];
    auto* bg = (i == selected_recipe_) ? selected : enabled;
    const int craft_level =
        std::max(1, (int)CraftGameInstance::datas_[selected_tab_].level);
    if (recipe.level > craft_level) {
      bg = disabled;
    }
    if (bg != nullptr) {
      SDL_FRect r{static_cast<float>(x), static_cast<float>(y), (float)bg->w,
                  (float)bg->h};
      SDL_RenderTexture(Window::renderer_, bg, nullptr, &r);
    }
    std::u16string name;
    uint32_t have = 0;
    SDL_Texture* icon = nullptr;
    LoadRecipeItem(recipe.result.item, icon, name, have);
    const bool locked = recipe.level > craft_level;
    const int icon_w = icon != nullptr ? icon->w : 0;
    const int icon_h = icon != nullptr ? icon->h : 0;
    const int icon_x = static_cast<int>(x) + (34 - icon_w) / 2;
    const int icon_y = static_cast<int>(y) + (34 - icon_h) / 2;
    SDL_FRect list_icon{static_cast<float>(icon_x), static_cast<float>(icon_y),
                        static_cast<float>(icon_w), static_cast<float>(icon_h)};
    if (icon != nullptr) {
      SDL_Texture* draw_icon = locked ? GrayIcon(icon) : icon;
      SDL_RenderTexture(Window::renderer_, draw_icon, nullptr, &list_icon);
    }
    if (locked && unlock != nullptr) {
      // 锁以配方格子左上角为基准。
      SDL_FRect lr{static_cast<float>(x), static_cast<float>(y),
                   static_cast<float>(unlock->w),
                   static_cast<float>(unlock->h)};
      SDL_RenderTexture(Window::renderer_, unlock, nullptr, &lr);
    }
    if (name.size() > 18) {
      name = name.substr(0, 17) + u"...";
    }
    Text(name, static_cast<float>(static_cast<int>(x) + 42),
         static_cast<float>(static_cast<int>(y) + 1), 12);
    Text(u"LEVEL " + Num(recipe.level),
         static_cast<float>(static_cast<int>(x) + 42),
         static_cast<float>(static_cast<int>(y) + 19), 11, 80, 80, 80);
    SDL_FRect hit{static_cast<float>(x), static_cast<float>(y), kRecipeW,
                  kRecipeH};
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &hit)) {
      hover_recipe_ = i;
    }
  }
}

void CraftUiSystem::RenderDetail() {
  hover_ingredient_ = std::nullopt;
  const auto recipes = LoadRecipes();
  if (recipes.empty() || selected_recipe_ >= recipes.size()) {
    return;
  }
  const auto& recipe = recipes[selected_recipe_];
  const int craft_level = std::max(
      1, static_cast<int>(CraftGameInstance::datas_[selected_tab_].level));
  if (recipe.level > craft_level) {
    return;
  }
  std::u16string result_name;
  uint32_t have = 0;
  SDL_Texture* result_icon = nullptr;
  LoadRecipeItem(recipe.result.item, result_icon, result_name, have);

  if (result_icon != nullptr) {
    // resultIcon 是图标中心点，按中心居中绘制。
    const int center_x = static_cast<int>(pos_.x) + 218;
    const int center_y = static_cast<int>(pos_.y) + 132;
    const int icon_x = center_x + (34 - result_icon->w) / 2;
    const int icon_y = center_y + (34 - result_icon->h) / 2;
    SDL_FRect r{static_cast<float>(icon_x), static_cast<float>(icon_y),
                static_cast<float>(result_icon->w),
                static_cast<float>(result_icon->h)};
    SDL_RenderTexture(Window::renderer_, result_icon, nullptr, &r);
    PackageUiSystem::RenderNumber(recipe.result.count,
                                  static_cast<int>(pos_.x) + 218,
                                  static_cast<int>(pos_.y) + 156);
  }
  Text(result_name, pos_.x + 268, pos_.y + 131, 14, 255, 255, 255, true);
  Text(u"Craft " + Num(recipe.result.count) + u" " + result_name, pos_.x + 268,
       pos_.y + 150, 11, 225, 225, 225);

  Text(CraftToolName(selected_tab_), pos_.x + 207, pos_.y + 210, 12);

  constexpr float kCellX = 205.0f;
  constexpr float kCellY = 260.0f;
  constexpr float kCellW = 165.0f;
  constexpr float kCellH = 38.0f;
  static auto cell_empty = WzResource::LoadTexture(
      WzResource::ui_->find(u"Crafting.img/ItemEntry/empty"));
  static auto cell_enabled = WzResource::LoadTexture(
      WzResource::ui_->find(u"Crafting.img/ItemEntry/enabled"));
  const int filled_slots =
      std::min<int>(static_cast<int>(recipe.ingredients.size()) + 1, 6);
  for (int s = 0; s < 6; ++s) {
    const int x = pos_.x + kCellX + (s % 2) * kCellW + 3;
    const int y = pos_.y + kCellY + (s / 2) * kCellH + 2;
    auto* texture = s < filled_slots ? cell_enabled : cell_empty;
    if (texture != nullptr) {
      SDL_FRect dst{static_cast<float>(x), static_cast<float>(y),
                    static_cast<float>(texture->w),
                    static_cast<float>(texture->h)};
      SDL_RenderTexture(Window::renderer_, texture, nullptr, &dst);
    }
  }
  int slot = 0;
  auto draw_slot = [&](uint32_t item_id, uint32_t own, uint32_t need,
                       const std::u16string& label) {
    const float x = pos_.x + kCellX + (slot % 2) * kCellW;
    const float y = pos_.y + kCellY + (slot / 2) * kCellH;
    std::u16string name;
    uint32_t item_have = 0;
    SDL_Texture* icon = nullptr;
    if (item_id == 0) {
      static auto meso_icon = WzResource::LoadTexture(
          WzResource::ui_->find(u"Crafting.img/Icon/meso"));
      icon = meso_icon;
      name = u"mesos";
    } else {
      LoadRecipeItem(item_id, icon, name, item_have);
    }
    SDL_FRect cell{x + 3, y + 2, 160, 34};
    if (item_id != 0 && SDL_PointInRectFloat(&Window::mouse_pos_, &cell)) {
      hover_ingredient_ = item_id;
    }
    if (icon != nullptr) {
      const int icon_x = static_cast<int>(x) + 3 + (34 - icon->w) / 2;
      const int icon_y = static_cast<int>(y) + 2 + (34 - icon->h) / 2;
      SDL_FRect ir{static_cast<float>(icon_x), static_cast<float>(icon_y),
                   static_cast<float>(icon->w), static_cast<float>(icon->h)};
      if (item_id == 0) {
        // 金币阴影使用 UI/_Canvas/Item.img/canvas:shadow 原始素材。
        static auto shadow_texture = WzResource::LoadTexture(
            WzResource::ui_->find(u"_Canvas/Item.img/canvas:shadow"));
        if (shadow_texture != nullptr) {
          const int shadow_x = icon_x + (icon->w - shadow_texture->w) / 2;
          const int shadow_y = icon_y + icon->h - shadow_texture->h + 2;
          SDL_FRect shadow{static_cast<float>(shadow_x),
                           static_cast<float>(shadow_y),
                           static_cast<float>(shadow_texture->w),
                           static_cast<float>(shadow_texture->h)};
          SDL_RenderTexture(Window::renderer_, shadow_texture, nullptr,
                            &shadow);
        }
      }
      SDL_RenderTexture(Window::renderer_, icon, nullptr, &ir);
    }
    const bool enough = own >= need;
    Text(label.empty() ? name : label,
         static_cast<float>(static_cast<int>(x) + 44),
         static_cast<float>(static_cast<int>(y) + 2), 11, enough ? 0 : 200, 0,
         0);
    Text(Num(own) + u"/" + Num(need),
         static_cast<float>(static_cast<int>(x) + 44),
         static_cast<float>(static_cast<int>(y) + 20), 11, enough ? 0 : 200, 0,
         0);
    ++slot;
  };

  for (const auto& ing : recipe.ingredients) {
    const uint32_t own = PackageGameInstance::LoadItemNum(ItemId(ing.item));
    draw_slot(ing.item, own, ing.count, u"");
    if (slot >= 6) {
      break;
    }
  }
  if (slot < 6) {
    draw_slot(0, static_cast<uint32_t>(PackageGameInstance::meso_), recipe.meso,
              u"mesos");
  }

  // ADDITIVE 空槽
  static auto additive_empty = WzResource::LoadTexture(
      WzResource::ui_->find(u"Crafting.img/ItemEntry/empty"));
  if (additive_empty != nullptr) {
    SDL_FRect dst{pos_.x + 367, pos_.y + 197, 34, 34};
    SDL_RenderTexture(Window::renderer_, additive_empty, nullptr, &dst);
  }
}
void CraftUiSystem::RenderBottom() {
  SDL_FPoint count_box{pos_.x + 369, pos_.y + 378};
  Text(Num(craft_count_), count_box.x, count_box.y, 11);

  static auto check_unchecked =
      WzResource::LoadTexture(WzResource::ui_->find(u"Basic.img/CheckBox/0/0"));
  static auto check_checked =
      WzResource::LoadTexture(WzResource::ui_->find(u"Basic.img/CheckBox/0/1"));
  auto* check_texture = additive_checked_ ? check_checked : check_unchecked;
  if (check_texture != nullptr) {
    SDL_FRect check{pos_.x + 368, pos_.y + 178,
                    static_cast<float>(check_texture->w),
                    static_cast<float>(check_texture->h)};
    SDL_RenderTexture(Window::renderer_, check_texture, nullptr, &check);
  }
}
void CraftUiSystem::RenderTooltip() {
  const float mouse_x =
      static_cast<float>(static_cast<int>(Window::mouse_pos_.x));
  const float mouse_y =
      static_cast<float>(static_cast<int>(Window::mouse_pos_.y));
  if (hover_ingredient_.has_value()) {
    const auto item_id = ItemId(*hover_ingredient_);
    if (ItemGameInstance::CheckItem(item_id)) {
      auto item = ItemGameInstance::LoadItem(item_id, 1);
      TooltipUiSystem::RenderItem(*item, mouse_x + 15, mouse_y + 15);
    } else {
      GameEquipItem equip;
      equip.id = item_id;
      equip.type = ItemEnum::kEquip;
      TooltipUiSystem::RenderEquip(equip, mouse_x + 15, mouse_y + 15);
    }
    return;
  }
  if (!hover_recipe_.has_value()) {
    return;
  }
  const auto recipes = LoadRecipes();
  if (*hover_recipe_ >= recipes.size()) {
    return;
  }
  auto item_id = ItemId(recipes[*hover_recipe_].result.item);
  if (ItemGameInstance::CheckItem(item_id)) {
    auto item = ItemGameInstance::LoadItem(item_id, 1);
    TooltipUiSystem::RenderItem(*item, mouse_x + 15, mouse_y + 15);
  } else {
    GameEquipItem equip;
    equip.id = item_id;
    equip.type = ItemEnum::kEquip;
    TooltipUiSystem::RenderEquip(equip, mouse_x + 15, mouse_y + 15);
  }
}

bool CraftUiSystem::CanCraft(const CraftGameInstance::CraftRecipe& recipe,
                             uint32_t count) {
  const int craft_level = std::max(
      1, static_cast<int>(CraftGameInstance::datas_[selected_tab_].level));
  if (craft_level < recipe.level) {
    return false;
  }
  if (PackageGameInstance::meso_ < static_cast<uint64_t>(recipe.meso) * count) {
    return false;
  }
  for (const auto& ing : recipe.ingredients) {
    if (PackageGameInstance::LoadItemNum(ItemId(ing.item)) <
        static_cast<uint32_t>(ing.count) * count) {
      return false;
    }
  }
  return true;
}

void CraftUiSystem::EventCountDelta(int delta) {
  const int v = std::clamp<int>(static_cast<int>(craft_count_) + delta, 1, 999);
  craft_count_ = static_cast<uint32_t>(v);
}

void CraftUiSystem::EventCraft(bool all) {
  const auto recipes = LoadRecipes();
  if (recipes.empty() || selected_recipe_ >= recipes.size()) {
    return;
  }
  const auto& recipe = recipes[selected_recipe_];
  uint32_t count = all ? 999 : craft_count_;
  while (count > 0 && !CanCraft(recipe, count)) {
    --count;
  }
  if (count == 0) {
    return;
  }

  auto result_id = ItemId(recipe.result.item);
  if (!ItemGameInstance::CheckItem(result_id)) {
    return;
  }
  if (pending_craft_.has_value()) {
    return;
  }

  PackageGameInstance::meso_ -= static_cast<uint64_t>(recipe.meso) * count;
  for (const auto& ing : recipe.ingredients) {
    auto sid = ItemId(ing.item);
    if (auto* item = PackageGameInstance::LoadItem(sid); item != nullptr) {
      ItemGameInstance::DecItemNum(*item, static_cast<int>(ing.count) * count);
    }
  }

  PendingCraft pending;
  pending.item_id = recipe.result.item;
  pending.result_count = recipe.result.count;
  pending.exp = recipe.result.exp;
  pending.count = count;
  pending.tab = selected_tab_;
  pending_craft_ = pending;
  craft_finish_time_ = Window::dt_now_ + 5000;

  auto& self = CharacterGameInstance::self_;
  self.action =
      (CharacterGameInstance::bone_data_.contains(u"craft") ||
       CharacterGameInstance::extern_action_.contains(u"craft"))
          ? u"craft"
          : u"alert";
  self.action_index = 0;
  self.action_time = 0;
  self.action_animate = true;
  self.skill = std::nullopt;

  if (!std::ranges::contains(System::logic_systems_, &Run)) {
    System::logic_systems_.push_back(Run);
  }
}

bool CraftUiSystem::Run() {
  if (!pending_craft_.has_value()) {
    return true;
  }
  {
    auto& self = CharacterGameInstance::self_;
    const auto craft_action =
        (CharacterGameInstance::bone_data_.contains(u"craft") ||
         CharacterGameInstance::extern_action_.contains(u"craft"))
            ? u"craft"
            : u"alert";
    if (self.action != craft_action) {
      self.action = craft_action;
      self.action_index = 0;
      self.action_time = 0;
      self.action_animate = true;
      self.skill = std::nullopt;
    }
  }  if (Window::dt_now_ < craft_finish_time_) {
    return true;
  }

  const PendingCraft pending = *pending_craft_;
  pending_craft_ = std::nullopt;

  const auto result_id = ItemId(pending.item_id);
  if (ItemGameInstance::CheckItem(result_id)) {
    for (uint32_t i = 0; i < pending.count; ++i) {
      auto item =
          ItemGameInstance::LoadItem(result_id, pending.result_count);
      PackageGameInstance::AddNewItem(item);
    }
  }

  auto& craft = CraftGameInstance::datas_[pending.tab];
  craft.exp += static_cast<uint32_t>(pending.exp) * pending.count;
  while (craft.exp >= 10000) {
    craft.exp -= 10000;
    craft.level = static_cast<uint8_t>(std::min<int>(craft.level + 1, 100));
  }

  auto& self = CharacterGameInstance::self_;
  self.action = u"stand1";
  self.action_index = 0;
  self.action_time = 0;
  self.action_animate = true;
  return true;
}
bool CraftUiSystem::Crafting() {
  return pending_craft_.has_value();
}
float CraftUiSystem::CraftProgress() {
  if (!pending_craft_.has_value()) {
    return 0.0f;
  }
  if (Window::dt_now_ >= craft_finish_time_) {
    return 1.0f;
  }
  return std::clamp(
      1.0f - static_cast<float>(craft_finish_time_ - Window::dt_now_) /
                 5000.0f,
      0.0f, 1.0f);
}
UIComponents& CraftUiSystem::GetComponent() {
  components_.clear();
  const static auto tab_node =
      WzResource::ui_->find(u"Crafting.img/tab:category");
  std::vector<SDL_FRect> rects;
  for (uint8_t i = 0; i <= 5; i++) {
    rects.push_back(
        SDL_FRect{pos_.x + 9.0f + i * 88.0f, pos_.y + 23.0f, 88.0f, 21.0f});
  }
  components_.push_back(
      Tab{tab_node, u"selected", u"normal", rects, selected_tab_});

  const auto focused = CursorGameInstance::cursor_ui_ == Render;
  auto add_button = [&](const char16_t* path, float x, float y,
                        Button::Callback cb) {
    auto* node = WzResource::ui_->find(path);
    components_.push_back(Button{
        .node = node,
        .rect = {pos_.x + x, pos_.y + y, ButtonW(node), ButtonH(node)},
        .callback = std::move(cb),
        .enable = true,
        .focused = focused,
    });
  };
  add_button(u"Crafting.img/button:minus", 339, 381,
             [] { EventCountDelta(-1); });
  add_button(u"Crafting.img/button:plus", 394, 381, [] { EventCountDelta(1); });
  add_button(u"Crafting.img/button:craft", 414, 379, [] { EventCraft(false); });
  add_button(u"Crafting.img/button:craftAll", 482, 379,
             [] { EventCraft(true); });
  add_button(u"Basic.img/BtClose", 531, 5, [] { Close(); });

  const auto recipes = LoadRecipes();
  const auto visible = static_cast<uint32_t>(kRecipeRows);
  // 7 个 34px 条目 + 6 个 4px 间距 = 262，与最后一个配方底部对齐。
  Scroll scroll{recipe_scroll_, static_cast<uint32_t>(recipes.size()),
                static_cast<int32_t>(kRecipeScroll), visible};
  scroll.pos = {pos_.x + 173, pos_.y + 124};
  scroll.top = focused;
  components_.push_back(scroll);
  return components_;
}

bool CraftUiSystem::EventUI(SDL_Event* event) {
  for (auto& component : GetComponent()) {
    if (auto* tab = std::get_if<Tab>(&component)) {
      if (auto i = tab->Click()) {
        selected_tab_ = *i;
        selected_recipe_ = 0;
        recipe_scroll_ = 0;
        const int craft_level = std::max(
            1,
            static_cast<int>(CraftGameInstance::datas_[selected_tab_].level));
        const auto recipes = LoadRecipes();
        for (size_t n = 0; n < recipes.size(); ++n) {
          if (recipes[n].level <= craft_level) {
            selected_recipe_ = n;
            break;
          }
        }
        return true;
      }
    }
  }
  if (DispatchButtonClick(components_, true)) {
    return true;
  }

  const auto mouse = SDL_FPoint{event->button.x, event->button.y};
  SDL_FRect scroll_area{pos_.x + 173, pos_.y + 124, 15, kRecipeScroll};
  if (SDL_PointInRectFloat(&mouse, &scroll_area)) {
    for (auto& component : components_) {
      if (auto* scroll = std::get_if<Scroll>(&component)) {
        recipe_scroll_ = scroll->Click(mouse);
        return true;
      }
    }
  }
  SDL_FRect additive{pos_.x + 365, pos_.y + 183, 11, 11};
  if (SDL_PointInRectFloat(&mouse, &additive)) {
    additive_checked_ = !additive_checked_;
    return true;
  }

  SDL_FRect list{pos_.x + kRecipeX, pos_.y + kRecipeY, kRecipeW, kRecipeScroll};
  if (SDL_PointInRectFloat(&mouse, &list)) {
    const int rel = static_cast<int>(mouse.y - list.y);
    for (int r = 0; r < 7; ++r) {
      const int top = r * 235 / 6;
      if (rel >= top && rel < top + 34) {
        const auto index = recipe_scroll_ + static_cast<uint32_t>(r);
        const auto recipes = LoadRecipes();
        if (index < recipes.size()) {
          const int craft_level = std::max(
              1,
              static_cast<int>(CraftGameInstance::datas_[selected_tab_].level));
          // 不可制作的配方点击无效，右侧不显示其详情。
          if (recipes[index].level > craft_level) {
            return true;
          }
          selected_recipe_ = index;
          return true;
        }
        break;
      }
    }
  }
  return false;
}

bool CraftUiSystem::Render() {
  // 所有 UI 渲染坐标强制取整，避免浮点坐标导致像素模糊。
  pos_.x = static_cast<float>(static_cast<int>(std::lround(pos_.x)));
  pos_.y = static_cast<float>(static_cast<int>(std::lround(pos_.y)));
  RenderBackgrnd();
  RenderLevel();
  RenderCategory();
  RenderRecipes();
  RenderDetail();
  RenderBottom();
  RenderUI();
  return true;
}

bool CraftUiSystem::RenderInfo() {
  if (CursorGameInstance::cursor_ui_ != Render) {
    return true;
  }
  if (CursorGameInstance::modal_overlay_) {
    return true;
  }
  RenderTooltip();
  return true;
}

void CraftUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;
    System::render_systems_.insert(it, Render);
    // 工具提示始终绘制在最后，避免被后打开的 UI 遮挡。
    System::render_systems_.push_back(RenderInfo);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void CraftUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::render_systems_, RenderInfo);
  std::erase(System::event_systems_, Event);
}

void CraftUiSystem::EventTop() {
  std::erase(System::render_systems_, Render);
  std::erase(System::render_systems_, RenderInfo);
  std::erase(System::event_systems_, Event);
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::render_systems_.push_back(RenderInfo);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void CraftUiSystem::EventDragStart(SDL_Event* event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos_.x, pos_.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - event->button.x, pos_.y - event->button.y};
  }
}

void CraftUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
}

void CraftUiSystem::EventDragMove(SDL_Event* event) {
  if (drag_.has_value()) {
    pos_ = {event->motion.x + drag_->x, event->motion.y + drag_->y};
    auto& camera = CameraGameInstance::camera_;
    auto [w, h] = LoadWh();
    pos_.x = std::clamp(pos_.x, 0.0f, camera.w - w);
    pos_.y = std::clamp(pos_.y, 0.0f, camera.h - h);
  }
}

void CraftUiSystem::Toggle() {
  if (std::ranges::contains(System::render_systems_, &Render)) {
    Close();
  } else {
    Open();
  }
}

bool CraftUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool CraftUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_MOUSE_WHEEL: {
      if (CursorGameInstance::cursor_ui_ == Render) {
        const SDL_FPoint mouse = Window::mouse_pos_;
        SDL_FRect list{pos_.x + kRecipeX, pos_.y + kRecipeY, 160,
                       kRecipeScroll};
        SDL_FRect track{pos_.x + 173, pos_.y + 124, 16, kRecipeScroll};
        if (SDL_PointInRectFloat(&mouse, &list) ||
            SDL_PointInRectFloat(&mouse, &track)) {
          const auto recipes = LoadRecipes();
          const int max_scroll =
              recipes.size() > 7 ? static_cast<int>(recipes.size()) - 7 : 0;
          const int delta = event->wheel.y > 0 ? -1 : 1;
          recipe_scroll_ = static_cast<uint32_t>(std::clamp(
              static_cast<int>(recipe_scroll_) + delta, 0, max_scroll));
          return false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          const SDL_FPoint mouse{event->button.x, event->button.y};
          auto& components = GetComponent();
          for (auto& component : components) {
            if (auto* scroll = std::get_if<Scroll>(&component);
                scroll != nullptr && scroll->InTrack(mouse)) {
              drag_scroll_ = 0;
              recipe_scroll_ = scroll->Drag(mouse);
              EventTop();
              return false;
            }
          }
          EventTop();
          EventDragStart(event);
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (drag_scroll_ >= 0) {
          drag_scroll_ = -1;
          EventDragEnd();
          return false;
        }
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = !EventUI(event);
        }
        EventDragEnd();
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      if (drag_scroll_ >= 0) {
        const SDL_FPoint mouse{event->motion.x, event->motion.y};
        auto& components = GetComponent();
        for (auto& component : components) {
          if (auto* scroll = std::get_if<Scroll>(&component)) {
            const auto recipes = LoadRecipes();
            const int max_scroll =
                recipes.size() > 7 ? static_cast<int>(recipes.size()) - 7 : 0;
            recipe_scroll_ = static_cast<uint32_t>(std::clamp(
                static_cast<int>(scroll->Drag(mouse)), 0, max_scroll));
            break;
          }
        }
        return false;
      }
      EventDragMove(event);
      break;
    }
    default: {
      break;
    }
  }
  return r;
}

}  // namespace sdlms
