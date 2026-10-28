#include "src/client/system/ui/tooltip_ui_system.h"
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/ui/minimap_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include <wz/Node.h>
#include <wz/Property.h>
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
namespace sdlms {


int TooltipUiSystem::LoadEquipWidth(const std::u16string &id) {
  Freetype::LoadSize(15);
  auto equip_name = EquipGameInstance::LoadEquipName(id);
  int w = Freetype::LoadW(equip_name);
  return std::max(w + 60, 238);
}

float TooltipUiSystem::LoadDecoBottomH(const std::u16string &id) {
  auto desc = EquipGameInstance::LoadEquipDesc(id);
  if (desc.empty()) {
    return 20;
  }
  const auto width = LoadEquipWidth(id) - 24;
  Freetype::LoadSize(12);
  auto h = Freetype::LoadRh(desc, width, 1.3, std::nullopt);
  return h + 18;
}

float TooltipUiSystem::LoadEquipBottomH(GameEquipItem &equip) {
  auto &id = equip.id;
  auto equip_inc = EquipGameInstance::LoadEquipInc(id);
  for (const auto &scroll : equip.scroll) {
    if (scroll.success) {
      auto scroll_inc = EquipGameInstance::LoadScrollInc(scroll.id);
      equip_inc.insert_range(scroll_inc);
    }
  }
  Freetype::LoadSize(12);
  auto h = Freetype::LoadLh();
  return h * (equip_inc.size() + 1);
}

void TooltipUiSystem::RenderEquipBottomInc(std::u16string label,
                                                std::u16string val, float x,
                                                float &y) {
  static auto dot1 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Tooltip.img/Equip/Dot/1"));
  SDL_FRect pos_rect{
      x - 10,
      y + 9,
      static_cast<float>(dot1->w),
      static_cast<float>(dot1->h),
  };
  SDL_RenderTexture(Window::renderer_, dot1, nullptr, &pos_rect);
  // static auto
  Freetype::LoadAligned(true);
  Freetype::LoadColor(240, 224, 104, 255);
  label += u":";
  Freetype::DrawLine(label, x, y);
  auto w = Freetype::LoadW(label);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(val, x + w + 2, y);
  auto h = Freetype::LoadLh();
  y += h + 1;
}

void TooltipUiSystem::RenderDecoBottom(GameDecoItem &deco, float x,
                                           float y) {
  auto id = deco.id;
  auto equip_inc = EquipGameInstance::LoadEquipInc(id);
  Freetype::LoadSize(12);
  auto h = Freetype::LoadLh();
  //   type
  auto equip_type = EquipGameInstance::LoadEquipType(id);
  if (equip_type == u"Weapon") {
    static auto weapon_node =
        WzResource::ms_->get_root()->find(u"String.img/Equip/weapon/type");
    auto sub_id = id.substr(0, 4);
    equip_type = static_cast<wz::Property<std::u16string> *>(
                     weapon_node->get_child(sub_id))
                     ->get();
  }
  auto str_node = WzResource::ms_->get_root()->find(u"String.img/Equip/type");
  auto type_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
  RenderEquipBottomInc(type_str, equip_type, x, y);

  auto w = LoadEquipWidth(deco.id);
  Freetype::LoadSize(12);

  auto desc = EquipGameInstance::LoadEquipDesc(deco.id);
  if (!desc.empty()) {
    x -= 10;
    y += 4;
    Freetype::DrawRstr(desc, x, y, w - 24, 1.3, std::nullopt);
  }
}

void TooltipUiSystem::RenderEquipBottom(GameEquipItem &equip, float x,
                                            float y) {
  auto id = equip.id;
  auto equip_inc = EquipGameInstance::LoadEquipInc(id);
  std::flat_map<EquipGameInstance::IncType, int> scroll_inc;
  for (const auto &scroll : equip.scroll) {
    if (scroll.success) {
      auto scroll_inc2 = EquipGameInstance::LoadScrollInc(scroll.id);
      for (auto [k, v] : scroll_inc2) {
        scroll_inc[k] += v;
      }
    }
  }

  Freetype::LoadSize(12);
  auto h = Freetype::LoadLh();
  //   type
  auto equip_type = EquipGameInstance::LoadEquipType(id);
  if (equip_type == u"Weapon") {
    static auto weapon_node =
        WzResource::ms_->get_root()->find(u"String.img/Equip/weapon/type");
    auto sub_id = id.substr(0, 4);
    equip_type = static_cast<wz::Property<std::u16string> *>(
                     weapon_node->get_child(sub_id))
                     ->get();
  }
  auto str_node = WzResource::ms_->get_root()->find(u"String.img/Equip/type");
  auto type_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
  RenderEquipBottomInc(type_str, equip_type, x, y);
  // weapon speed
  if (equip_inc.contains(EquipGameInstance::IncType::kWeaponSpeed)) {
    auto str_node =
        WzResource::ms_->get_root()->find(u"String.img/Equip/attackSpeed");
    auto spd_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    std::u16string val_str;
    auto val = equip_inc.at(EquipGameInstance::IncType::kWeaponSpeed);
    if (val >= 6) {
      // slow
      str_node = WzResource::ms_->get_root()->find(
          u"String.img/Equip/weapon/speed/slow");
      val_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    } else if (val > 4) {
      // Normal
      str_node = WzResource::ms_->get_root()->find(
          u"String.img/Equip/weapon/speed/normal");
      val_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    } else {
      // Fast
      str_node = WzResource::ms_->get_root()->find(
          u"String.img/Equip/weapon/speed/fast");
      val_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    }
    auto speed_val_str = std::to_string(val);
    auto speed_val_str2 =
        std::u16string{speed_val_str.begin(), speed_val_str.end()};
    val_str = val_str + u"(" + speed_val_str2 + u")";
    RenderEquipBottomInc(spd_str, val_str, x, y);
  }

  // weapon pad
  if (equip_inc.contains(EquipGameInstance::IncType::kWeaponPad) ||
      scroll_inc.contains(EquipGameInstance::IncType::kWeaponPad)) {
    auto str_node =
        WzResource::ms_->get_root()->find(u"String.img/Equip/weaponAttack");
    auto pad_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kWeaponPad];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kWeaponPad)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kWeaponPad);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    RenderEquipBottomInc(
        pad_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  // weapon pdd
  if (equip_inc.contains(EquipGameInstance::IncType::kPdd) ||
      scroll_inc.contains(EquipGameInstance::IncType::kPdd)) {
    str_node = WzResource::ms_->get_root()->find(u"String.img/Character/pdd");
    auto pdd_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kPdd];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kPdd)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kPdd);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    RenderEquipBottomInc(
        pdd_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  // weapon mad
  if (equip_inc.contains(EquipGameInstance::IncType::kWeaponMad) ||
      scroll_inc.contains(EquipGameInstance::IncType::kWeaponMad)) {
    str_node =
        WzResource::ms_->get_root()->find(u"String.img/Equip/weaponMagic");
    auto mad_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kWeaponMad];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kWeaponMad)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kWeaponMad);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    RenderEquipBottomInc(
        mad_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  // acc
  if (equip_inc.contains(EquipGameInstance::IncType::kAcc) ||
      scroll_inc.contains(EquipGameInstance::IncType::kAcc)) {
    str_node = WzResource::ms_->get_root()->find(u"String.img/Character/acc");
    auto acc_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kAcc];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kAcc)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kAcc);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    RenderEquipBottomInc(
        acc_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(EquipGameInstance::IncType::kStr) ||
      scroll_inc.contains(EquipGameInstance::IncType::kStr)) {
    auto str_node =
        WzResource::ms_->get_root()->find(u"String.img/Character/str");
    auto str_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kStr];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kStr)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kStr);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    RenderEquipBottomInc(
        str_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(EquipGameInstance::IncType::kDex) ||
      scroll_inc.contains(EquipGameInstance::IncType::kDex)) {
    auto str_node =
        WzResource::ms_->get_root()->find(u"String.img/Character/dex");
    auto dex_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kDex];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kDex)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kDex);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    RenderEquipBottomInc(
        dex_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(EquipGameInstance::IncType::kInt) ||
      scroll_inc.contains(EquipGameInstance::IncType::kInt)) {
    auto str_node =
        WzResource::ms_->get_root()->find(u"String.img/Character/int");
    auto int_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kInt];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kInt)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kInt);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    RenderEquipBottomInc(
        int_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(EquipGameInstance::IncType::kSpeed) ||
      scroll_inc.contains(EquipGameInstance::IncType::kSpeed)) {
    auto str_node =
        WzResource::ms_->get_root()->find(u"String.img/Character/speed");
    auto spd_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kSpeed];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kSpeed)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kSpeed);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    RenderEquipBottomInc(
        spd_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(EquipGameInstance::IncType::kJump) ||
      scroll_inc.contains(EquipGameInstance::IncType::kJump)) {
    auto str_node =
        WzResource::ms_->get_root()->find(u"String.img/Character/jump");
    auto jump_str =
        static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kJump];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kJump)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kJump);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    RenderEquipBottomInc(
        jump_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }
  // enh
  str_node = WzResource::ms_->get_root()->find(
      u"String.img/Equip/remainingEnhancements");
  auto enh_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
  auto val = EquipGameInstance::LoadEquipTuc(id) - equip.scroll.size();
  auto val_str = std::to_string(val);
  RenderEquipBottomInc(enh_str,
                          std::u16string{val_str.begin(), val_str.end()}, x, y);
}

