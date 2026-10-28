#include "tooltip_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
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
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>

int TooltipUiSystem::load_equip_width(const std::u16string &id) {
  Freetype::load_size(15);
  auto equip_name = EquipGameInstance::load_equip_name(id);
  int w = Freetype::load_w(equip_name);
  return std::max(w + 60, 238);
}

float TooltipUiSystem::load_deco_bottom_h(const std::u16string &id) {
  auto desc = EquipGameInstance::load_equip_desc(id);
  if (desc.empty()) {
    return 20;
  }
  const auto width = load_equip_width(id) - 24;
  Freetype::load_size(12);
  auto h = Freetype::load_rh(desc, width, 1.3, std::nullopt);
  return h + 18;
}

float TooltipUiSystem::load_equip_bottom_h(GameEquipItem &equip) {
  auto &id = equip.id;
  auto equip_inc = EquipGameInstance::load_equip_inc(id);
  for (const auto &scroll : equip.scroll) {
    if (scroll.success) {
      auto scroll_inc = EquipGameInstance::load_scroll_inc(scroll.id);
      equip_inc.insert_range(scroll_inc);
    }
  }
  Freetype::load_size(12);
  auto h = Freetype::load_lh();
  return h * (equip_inc.size() + 1);
}

void TooltipUiSystem::render_equip_bottom_inc(std::u16string label,
                                                std::u16string val, float x,
                                                float &y) {
  static auto dot1 = WzResource::load_texture(
      WzResource::ui->find(u"Tooltip.img/Equip/Dot/1"));
  SDL_FRect pos_rect{
      x - 10,
      y + 9,
      static_cast<float>(dot1->w),
      static_cast<float>(dot1->h),
  };
  SDL_RenderTexture(Window::renderer, dot1, nullptr, &pos_rect);
  // static auto
  Freetype::load_aligned(true);
  Freetype::load_color(240, 224, 104, 255);
  label += u":";
  Freetype::draw_line(label, x, y);
  auto w = Freetype::load_w(label);
  Freetype::load_color(255, 255, 255, 255);
  Freetype::draw_line(val, x + w + 2, y);
  auto h = Freetype::load_lh();
  y += h + 1;
}

void TooltipUiSystem::render_deco_bottom(GameDecoItem &deco, float x,
                                           float y) {
  auto id = deco.id;
  auto equip_inc = EquipGameInstance::load_equip_inc(id);
  Freetype::load_size(12);
  auto h = Freetype::load_lh();
  //   type
  auto equip_type = EquipGameInstance::load_equip_type(id);
  if (equip_type == u"Weapon") {
    static auto weapon_node =
        WzResource::ms->get_root()->find(u"String.img/Equip/weapon/type");
    auto sub_id = id.substr(0, 4);
    equip_type = static_cast<wz::Property<std::u16string> *>(
                     weapon_node->get_child(sub_id))
                     ->get();
  }
  auto str_node = WzResource::ms->get_root()->find(u"String.img/Equip/type");
  auto type_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
  render_equip_bottom_inc(type_str, equip_type, x, y);

  auto w = load_equip_width(deco.id);
  Freetype::load_size(12);

  auto desc = EquipGameInstance::load_equip_desc(deco.id);
  if (!desc.empty()) {
    x -= 10;
    y += 4;
    Freetype::draw_rstr(desc, x, y, w - 24, 1.3, std::nullopt);
  }
}