void TooltipUiSystem::RenderEquipReq(uint32_t req, uint32_t val,
                                         const std::u16string &path, float x,
                                         float y) {
  static auto equip_node = WzResource::ui_->find(u"UIToolTip.img/Item/Equip");
  auto color_node = equip_node;
  SDL_Texture *texture;
  if (req == 0) {
    texture = WzResource::LoadTexture(equip_node->find(u"Disabled/" + path));
    color_node = equip_node->get_child(u"Disabled");
  } else if (req >= val) {
    texture = WzResource::LoadTexture(equip_node->find(u"Cannot/" + path));
    color_node = equip_node->get_child(u"Cannot");
  } else {
    texture = WzResource::LoadTexture(equip_node->find(u"Can/" + path));
    color_node = equip_node->get_child(u"Can");
  }
  SDL_FRect pos_rect = {
      x,
      y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);

  auto req_str = std::to_string(req);
  uint32_t w = 0;
  for (auto c : req_str) {
    auto num_node = color_node->get_child(std::string{c});
    auto num_texture = WzResource::LoadTexture(num_node);
    auto num_origin = WzResource::LoadFpoint(num_node->get_child(u"origin"));
    SDL_FRect r = {
        x + 54 + w - num_origin.x,
        pos_rect.y,
        static_cast<float>(num_texture->w),
        static_cast<float>(num_texture->h),
    };
    SDL_RenderTexture(Window::renderer_, num_texture, nullptr, &r);
    w += num_texture->w + 1 - num_origin.x;
  }
}

void TooltipUiSystem::RenderEquipJob(const std::u16string &id, float x,
                                         float y) {
  // job
  auto jobs = EquipGameInstance::LoadEquipJob(id);
  static auto job_node = WzResource::ui_->find(u"UIToolTip.img/Item/Equip/Job");
  wz::Node *node;
  SDL_FRect pos_rect;
  for (auto i : {
           JobType::kBeginner,
           JobType::kWarrior,
           JobType::kMagician,
           JobType::kBowman,
           JobType::kThief,
       }) {
    if (jobs.contains(i)) {
      node = job_node->get_child(u"enable");
    } else {
      node = job_node->get_child(u"disable");
    }
    node = node->get_child(std::to_string((uint8_t)i));
    auto origin = WzResource::LoadFpoint(node->get_child(u"origin"));
    auto texture = WzResource::LoadTexture(node);
    pos_rect.x = x - origin.x;
    pos_rect.y = y - origin.y;
    pos_rect.w = texture->w;
    pos_rect.h = texture->h;
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  }
}

void TooltipUiSystem::RenderDeco(GameDecoItem &deco, float x, float y) {
  // backgrnd
  auto w = LoadEquipWidth(deco.id);
  const auto h = 160 + LoadDecoBottomH(deco.id);
  RenderBackgrnd(x, y, w, h);

  //   render
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::LoadAligned(true);

  static auto dot0 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Tooltip.img/Equip/Dot/0"));

  SDL_FRect pos_rect = {
      x + 10,
      y + 20,
      static_cast<float>(dot0->w),
      static_cast<float>(dot0->h),
  };
  SDL_RenderTexture(Window::renderer_, dot0, nullptr, &pos_rect);

  auto equip_name = EquipGameInstance::LoadEquipName(deco.id);
  Freetype::LoadBold(true);
  Freetype::LoadSize(15);
  Freetype::DrawLine(equip_name, x + 20, y + 10);
  Freetype::LoadBold(false);

  static auto item_base = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIToolTip.img/Item/ItemIcon/base"));
  SDL_FPoint base{10, 35};
  pos_rect = {x + base.x, y + base.y, static_cast<float>(item_base->w),
              static_cast<float>(item_base->h)};
  SDL_RenderTexture(Window::renderer_, item_base, nullptr, &pos_rect);

  static auto item_cover = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIToolTip.img/Item/ItemIcon/cover"));
  pos_rect = {
      x + base.x + 4,
      y + base.y + 4,
      static_cast<float>(item_cover->w),
      static_cast<float>(item_cover->h),
  };
  SDL_RenderTexture(Window::renderer_, item_cover, nullptr, &pos_rect);

  auto equip_info = EquipGameInstance::LoadEquipInfo(deco.id);
  auto eqp_texture = WzResource::LoadTexture(equip_info->get_child(u"icon"));
  pos_rect.w = eqp_texture->w * 2;
  pos_rect.h = eqp_texture->h * 2;
  auto dx = 74 - pos_rect.w;
  auto dy = 74 - pos_rect.h;
  pos_rect.x += dx / 2;
  pos_rect.y += dy / 2;
  SDL_RenderTexture(Window::renderer_, eqp_texture, nullptr, &pos_rect);

  auto cash_icon = WzResource::LoadTexture(
      WzResource::ms_->get_root()->find(u"UI.img/CashItem"));
  pos_rect = {
      x + base.x + 4 + 46,
      y + base.y + 4 + 46,
      static_cast<float>(cash_icon->w) * 2,
      static_cast<float>(cash_icon->h) * 2,
  };
  SDL_RenderTexture(Window::renderer_, cash_icon, nullptr, &pos_rect);

  auto req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLevel"))
          ->get();
  auto val = CharacterGameInstance::self_.level;
  RenderEquipReq(req, val, u"reqLEV", x + base.x + 95, y + base.y + 6);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqSTR"))->get();
  val = CharacterStatGameInstance::GetStrAp();
  RenderEquipReq(req, val, u"reqSTR", x + base.x + 95, y + base.y + 18);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqDEX"))->get();
  val = CharacterStatGameInstance::GetDexAp();
  RenderEquipReq(req, val, u"reqDEX", x + base.x + 95, y + base.y + 30);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqINT"))->get();
  val = CharacterStatGameInstance::GetIntAp();
  RenderEquipReq(req, val, u"reqINT", x + base.x + 95, y + base.y + 42);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLUK"))->get();
  val = CharacterStatGameInstance::GetLukAp();
  RenderEquipReq(req, val, u"reqLUK", x + base.x + 95, y + base.y + 54);

  req = 0;
  val = 0;
  RenderEquipReq(req, val, u"reqPOP", x + base.x + 95, y + base.y + 66);

  RenderEquipJob(deco.id, x + base.x, y + base.y + 80);

  auto line_y = y + base.y + 112;
  SDL_SetRenderDrawColor(Window::renderer_, 255, 255, 255, 255);
  SDL_RenderLine(Window::renderer_, x + 5, line_y, x + w - 8, line_y);

  Freetype::LoadAligned(false);

  RenderDecoBottom(deco, x + 22, line_y + 5);
}