void TooltipUiSystem::render_equip_bottom(GameEquipItem &equip, float x,
                                            float y) {
  auto id = equip.id;
  auto equip_inc = EquipGameInstance::load_equip_inc(id);
  std::flat_map<EquipGameInstance::IncType, int> scroll_inc;
  for (const auto &scroll : equip.scroll) {
    if (scroll.success) {
      auto scroll_inc2 = EquipGameInstance::load_scroll_inc(scroll.id);
      for (auto [k, v] : scroll_inc2) {
        scroll_inc[k] += v;
      }
    }
  }

  Freetype::load_size(12);
  auto h = Freetype::load_lh();
  //   type
  auto equip_type = EquipGameInstance::load_equip_type(id);
  if (equip_type == u"Weapon") {
    static auto weapon_node =
        WzResource::ms->get_root()->find(u"String.img/Equip/weapon/type");
    auto sub_id = id.substr(0, 4);
    equip_type = static_cast<wz::Property<std::u16string> *>(
                     weapon_node->get_child(sub_id))
                     ->get();
  }
  auto str_node = WzResource::ms->get_root()->find(u"String.img/Equip/type");
  auto type_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
  render_equip_bottom_inc(type_str, equip_type, x, y);
  // weapon speed
  if (equip_inc.contains(EquipGameInstance::IncType::kWeaponSpeed)) {
    auto str_node =
        WzResource::ms->get_root()->find(u"String.img/Equip/attackSpeed");
    auto spd_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    std::u16string val_str;
    auto val = equip_inc.at(EquipGameInstance::IncType::kWeaponSpeed);
    if (val >= 6) {
      // slow
      str_node = WzResource::ms->get_root()->find(
          u"String.img/Equip/weapon/speed/slow");
      val_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    } else if (val > 4) {
      // Normal
      str_node = WzResource::ms->get_root()->find(
          u"String.img/Equip/weapon/speed/normal");
      val_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    } else {
      // Fast
      str_node = WzResource::ms->get_root()->find(
          u"String.img/Equip/weapon/speed/fast");
      val_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    }
    auto speed_val_str = std::to_string(val);
    auto speed_val_str2 =
        std::u16string{speed_val_str.begin(), speed_val_str.end()};
    val_str = val_str + u"(" + speed_val_str2 + u")";
    render_equip_bottom_inc(spd_str, val_str, x, y);
  }

  // weapon pad
  if (equip_inc.contains(EquipGameInstance::IncType::kWeaponPad) ||
      scroll_inc.contains(EquipGameInstance::IncType::kWeaponPad)) {
    auto str_node =
        WzResource::ms->get_root()->find(u"String.img/Equip/weaponAttack");
    auto pad_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kWeaponPad];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kWeaponPad)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kWeaponPad);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    render_equip_bottom_inc(
        pad_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  // weapon pdd
  if (equip_inc.contains(EquipGameInstance::IncType::kPdd) ||
      scroll_inc.contains(EquipGameInstance::IncType::kPdd)) {
    str_node = WzResource::ms->get_root()->find(u"String.img/Character/pdd");
    auto pdd_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kPdd];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kPdd)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kPdd);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    render_equip_bottom_inc(
        pdd_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  // weapon mad
  if (equip_inc.contains(EquipGameInstance::IncType::kWeaponMad) ||
      scroll_inc.contains(EquipGameInstance::IncType::kWeaponMad)) {
    str_node =
        WzResource::ms->get_root()->find(u"String.img/Equip/weaponMagic");
    auto mad_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kWeaponMad];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kWeaponMad)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kWeaponMad);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    render_equip_bottom_inc(
        mad_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  // acc
  if (equip_inc.contains(EquipGameInstance::IncType::kAcc) ||
      scroll_inc.contains(EquipGameInstance::IncType::kAcc)) {
    str_node = WzResource::ms->get_root()->find(u"String.img/Character/acc");
    auto acc_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kAcc];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kAcc)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kAcc);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    render_equip_bottom_inc(
        acc_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(EquipGameInstance::IncType::kStr) ||
      scroll_inc.contains(EquipGameInstance::IncType::kStr)) {
    auto str_node =
        WzResource::ms->get_root()->find(u"String.img/Character/str");
    auto str_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kStr];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kStr)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kStr);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    render_equip_bottom_inc(
        str_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(EquipGameInstance::IncType::kDex) ||
      scroll_inc.contains(EquipGameInstance::IncType::kDex)) {
    auto str_node =
        WzResource::ms->get_root()->find(u"String.img/Character/dex");
    auto dex_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kDex];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kDex)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kDex);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    render_equip_bottom_inc(
        dex_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(EquipGameInstance::IncType::kInt) ||
      scroll_inc.contains(EquipGameInstance::IncType::kInt)) {
    auto str_node =
        WzResource::ms->get_root()->find(u"String.img/Character/int");
    auto int_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kInt];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kInt)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kInt);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    render_equip_bottom_inc(
        int_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(EquipGameInstance::IncType::kSpeed) ||
      scroll_inc.contains(EquipGameInstance::IncType::kSpeed)) {
    auto str_node =
        WzResource::ms->get_root()->find(u"String.img/Character/speed");
    auto spd_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc[EquipGameInstance::IncType::kSpeed];
    std::string val_str = "+";
    if (scroll_inc.contains(EquipGameInstance::IncType::kSpeed)) {
      auto val2 = scroll_inc.at(EquipGameInstance::IncType::kSpeed);
      val_str = std::format("+{}({}+{})", val + val2, val, val2);
    } else {
      val_str = val_str + std::to_string(val);
    }
    render_equip_bottom_inc(
        spd_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(EquipGameInstance::IncType::kJump) ||
      scroll_inc.contains(EquipGameInstance::IncType::kJump)) {
    auto str_node =
        WzResource::ms->get_root()->find(u"String.img/Character/jump");
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
    render_equip_bottom_inc(
        jump_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }
  // enh
  str_node = WzResource::ms->get_root()->find(
      u"String.img/Equip/remainingEnhancements");
  auto enh_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
  auto val = EquipGameInstance::load_equip_tuc(id) - equip.scroll.size();
  auto val_str = std::to_string(val);
  render_equip_bottom_inc(enh_str,
                          std::u16string{val_str.begin(), val_str.end()}, x, y);
}

void TooltipUiSystem::render_equip_req(uint32_t req, uint32_t val,
                                         const std::u16string &path, float x,
                                         float y) {
  static auto equip_node = WzResource::ui->find(u"UIToolTip.img/Item/Equip");
  auto color_node = equip_node;
  SDL_Texture *texture;
  if (req == 0) {
    texture = WzResource::load_texture(equip_node->find(u"Disabled/" + path));
    color_node = equip_node->get_child(u"Disabled");
  } else if (req >= val) {
    texture = WzResource::load_texture(equip_node->find(u"Cannot/" + path));
    color_node = equip_node->get_child(u"Cannot");
  } else {
    texture = WzResource::load_texture(equip_node->find(u"Can/" + path));
    color_node = equip_node->get_child(u"Can");
  }
  SDL_FRect pos_rect = {
      x,
      y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);

  auto req_str = std::to_string(req);
  uint32_t w = 0;
  for (auto c : req_str) {
    auto num_node = color_node->get_child(std::string{c});
    auto num_texture = WzResource::load_texture(num_node);
    auto num_origin = WzResource::load_fpoint(num_node->get_child(u"origin"));
    SDL_FRect r = {
        x + 54 + w - num_origin.x,
        pos_rect.y,
        static_cast<float>(num_texture->w),
        static_cast<float>(num_texture->h),
    };
    SDL_RenderTexture(Window::renderer, num_texture, nullptr, &r);
    w += num_texture->w + 1 - num_origin.x;
  }
}

void TooltipUiSystem::render_equip_job(const std::u16string &id, float x,
                                         float y) {
  // job
  auto jobs = EquipGameInstance::load_equip_job(id);
  static auto job_node = WzResource::ui->find(u"UIToolTip.img/Item/Equip/Job");
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
    auto origin = WzResource::load_fpoint(node->get_child(u"origin"));
    auto texture = WzResource::load_texture(node);
    pos_rect.x = x - origin.x;
    pos_rect.y = y - origin.y;
    pos_rect.w = texture->w;
    pos_rect.h = texture->h;
    SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
  }
}

void TooltipUiSystem::render_deco(GameDecoItem &deco, float x, float y) {
  // backgrnd
  auto w = load_equip_width(deco.id);
  const auto h = 160 + load_deco_bottom_h(deco.id);
  render_backgrnd(x, y, w, h);

  //   render
  Freetype::load_color(255, 255, 255, 255);
  Freetype::load_aligned(true);

  static auto dot0 = WzResource::load_texture(
      WzResource::ui->find(u"Tooltip.img/Equip/Dot/0"));

  SDL_FRect pos_rect = {
      x + 10,
      y + 20,
      static_cast<float>(dot0->w),
      static_cast<float>(dot0->h),
  };
  SDL_RenderTexture(Window::renderer, dot0, nullptr, &pos_rect);

  auto equip_name = EquipGameInstance::load_equip_name(deco.id);
  Freetype::load_bold(true);
  Freetype::load_size(15);
  Freetype::draw_line(equip_name, x + 20, y + 10);
  Freetype::load_bold(false);

  static auto item_base = WzResource::load_texture(
      WzResource::ui->find(u"UIToolTip.img/Item/ItemIcon/base"));
  SDL_FPoint base{10, 35};
  pos_rect = {x + base.x, y + base.y, static_cast<float>(item_base->w),
              static_cast<float>(item_base->h)};
  SDL_RenderTexture(Window::renderer, item_base, nullptr, &pos_rect);

  static auto item_cover = WzResource::load_texture(
      WzResource::ui->find(u"UIToolTip.img/Item/ItemIcon/cover"));
  pos_rect = {
      x + base.x + 4,
      y + base.y + 4,
      static_cast<float>(item_cover->w),
      static_cast<float>(item_cover->h),
  };
  SDL_RenderTexture(Window::renderer, item_cover, nullptr, &pos_rect);

  auto equip_info = EquipGameInstance::load_equip_info(deco.id);
  auto eqp_texture = WzResource::load_texture(equip_info->get_child(u"icon"));
  pos_rect.w = eqp_texture->w * 2;
  pos_rect.h = eqp_texture->h * 2;
  auto dx = 74 - pos_rect.w;
  auto dy = 74 - pos_rect.h;
  pos_rect.x += dx / 2;
  pos_rect.y += dy / 2;
  SDL_RenderTexture(Window::renderer, eqp_texture, nullptr, &pos_rect);

  auto cash_icon = WzResource::load_texture(
      WzResource::ms->get_root()->find(u"UI.img/CashItem"));
  pos_rect = {
      x + base.x + 4 + 46,
      y + base.y + 4 + 46,
      static_cast<float>(cash_icon->w) * 2,
      static_cast<float>(cash_icon->h) * 2,
  };
  SDL_RenderTexture(Window::renderer, cash_icon, nullptr, &pos_rect);

  auto req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLevel"))
          ->get();
  auto val = CharacterGameInstance::self.level;
  render_equip_req(req, val, u"reqLEV", x + base.x + 95, y + base.y + 6);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqSTR"))->get();
  val = CharacterStatGameInstance::get_str_ap();
  render_equip_req(req, val, u"reqSTR", x + base.x + 95, y + base.y + 18);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqDEX"))->get();
  val = CharacterStatGameInstance::get_dex_ap();
  render_equip_req(req, val, u"reqDEX", x + base.x + 95, y + base.y + 30);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqINT"))->get();
  val = CharacterStatGameInstance::get_int_ap();
  render_equip_req(req, val, u"reqINT", x + base.x + 95, y + base.y + 42);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLUK"))->get();
  val = CharacterStatGameInstance::get_luk_ap();
  render_equip_req(req, val, u"reqLUK", x + base.x + 95, y + base.y + 54);

  req = 0;
  val = 0;
  render_equip_req(req, val, u"reqPOP", x + base.x + 95, y + base.y + 66);

  render_equip_job(deco.id, x + base.x, y + base.y + 80);

  auto line_y = y + base.y + 112;
  SDL_SetRenderDrawColor(Window::renderer, 255, 255, 255, 255);
  SDL_RenderLine(Window::renderer, x + 5, line_y, x + w - 8, line_y);

  Freetype::load_aligned(false);

  render_deco_bottom(deco, x + 22, line_y + 5);
}