void TooltipUiSystem::RenderEquip(GameEquipItem &equip, float x, float y) {
  // backgrnd
  static auto backgrnd_node = WzResource::ui_->find(u"UIToolTip.img/Item");
  static auto top =
      WzResource::LoadTexture(backgrnd_node->find(u"Frame/top"));
  static auto line =
      WzResource::LoadTexture(backgrnd_node->find(u"Frame/line"));
  static auto bottom =
      WzResource::LoadTexture(backgrnd_node->find(u"Frame/bottom"));
  const auto h = 180 + LoadEquipBottomH(equip);
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(top->w),
      static_cast<float>(top->h),
  };
  SDL_RenderTexture(Window::renderer_, top, nullptr, &pos_rect);

  pos_rect.y = y + h - bottom->h;
  pos_rect.h = bottom->h;
  SDL_RenderTexture(Window::renderer_, bottom, nullptr, &pos_rect);

  pos_rect.y = y + top->h;
  pos_rect.h = h - top->h - bottom->h;
  SDL_RenderTexture(Window::renderer_, line, nullptr, &pos_rect);

  //   render
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::LoadAligned(true);

  static auto dot0 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Tooltip.img/Equip/Dot/0"));

  pos_rect = {x + 10, y + 20, static_cast<float>(dot0->w),
              static_cast<float>(dot0->h)};
  SDL_RenderTexture(Window::renderer_, dot0, nullptr, &pos_rect);

  auto equip_name = EquipGameInstance::LoadEquipName(equip.id);
  int scroll_num = 0;
  for (auto &scroll : equip.scroll) {
    if (scroll.success) {
      scroll_num++;
    }
  }
  Freetype::LoadBold(true);
  Freetype::LoadSize(15);
  if (scroll_num > 0) {
    auto tmp = std::to_string(scroll_num);
    equip_name =
        equip_name + u"(+" + std::u16string{tmp.begin(), tmp.end()} + u")";
  }
  Freetype::DrawLine(equip_name, x + 20, y + 10);
  Freetype::LoadBold(false);

  static auto item_base = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIToolTip.img/Item/ItemIcon/base"));
  SDL_FPoint base{10, 35};
  pos_rect = {x + base.x, y + base.y, static_cast<float>(item_base->w),
              static_cast<float>(item_base->h)};
  SDL_RenderTexture(Window::renderer_, item_base, nullptr, &pos_rect);

  static auto item_cover = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIToolTip.img/Item/ItemIcon/cover"));
  pos_rect = {x + base.x + 4, y + base.y + 4, static_cast<float>(item_cover->w),
              static_cast<float>(item_cover->h)};
  SDL_RenderTexture(Window::renderer_, item_cover, nullptr, &pos_rect);

  auto equip_info = EquipGameInstance::LoadEquipInfo(equip.id);
  auto eqp_texture = WzResource::LoadTexture(equip_info->get_child(u"icon"));
  pos_rect.w = eqp_texture->w * 2;
  pos_rect.h = eqp_texture->h * 2;
  auto dx = 74 - pos_rect.w;
  auto dy = 74 - pos_rect.h;
  pos_rect.x += dx / 2;
  pos_rect.y += dy / 2;

  SDL_RenderTexture(Window::renderer_, eqp_texture, nullptr, &pos_rect);

  auto req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLevel"))
          ->get();
  auto val = CharacterGameInstance::self_.level;
  RenderEquipReq(req, val, u"reqLEV", x + base.x + 95, y + base.y + 6);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqSTR"))->get();
  val = CharacterStatGameInstance::GetStrAp();
  RenderEquipReq(req, val, u"reqSTR", x + base.x + 95, y + base.y + 18);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqDEX"))->get();
  val = CharacterStatGameInstance::GetDexAp();
  RenderEquipReq(req, val, u"reqDEX", x + base.x + 95, y + base.y + 30);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqINT"))->get();
  val = CharacterStatGameInstance::GetIntAp();
  RenderEquipReq(req, val, u"reqINT", x + base.x + 95, y + base.y + 42);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLUK"))->get();
  val = CharacterStatGameInstance::GetLukAp();
  RenderEquipReq(req, val, u"reqLUK", x + base.x + 95, y + base.y + 54);

  req = 0;
  val = 0;
  RenderEquipReq(req, val, u"reqPOP", x + base.x + 95, y + base.y + 66);

  RenderEquipJob(equip.id, x + base.x, y + base.y + 80);

  auto line_y = y + base.y + 112;
  SDL_SetRenderDrawColor(Window::renderer_, 255, 255, 255, 255);
  SDL_RenderLine(Window::renderer_, x + 5, line_y, x + 230, line_y);

  Freetype::LoadAligned(false);

  RenderEquipBottom(equip, x + 22, line_y + 5);
}