void TooltipUiSystem::render_equip(GameEquipItem &equip, float x, float y) {
  // backgrnd
  static auto backgrnd_node = WzResource::ui->find(u"UIToolTip.img/Item");
  static auto top =
      WzResource::load_texture(backgrnd_node->find(u"Frame/top"));
  static auto line =
      WzResource::load_texture(backgrnd_node->find(u"Frame/line"));
  static auto bottom =
      WzResource::load_texture(backgrnd_node->find(u"Frame/bottom"));
  const auto h = 180 + load_equip_bottom_h(equip);
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(top->w),
      static_cast<float>(top->h),
  };
  SDL_RenderTexture(Window::renderer, top, nullptr, &pos_rect);

  pos_rect.y = y + h - bottom->h;
  pos_rect.h = bottom->h;
  SDL_RenderTexture(Window::renderer, bottom, nullptr, &pos_rect);

  pos_rect.y = y + top->h;
  pos_rect.h = h - top->h - bottom->h;
  SDL_RenderTexture(Window::renderer, line, nullptr, &pos_rect);

  //   render
  Freetype::load_color(255, 255, 255, 255);
  Freetype::load_aligned(true);

  static auto dot0 = WzResource::load_texture(
      WzResource::ui->find(u"Tooltip.img/Equip/Dot/0"));

  pos_rect = {x + 10, y + 20, static_cast<float>(dot0->w),
              static_cast<float>(dot0->h)};
  SDL_RenderTexture(Window::renderer, dot0, nullptr, &pos_rect);

  auto equip_name = EquipGameInstance::load_equip_name(equip.id);
  int scroll_num = 0;
  for (auto &scroll : equip.scroll) {
    if (scroll.success) {
      scroll_num++;
    }
  }
  Freetype::load_bold(true);
  Freetype::load_size(15);
  if (scroll_num > 0) {
    auto tmp = std::to_string(scroll_num);
    equip_name =
        equip_name + u"(+" + std::u16string{tmp.begin(), tmp.end()} + u")";
  }
  Freetype::draw_line(equip_name, x + 20, y + 10);
  Freetype::load_bold(false);

  static auto item_base = WzResource::load_texture(
      WzResource::ui->find(u"UIToolTip.img/Item/ItemIcon/base"));
  SDL_FPoint base{10, 35};
  pos_rect = {x + base.x, y + base.y, static_cast<float>(item_base->w),
              static_cast<float>(item_base->h)};
  SDL_RenderTexture(Window::renderer, item_base, nullptr, &pos_rect);

  static auto item_cover = WzResource::load_texture(
      WzResource::ui->find(u"UIToolTip.img/Item/ItemIcon/cover"));
  pos_rect = {x + base.x + 4, y + base.y + 4, static_cast<float>(item_cover->w),
              static_cast<float>(item_cover->h)};
  SDL_RenderTexture(Window::renderer, item_cover, nullptr, &pos_rect);

  auto equip_info = EquipGameInstance::load_equip_info(equip.id);
  auto eqp_texture = WzResource::load_texture(equip_info->get_child(u"icon"));
  pos_rect.w = eqp_texture->w * 2;
  pos_rect.h = eqp_texture->h * 2;
  auto dx = 74 - pos_rect.w;
  auto dy = 74 - pos_rect.h;
  pos_rect.x += dx / 2;
  pos_rect.y += dy / 2;

  SDL_RenderTexture(Window::renderer, eqp_texture, nullptr, &pos_rect);

  auto req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLevel"))
          ->get();
  auto val = CharacterGameInstance::self.level;
  render_equip_req(req, val, u"reqLEV", x + base.x + 95, y + base.y + 6);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqSTR"))->get();
  val = CharacterStatGameInstance::get_str_ap();
  render_equip_req(req, val, u"reqSTR", x + base.x + 95, y + base.y + 18);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqDEX"))->get();
  val = CharacterStatGameInstance::get_dex_ap();
  render_equip_req(req, val, u"reqDEX", x + base.x + 95, y + base.y + 30);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqINT"))->get();
  val = CharacterStatGameInstance::get_int_ap();
  render_equip_req(req, val, u"reqINT", x + base.x + 95, y + base.y + 42);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLUK"))->get();
  val = CharacterStatGameInstance::get_luk_ap();
  render_equip_req(req, val, u"reqLUK", x + base.x + 95, y + base.y + 54);

  req = 0;
  val = 0;
  render_equip_req(req, val, u"reqPOP", x + base.x + 95, y + base.y + 66);

  render_equip_job(equip.id, x + base.x, y + base.y + 80);

  auto line_y = y + base.y + 112;
  SDL_SetRenderDrawColor(Window::renderer, 255, 255, 255, 255);
  SDL_RenderLine(Window::renderer, x + 5, line_y, x + 230, line_y);

  Freetype::load_aligned(false);

  render_equip_bottom(equip, x + 22, line_y + 5);
}

void TooltipUiSystem::render_backgrnd(float x, float y, float w, float h) {
  // backgrnd
  static auto backgrnd_node =
      WzResource::ui->find(u"UIToolTip.img/Skill/Frame");

  auto texture_c = WzResource::load_texture(backgrnd_node->get_child(u"c"));
  auto texture_e =
      WzResource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"e")));
  auto texture_ne =
      WzResource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"ne")));
  auto texture_n =
      WzResource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"n")));
  auto texture_nw =
      WzResource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"nw")));
  auto texture_w =
      WzResource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"w")));
  auto texture_sw =
      WzResource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"sw")));
  auto texture_s =
      WzResource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"s")));
  auto texture_se =
      WzResource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
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
  SDL_RenderTexture(Window::renderer, texture_nw, nullptr, &rect);

  // 左下
  SDL_FPoint p_sw{
      x,
      y + h - texture_sw->h,
  };
  rect.x = (p_sw.x);
  rect.y = (p_sw.y);
  rect.w = texture_sw->w;
  rect.h = texture_sw->h;
  SDL_RenderTexture(Window::renderer, texture_sw, nullptr, &rect);

  // 右上
  SDL_FPoint p_ne{
      x + w - texture_ne->w,
      y,
  };
  rect.x = (p_ne.x);
  rect.y = (p_ne.y);
  rect.w = texture_ne->w;
  rect.h = texture_ne->h;
  SDL_RenderTexture(Window::renderer, texture_ne, nullptr, &rect);

  // 右下
  SDL_FPoint p_se{
      x + w - texture_ne->w,
      y + h - texture_sw->h,
  };
  rect.x = (p_se.x);
  rect.y = (p_se.y);
  rect.w = texture_se->w;
  rect.h = texture_se->h;
  SDL_RenderTexture(Window::renderer, texture_se, nullptr, &rect);

  rect.x = (x + texture_nw->w);
  rect.y = (y + texture_nw->h);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = p_sw.y - (p_nw.y + texture_nw->h);
  SDL_RenderTextureTiled(Window::renderer, texture_c, nullptr, 1, &rect);

  // 竖着的两边
  rect.x = (p_nw.x);
  rect.y = (p_nw.y + texture_nw->h);
  rect.w = texture_w->w;
  rect.h = p_sw.y - (p_nw.y + texture_nw->h);
  SDL_RenderTextureTiled(Window::renderer, texture_w, nullptr, 1, &rect);

  rect.x = (p_se.x);
  rect.y = (p_ne.y + texture_ne->h);
  rect.w = texture_e->w;
  rect.h = p_se.y - (p_ne.y + texture_ne->h);
  SDL_RenderTextureTiled(Window::renderer, texture_e, nullptr, 1, &rect);

  // 横着的两边
  rect.x = (p_nw.x + texture_nw->w);
  rect.y = (p_ne.y);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = texture_n->h;
  SDL_RenderTextureTiled(Window::renderer, texture_n, nullptr, 1, &rect);

  rect.x = (p_nw.x + texture_nw->w);
  rect.y = (p_sw.y);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = texture_s->h;
  SDL_RenderTextureTiled(Window::renderer, texture_s, nullptr, 1, &rect);
}