void TooltipUiSystem::RenderBackgrnd(float x, float y, float w, float h) {
  // backgrnd
  static auto backgrnd_node =
      WzResource::ui_->find(u"UIToolTip.img/Skill/Frame");

  auto texture_c = WzResource::LoadTexture(backgrnd_node->get_child(u"c"));
  auto texture_e =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"e")));
  auto texture_ne =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"ne")));
  auto texture_n =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"n")));
  auto texture_nw =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"nw")));
  auto texture_w =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"w")));
  auto texture_sw =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"sw")));
  auto texture_s =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"s")));
  auto texture_se =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"se")));

  SDL_FRect rect;
  rect.x = x;
  rect.y = y;

  // 左上
  SDL_FPoint p_nw{x, y};
  rect.x = (p_nw.x);
  rect.y = (p_nw.y);
  rect.w = texture_nw->w;
  rect.h = texture_nw->h;
  SDL_RenderTexture(Window::renderer_, texture_nw, nullptr, &rect);

  // 左下
  SDL_FPoint p_sw{
      x,
      y + h - texture_sw->h,
  };
  rect.x = (p_sw.x);
  rect.y = (p_sw.y);
  rect.w = texture_sw->w;
  rect.h = texture_sw->h;
  SDL_RenderTexture(Window::renderer_, texture_sw, nullptr, &rect);

  // 右上
  SDL_FPoint p_ne{
      x + w - texture_ne->w,
      y,
  };
  rect.x = (p_ne.x);
  rect.y = (p_ne.y);
  rect.w = texture_ne->w;
  rect.h = texture_ne->h;
  SDL_RenderTexture(Window::renderer_, texture_ne, nullptr, &rect);

  // 右下
  SDL_FPoint p_se{
      x + w - texture_ne->w,
      y + h - texture_sw->h,
  };
  rect.x = (p_se.x);
  rect.y = (p_se.y);
  rect.w = texture_se->w;
  rect.h = texture_se->h;
  SDL_RenderTexture(Window::renderer_, texture_se, nullptr, &rect);

  rect.x = (x + texture_nw->w);
  rect.y = (y + texture_nw->h);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = p_sw.y - (p_nw.y + texture_nw->h);
  SDL_RenderTextureTiled(Window::renderer_, texture_c, nullptr, 1, &rect);

  // 竖着的两边
  rect.x = (p_nw.x);
  rect.y = (p_nw.y + texture_nw->h);
  rect.w = texture_w->w;
  rect.h = p_sw.y - (p_nw.y + texture_nw->h);
  SDL_RenderTextureTiled(Window::renderer_, texture_w, nullptr, 1, &rect);

  rect.x = (p_se.x);
  rect.y = (p_ne.y + texture_ne->h);
  rect.w = texture_e->w;
  rect.h = p_se.y - (p_ne.y + texture_ne->h);
  SDL_RenderTextureTiled(Window::renderer_, texture_e, nullptr, 1, &rect);

  // 横着的两边
  rect.x = (p_nw.x + texture_nw->w);
  rect.y = (p_ne.y);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = texture_n->h;
  SDL_RenderTextureTiled(Window::renderer_, texture_n, nullptr, 1, &rect);

  rect.x = (p_nw.x + texture_nw->w);
  rect.y = (p_sw.y);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = texture_s->h;
  SDL_RenderTextureTiled(Window::renderer_, texture_s, nullptr, 1, &rect);
}