float TooltipUiSystem::load_skill_bottom_h(const std::u16string &id) {
  auto ski_name = SkillGameInstance::load_ski_name(id);
  auto ski_level = JobSkillGameInstance::load_ski_level(id);
  float h = 0;
  const auto w = 390;
  Freetype::load_size(12);
  if (ski_level == 0 || ski_level == ski_name.level.size()) {
    ski_level =
        std::clamp(ski_level, (uint8_t)0, (uint8_t)(ski_name.level.size() - 1));
    auto ski_l0 = ski_name.level[ski_level];
    h += Freetype::load_lh() * 1.1;
    h += Freetype::load_rh(ski_l0, w - 28, 1.3, std::nullopt);
  } else {
    auto ski_l0 = ski_name.level[ski_level - 1];
    auto ski_l1 = ski_name.level[ski_level];
    h += Freetype::load_lh() * 2 * 1.1;
    h += Freetype::load_rh(ski_l0, w - 28, 1.3, std::nullopt);
    h += Freetype::load_rh(ski_l1, w - 28, 1.3, std::nullopt);
    h += 5;
  }
  return h;
}

void TooltipUiSystem::render_skill_bottom(const std::u16string &id,
                                            uint8_t level, float x, float y) {
  const auto w = 390;
  auto ski_name = SkillGameInstance::load_ski_name(id);
  auto ski_level = JobSkillGameInstance::load_ski_level(id);
  auto ski_level2 = std::to_string(ski_level);
  std::u16string ski_level3{ski_level2.begin(), ski_level2.end()};

  auto next_level = ski_level + 1;
  auto next_level2 = std::to_string(next_level);
  std::u16string next_level3{next_level2.begin(), next_level2.end()};

  if (ski_level == 0) {
    Freetype::load_size(12);
    Freetype::load_color(255, 255, 255, 255);
    auto next_node =
        WzResource::ms->get_root()->find(u"String.img/Skill/nextLevel");
    auto str = static_cast<wz::Property<std::u16string> *>(next_node)->get();

    str = u"[" + str + u" " + next_level3 + u"]";
    Freetype::draw_line(str, x, y);

    str = ski_name.level[ski_level];
    y += Freetype::load_lh() * 1.3;
    Freetype::draw_rstr(str, x + 2, y, w - 22, 1.3, std::nullopt);
  } else if (ski_level == ski_name.level.size()) {
    Freetype::load_size(12);
    Freetype::load_color(255, 255, 255, 255);
    auto cur_node =
        WzResource::ms->get_root()->find(u"String.img/Skill/currentLevel");
    auto str = static_cast<wz::Property<std::u16string> *>(cur_node)->get();

    str = u"[" + str + u" " + ski_level3 + u"]";
    Freetype::draw_line(str, x, y);

    str = ski_name.level[ski_level - 1];
    y += Freetype::load_lh() * 1.3;
    Freetype::draw_rstr(str, x + 2, y, w - 22, 1.3, std::nullopt);
  } else {
    Freetype::load_size(12);
    Freetype::load_color(255, 255, 255, 255);
    auto cur_node =
        WzResource::ms->get_root()->find(u"String.img/Skill/currentLevel");
    auto str = static_cast<wz::Property<std::u16string> *>(cur_node)->get();

    str = u"[" + str + u" " + ski_level3 + u"]";
    Freetype::draw_line(str, x, y);

    str = ski_name.level[ski_level - 1];
    y += Freetype::load_lh() * 1.3;
    auto lh =
        Freetype::draw_rstr(str, x + 2, y, w - 28, 1.3, std::nullopt).height;

    y += lh;

    auto next_node =
        WzResource::ms->get_root()->find(u"String.img/Skill/nextLevel");
    str = static_cast<wz::Property<std::u16string> *>(next_node)->get();

    Freetype::load_color(255, 255, 255, 255);
    str = u"[" + str + u" " + next_level3 + u"]";
    Freetype::draw_line(str, x, y);
    str = ski_name.level[ski_level];
    y += Freetype::load_lh() * 1.3;
    Freetype::draw_rstr(str, x + 2, y, w - 28, 1.3, std::nullopt);
  }
}

void TooltipUiSystem::render_skill(const std::u16string &id, uint8_t level,
                                     float x, float y) {
  auto ski_name = SkillGameInstance::load_ski_name(id);
  const auto w = 390;
  Freetype::load_size(12);
  auto desc = ski_name.desc;
  auto rh = Freetype::load_rh(desc, w - 28, 1.2, SDL_FRect{0, 0, 74, 74});
  auto h = std::max(140.0f, rh + 60);
  h += load_skill_bottom_h(id);
  render_backgrnd(x, y, w, h);
  static auto dot0 = WzResource::load_texture(
      WzResource::ui->find(u"Tooltip.img/Equip/Dot/0"));

  SDL_FRect pos_rect = {x + 10, y + 20, static_cast<float>(dot0->w),
                        static_cast<float>(dot0->h)};
  SDL_RenderTexture(Window::renderer, dot0, nullptr, &pos_rect);

  static auto item_base = WzResource::load_texture(
      WzResource::ui->find(u"UIToolTip.img/Item/ItemIcon/base"));
  SDL_FPoint base{10, 35};
  pos_rect = {x + base.x, y + base.y, static_cast<float>(item_base->w),
              static_cast<float>(item_base->h)};
  SDL_RenderTexture(Window::renderer, item_base, nullptr, &pos_rect);

  Freetype::load_color(255, 255, 255, 255);
  Freetype::load_aligned(true);
  Freetype::load_bold(true);
  Freetype::load_size(15);
  Freetype::draw_line(ski_name.name, x + 20, y + 10);
  Freetype::load_bold(false);

  auto ski_node = SkillGameInstance::load_ski_node(id);
  auto ski_texture = WzResource::load_texture(ski_node->get_child(u"icon"));
  pos_rect.w = ski_texture->w * 2;
  pos_rect.h = ski_texture->h * 2;
  auto dx = 74 - pos_rect.w;
  auto dy = 74 - pos_rect.h;
  pos_rect.x += 4 + dx / 2;
  pos_rect.y += 4 + dy / 2;

  SDL_RenderTexture(Window::renderer, ski_texture, nullptr, &pos_rect);

  Freetype::load_size(12);
  Freetype::load_color(255, 255, 255, 255);
  auto sx = pos_rect.x;
  auto sy = pos_rect.y - 8;
  auto rstr_h =
      Freetype::draw_rstr(desc, sx, sy, w - 28, 1.2, SDL_FRect{sx, sy, 74, 74})
          .height;

  auto line_y = std::max(sy + rstr_h + 10, y + base.y + 90);
  SDL_SetRenderDrawColor(Window::renderer, 255, 255, 255, 255);
  SDL_RenderLine(Window::renderer, x + 5, line_y, x + w - 5, line_y);

  render_skill_bottom(id, level, x + 14, line_y + 5);
}