float TooltipUiSystem::LoadSkillBottomH(const std::u16string &id) {
  auto ski_name = SkillGameInstance::LoadSkiName(id);
  auto ski_level = JobSkillGameInstance::LoadSkiLevel(id);
  float h = 0;
  const auto w = 390;
  Freetype::LoadSize(12);
  if (ski_level == 0 || ski_level == ski_name.level.size()) {
    ski_level =
        std::clamp(ski_level, (uint8_t)0, (uint8_t)(ski_name.level.size() - 1));
    auto ski_l0 = ski_name.level[ski_level];
    h += Freetype::LoadLh() * 1.1;
    h += Freetype::LoadRh(ski_l0, w - 28, 1.3, std::nullopt);
  } else {
    auto ski_l0 = ski_name.level[ski_level - 1];
    auto ski_l1 = ski_name.level[ski_level];
    h += Freetype::LoadLh() * 2 * 1.1;
    h += Freetype::LoadRh(ski_l0, w - 28, 1.3, std::nullopt);
    h += Freetype::LoadRh(ski_l1, w - 28, 1.3, std::nullopt);
    h += 5;
  }
  return h;
}

void TooltipUiSystem::RenderSkillBottom(const std::u16string &id,
                                            uint8_t level, float x, float y) {
  const auto w = 390;
  auto ski_name = SkillGameInstance::LoadSkiName(id);
  auto ski_level = JobSkillGameInstance::LoadSkiLevel(id);
  auto ski_level2 = std::to_string(ski_level);
  std::u16string ski_level3{ski_level2.begin(), ski_level2.end()};

  auto next_level = ski_level + 1;
  auto next_level2 = std::to_string(next_level);
  std::u16string next_level3{next_level2.begin(), next_level2.end()};

  if (ski_level == 0) {
    Freetype::LoadSize(12);
    Freetype::LoadColor(255, 255, 255, 255);
    auto next_node =
        WzResource::ms_->get_root()->find(u"String.img/Skill/nextLevel");
    auto str = static_cast<wz::Property<std::u16string> *>(next_node)->get();

    str = u"[" + str + u" " + next_level3 + u"]";
    Freetype::DrawLine(str, x, y);

    str = ski_name.level[ski_level];
    y += Freetype::LoadLh() * 1.3;
    Freetype::DrawRstr(str, x + 2, y, w - 22, 1.3, std::nullopt);
  } else if (ski_level == ski_name.level.size()) {
    Freetype::LoadSize(12);
    Freetype::LoadColor(255, 255, 255, 255);
    auto cur_node =
        WzResource::ms_->get_root()->find(u"String.img/Skill/currentLevel");
    auto str = static_cast<wz::Property<std::u16string> *>(cur_node)->get();

    str = u"[" + str + u" " + ski_level3 + u"]";
    Freetype::DrawLine(str, x, y);

    str = ski_name.level[ski_level - 1];
    y += Freetype::LoadLh() * 1.3;
    Freetype::DrawRstr(str, x + 2, y, w - 22, 1.3, std::nullopt);
  } else {
    Freetype::LoadSize(12);
    Freetype::LoadColor(255, 255, 255, 255);
    auto cur_node =
        WzResource::ms_->get_root()->find(u"String.img/Skill/currentLevel");
    auto str = static_cast<wz::Property<std::u16string> *>(cur_node)->get();

    str = u"[" + str + u" " + ski_level3 + u"]";
    Freetype::DrawLine(str, x, y);

    str = ski_name.level[ski_level - 1];
    y += Freetype::LoadLh() * 1.3;
    auto lh =
        Freetype::DrawRstr(str, x + 2, y, w - 28, 1.3, std::nullopt).height;

    y += lh;

    auto next_node =
        WzResource::ms_->get_root()->find(u"String.img/Skill/nextLevel");
    str = static_cast<wz::Property<std::u16string> *>(next_node)->get();

    Freetype::LoadColor(255, 255, 255, 255);
    str = u"[" + str + u" " + next_level3 + u"]";
    Freetype::DrawLine(str, x, y);
    str = ski_name.level[ski_level];
    y += Freetype::LoadLh() * 1.3;
    Freetype::DrawRstr(str, x + 2, y, w - 28, 1.3, std::nullopt);
  }
}