void TooltipUiSystem::render_item(GameItem &item, float x, float y) {
  auto item_name = ItemGameInstance::load_item_text(item.id, u"name");
  Freetype::load_bold(true);
  Freetype::load_size(15);
  auto w = Freetype::load_w(item_name) + 150;
  w = std::max(w, 360.0f);
  render_backgrnd(x, y, w, 128);
  Freetype::load_color(255, 255, 255, 255);
  Freetype::load_aligned(true);
  Freetype::draw_line(item_name, x + 20, y + 10);
  static auto dot0 = WzResource::load_texture(
      WzResource::ui->find(u"Tooltip.img/Equip/Dot/0"));

  SDL_FRect pos_rect = {x + 10, y + 20, static_cast<float>(dot0->w),
                        static_cast<float>(dot0->h)};
  SDL_RenderTexture(Window::renderer, dot0, nullptr, &pos_rect);

  static auto item_base = WzResource::load_texture(
      WzResource::ui->find(u"UIToolTip.img/Item/ItemIcon/base"));
  SDL_FPoint base{10, 35};
  pos_rect = {x + base.x, y + base.y, static_cast<float>(item_base->w),
              static_cast<float>(item_base->h)};
  SDL_RenderTexture(Window::renderer, item_base, nullptr, &pos_rect);

  static auto item_cover = WzResource::load_texture(
      WzResource::ui->find(u"UIToolTip.img/Item/ItemIcon/cover"));
  pos_rect = {x + base.x + 4, y + base.y + 4, static_cast<float>(item_cover->w),
              static_cast<float>(item_cover->h)};
  SDL_RenderTexture(Window::renderer, item_cover, nullptr, &pos_rect);

  auto item_info = ItemGameInstance::load_item_info(item.id, 0);
  auto item_texture = WzResource::load_texture(item_info->get_child(u"icon"));
  pos_rect.w = item_texture->w * 2;
  pos_rect.h = item_texture->h * 2;
  auto dx = 74 - pos_rect.w;
  auto dy = 74 - pos_rect.h;
  pos_rect.x += dx / 2;
  pos_rect.y += dy / 2;

  SDL_RenderTexture(Window::renderer, item_texture, nullptr, &pos_rect);
  Freetype::load_bold(false);

  auto item_desc = ItemGameInstance::load_item_text(item.id, u"desc");
  Freetype::load_size(13);
  Freetype::draw_rstr(item_desc, x + base.x + 84, y + base.y - 2, w - 112, 1.1,
                      std::nullopt);
}

void TooltipUiSystem::render_world_map_info(uint32_t id, float x, float y) {
  auto MapName = MinimapUiSystem::load_map_name(id).map_name;
  auto street_name = MinimapUiSystem::load_map_name(id).street_name;
  auto map_desc = MinimapUiSystem::load_map_name(id).map_desc;
  Freetype::load_size(14);
  const auto bh = 60;
  const auto w = 300;
  if (map_desc != u"") {
    auto h = Freetype::load_rh(map_desc, 280, 1.1, std::nullopt);
    render_backgrnd(x, y, w, h + bh + 20);
  } else {
    render_backgrnd(x, y, w, bh);
  }
  Freetype::load_color(240, 224, 104, 255);
  Freetype::load_aligned(true);
  auto s1 = street_name;
  auto sw = Freetype::load_w(s1);
  Freetype::draw_line(s1, x + (w - sw) / 2, y + 8);

  auto s2 = MapName;
  sw = Freetype::load_w(s2);
  Freetype::load_color(255, 255, 255, 255);
  auto sh = y + Freetype::load_lh() + 12;
  Freetype::draw_line(s2, x + (w - sw) / 2, sh);

  if (map_desc != u"") {
    SDL_SetRenderDrawColor(Window::renderer, 240, 224, 104, 255);
    sh += 25;
    SDL_RenderLine(Window::renderer, x, sh, x + w, sh);

    auto s3 = map_desc;
    Freetype::draw_cstr(s3, x + 10, sh + 5, w - 20, 1.1);
  }
}

void TooltipUiSystem::render_str(const std::u16string &str, float x,
                                   float y) {
  const auto w = Freetype::load_w(str) + 12;
  Freetype::load_size(12);
  auto h = Freetype::load_lh();
  render_backgrnd(x, y, w, 32);
  Freetype::load_aligned(true);
  Freetype::load_color(255, 255, 255, 255);
  Freetype::draw_line(str, x + 5, y + 5);
}