void TooltipUiSystem::RenderSkill(const std::u16string &id, uint8_t level,
                                     float x, float y) {
  auto ski_name = SkillGameInstance::LoadSkiName(id);
  const auto w = 390;
  Freetype::LoadSize(12);
  auto desc = ski_name.desc;
  auto rh = Freetype::LoadRh(desc, w - 28, 1.2, SDL_FRect{0, 0, 74, 74});
  auto h = std::max(140.0f, rh + 60);
  h += LoadSkillBottomH(id);
  RenderBackgrnd(x, y, w, h);
  static auto dot0 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Tooltip.img/Equip/Dot/0"));

  SDL_FRect pos_rect = {x + 10, y + 20, static_cast<float>(dot0->w),
                        static_cast<float>(dot0->h)};
  SDL_RenderTexture(Window::renderer_, dot0, nullptr, &pos_rect);

  static auto item_base = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIToolTip.img/Item/ItemIcon/base"));
  SDL_FPoint base{10, 35};
  pos_rect = {x + base.x, y + base.y, static_cast<float>(item_base->w),
              static_cast<float>(item_base->h)};
  SDL_RenderTexture(Window::renderer_, item_base, nullptr, &pos_rect);

  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::LoadAligned(true);
  Freetype::LoadBold(true);
  Freetype::LoadSize(15);
  Freetype::DrawLine(ski_name.name, x + 20, y + 10);
  Freetype::LoadBold(false);

  auto ski_node = SkillGameInstance::LoadSkiNode(id);
  auto ski_texture = WzResource::LoadTexture(ski_node->get_child(u"icon"));
  pos_rect.w = ski_texture->w * 2;
  pos_rect.h = ski_texture->h * 2;
  auto dx = 74 - pos_rect.w;
  auto dy = 74 - pos_rect.h;
  pos_rect.x += 4 + dx / 2;
  pos_rect.y += 4 + dy / 2;

  SDL_RenderTexture(Window::renderer_, ski_texture, nullptr, &pos_rect);

  Freetype::LoadSize(12);
  Freetype::LoadColor(255, 255, 255, 255);
  auto sx = pos_rect.x;
  auto sy = pos_rect.y - 8;
  auto rstr_h =
      Freetype::DrawRstr(desc, sx, sy, w - 28, 1.2, SDL_FRect{sx, sy, 74, 74})
          .height;

  auto line_y = std::max(sy + rstr_h + 10, y + base.y + 90);
  SDL_SetRenderDrawColor(Window::renderer_, 255, 255, 255, 255);
  SDL_RenderLine(Window::renderer_, x + 5, line_y, x + w - 5, line_y);

  RenderSkillBottom(id, level, x + 14, line_y + 5);
}

void TooltipUiSystem::RenderItem(GameItem &item, float x, float y) {
  auto item_name = ItemGameInstance::LoadItemText(item.id, u"name");
  Freetype::LoadBold(true);
  Freetype::LoadSize(15);
  auto w = Freetype::LoadW(item_name) + 150;
  w = std::max(w, 360.0f);
  RenderBackgrnd(x, y, w, 128);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::LoadAligned(true);
  Freetype::DrawLine(item_name, x + 20, y + 10);
  static auto dot0 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Tooltip.img/Equip/Dot/0"));

  SDL_FRect pos_rect = {x + 10, y + 20, static_cast<float>(dot0->w),
                        static_cast<float>(dot0->h)};
  SDL_RenderTexture(Window::renderer_, dot0, nullptr, &pos_rect);

  static auto item_base = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIToolTip.img/Item/ItemIcon/base"));
  SDL_FPoint base{10, 35};
  pos_rect = {x + base.x, y + base.y, static_cast<float>(item_base->w),
              static_cast<float>(item_base->h)};
  SDL_RenderTexture(Window::renderer_, item_base, nullptr, &pos_rect);

  static auto item_cover = WzResource::LoadTexture(
      WzResource::ui_->find(u"UIToolTip.img/Item/ItemIcon/cover"));
  pos_rect = {x + base.x + 4, y + base.y + 4, static_cast<float>(item_cover->w),
              static_cast<float>(item_cover->h)};
  SDL_RenderTexture(Window::renderer_, item_cover, nullptr, &pos_rect);

  auto item_info = ItemGameInstance::LoadItemInfo(item.id, 0);
  auto item_texture = WzResource::LoadTexture(item_info->get_child(u"icon"));
  pos_rect.w = item_texture->w * 2;
  pos_rect.h = item_texture->h * 2;
  auto dx = 74 - pos_rect.w;
  auto dy = 74 - pos_rect.h;
  pos_rect.x += dx / 2;
  pos_rect.y += dy / 2;

  SDL_RenderTexture(Window::renderer_, item_texture, nullptr, &pos_rect);
  Freetype::LoadBold(false);

  auto item_desc = ItemGameInstance::LoadItemText(item.id, u"desc");
  Freetype::LoadSize(13);
  Freetype::DrawRstr(item_desc, x + base.x + 84, y + base.y - 2, w - 112, 1.1,
                      std::nullopt);
}

void TooltipUiSystem::RenderWorldMapInfo(uint32_t id, float x, float y) {
  auto MapName = MinimapUiSystem::LoadMapName(id).map_name;
  auto street_name = MinimapUiSystem::LoadMapName(id).street_name;
  auto map_desc = MinimapUiSystem::LoadMapName(id).map_desc;
  Freetype::LoadSize(14);
  const auto bh = 60;
  const auto w = 300;
  if (map_desc != u"") {
    auto h = Freetype::LoadRh(map_desc, 280, 1.1, std::nullopt);
    RenderBackgrnd(x, y, w, h + bh + 20);
  } else {
    RenderBackgrnd(x, y, w, bh);
  }
  Freetype::LoadColor(240, 224, 104, 255);
  Freetype::LoadAligned(true);
  auto s1 = street_name;
  auto sw = Freetype::LoadW(s1);
  Freetype::DrawLine(s1, x + (w - sw) / 2, y + 8);

  auto s2 = MapName;
  sw = Freetype::LoadW(s2);
  Freetype::LoadColor(255, 255, 255, 255);
  auto sh = y + Freetype::LoadLh() + 12;
  Freetype::DrawLine(s2, x + (w - sw) / 2, sh);

  if (map_desc != u"") {
    SDL_SetRenderDrawColor(Window::renderer_, 240, 224, 104, 255);
    sh += 25;
    SDL_RenderLine(Window::renderer_, x, sh, x + w, sh);

    auto s3 = map_desc;
    Freetype::DrawCstr(s3, x + 10, sh + 5, w - 20, 1.1);
  }
}

void TooltipUiSystem::RenderStr(const std::u16string &str, float x,
                                   float y) {
  const auto w = Freetype::LoadW(str) + 12;
  Freetype::LoadSize(12);
  auto h = Freetype::LoadLh();
  RenderBackgrnd(x, y, w, 32);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(str, x + 5, y + 5);
}

}  // namespace sdlms
