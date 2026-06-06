#include "tooltip_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/system/ui/minimap_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <string>

float tooltip_ui_system::load_equip_bottom_h(const std::u16string &id) {
  auto equip_inc = equip_game_instance::load_equip_inc(id);
  freetype::load_size(12);
  auto h = freetype::load_lh();
  return h * (equip_inc.size() + 1);
}

void tooltip_ui_system::render_equip_bottom_inc(std::u16string label,
                                                std::u16string val, float x,
                                                float &y) {
  static auto dot1 = wz_resource::load_texture(
      wz_resource::ui->find(u"Tooltip.img/Equip/Dot/1"));
  SDL_FRect pos_rect{
      x - 10,
      y + 9,
      static_cast<float>(dot1->w),
      static_cast<float>(dot1->h),
  };
  SDL_RenderTexture(window::renderer, dot1, nullptr, &pos_rect);
  // static auto
  freetype::load_aligned(true);
  freetype::load_color(240, 224, 104, 255);
  label += u":";
  freetype::draw_line(label, x, y);
  auto w = freetype::load_w(label);
  freetype::load_color(255, 255, 255, 255);
  freetype::draw_line(val, x + w + 2, y);
  auto h = freetype::load_lh();
  y += h + 1;
}

void tooltip_ui_system::render_equip_bottom(game_equip &equip, float x,
                                            float y) {
  auto id = equip.id;
  auto equip_inc = equip_game_instance::load_equip_inc(id);
  freetype::load_size(12);
  auto h = freetype::load_lh();
  //   type
  auto equip_type = equip_game_instance::load_equip_type(id);
  if (equip_type == u"Weapon") {
    static auto weapon_node =
        wz_resource::string2->get_root()->find(u"Equip.img/Weapon/Type");
    auto sub_id = id.substr(0, 4);
    equip_type = static_cast<wz::Property<std::u16string> *>(
                     weapon_node->get_child(sub_id))
                     ->get();
  }
  auto str_node = wz_resource::string2->get_root()->find(u"Equip.img/TYPE");
  auto type_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
  render_equip_bottom_inc(type_str, equip_type, x, y);
  // weapon speed
  if (equip_inc.contains(equip_game_instance::inc_type::WEAPON_SPEED)) {
    auto str_node =
        wz_resource::string2->get_root()->find(u"Equip.img/WEAPON_SPEED");
    auto spd_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    std::u16string val_str;
    auto val = equip_inc.at(equip_game_instance::inc_type::WEAPON_SPEED);
    if (val >= 6) {
      // slow
      str_node = wz_resource::string2->get_root()->find(
          u"Equip.img/Weapon/Speed/SLOW");
      val_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    } else if (val > 4) {
      // Normal
      str_node = wz_resource::string2->get_root()->find(
          u"Equip.img/Weapon/Speed/NORMAL");
      val_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    } else {
      // Fast
      str_node = wz_resource::string2->get_root()->find(
          u"Equip.img/Weapon/Speed/FAST");
      val_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    }
    render_equip_bottom_inc(spd_str, val_str, x, y);
  }

  // weapon pad
  if (equip_inc.contains(equip_game_instance::inc_type::WEAPON_PAD)) {
    auto str_node =
        wz_resource::string2->get_root()->find(u"Equip.img/WEAPON_PAD");
    auto pad_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc.at(equip_game_instance::inc_type::WEAPON_PAD);
    auto val_str = "+" + std::to_string(val);
    render_equip_bottom_inc(
        pad_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  // weapon pad
  if (equip_inc.contains(equip_game_instance::inc_type::PDD)) {
    str_node = wz_resource::string2->get_root()->find(u"Character.img/PDD");
    auto pdd_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc.at(equip_game_instance::inc_type::PDD);
    auto val_str = "+" + std::to_string(val);
    render_equip_bottom_inc(
        pdd_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  // weapon mad
  if (equip_inc.contains(equip_game_instance::inc_type::WEAPON_MAD)) {
    str_node = wz_resource::string2->get_root()->find(u"Equip.img/WEAPON_MAD");
    auto mad_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc.at(equip_game_instance::inc_type::WEAPON_MAD);
    auto val_str = "+" + std::to_string(val);
    render_equip_bottom_inc(
        mad_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  // acc
  if (equip_inc.contains(equip_game_instance::inc_type::ACC)) {
    str_node = wz_resource::string2->get_root()->find(u"Character.img/ACC");
    auto acc_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc.at(equip_game_instance::inc_type::ACC);
    auto val_str = "+" + std::to_string(val);
    render_equip_bottom_inc(
        acc_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(equip_game_instance::inc_type::STR)) {
    auto str_node =
        wz_resource::string2->get_root()->find(u"Character.img/STR");
    auto str_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc.at(equip_game_instance::inc_type::STR);
    auto val_str = "+" + std::to_string(val);
    render_equip_bottom_inc(
        str_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(equip_game_instance::inc_type::DEX)) {
    auto str_node =
        wz_resource::string2->get_root()->find(u"Character.img/DEX");
    auto dex_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc.at(equip_game_instance::inc_type::DEX);
    auto val_str = "+" + std::to_string(val);
    render_equip_bottom_inc(
        dex_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(equip_game_instance::inc_type::INT)) {
    auto str_node =
        wz_resource::string2->get_root()->find(u"Character.img/INT");
    auto int_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc.at(equip_game_instance::inc_type::INT);
    auto val_str = "+" + std::to_string(val);
    render_equip_bottom_inc(
        int_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(equip_game_instance::inc_type::SPEED)) {
    auto str_node =
        wz_resource::string2->get_root()->find(u"Character.img/SPEED");
    auto spd_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc.at(equip_game_instance::inc_type::SPEED);
    auto val_str = "+" + std::to_string(val);
    render_equip_bottom_inc(
        spd_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }

  if (equip_inc.contains(equip_game_instance::inc_type::JUMP)) {
    auto str_node =
        wz_resource::string2->get_root()->find(u"Character.img/JUMP");
    auto jump_str =
        static_cast<wz::Property<std::u16string> *>(str_node)->get();
    auto val = equip_inc.at(equip_game_instance::inc_type::JUMP);
    auto val_str = "+" + std::to_string(val);
    render_equip_bottom_inc(
        jump_str, std::u16string{val_str.begin(), val_str.end()}, x, y);
  }
  // enh
  str_node = wz_resource::string2->get_root()->find(u"Equip.img/REMAIN_ENH");
  auto enh_str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
  auto val = equip_game_instance::load_equip_tuc(id) - equip.scroll.size();
  auto val_str = std::to_string(val);
  render_equip_bottom_inc(enh_str,
                          std::u16string{val_str.begin(), val_str.end()}, x, y);

  freetype::load_aligned(false);
}

void tooltip_ui_system::render_equip_req(uint32_t req, uint32_t val,
                                         const std::u16string &path, float x,
                                         float y) {
  static auto equip_node = wz_resource::ui->find(u"UIToolTip.img/Item/Equip");
  auto color_node = equip_node;
  SDL_Texture *texture;
  if (req == 0) {
    texture = wz_resource::load_texture(equip_node->find(u"Disabled/" + path));
    color_node = equip_node->get_child(u"Disabled");
  } else if (req >= val) {
    texture = wz_resource::load_texture(equip_node->find(u"Cannot/" + path));
    color_node = equip_node->get_child(u"Cannot");
  } else {
    texture = wz_resource::load_texture(equip_node->find(u"Can/" + path));
    color_node = equip_node->get_child(u"Can");
  }
  SDL_FRect pos_rect = {
      x,
      y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);

  auto req_str = std::to_string(req);
  uint32_t w = 0;
  for (auto c : req_str) {
    auto num_node = color_node->get_child(std::string{c});
    auto num_texture = wz_resource::load_texture(num_node);
    auto num_origin = wz_resource::load_fpoint(num_node->get_child(u"origin"));
    SDL_FRect r = {
        x + 54 + w - num_origin.x,
        pos_rect.y,
        static_cast<float>(num_texture->w),
        static_cast<float>(num_texture->h),
    };
    SDL_RenderTexture(window::renderer, num_texture, nullptr, &r);
    w += num_texture->w + 1 - num_origin.x;
  }
}

void tooltip_ui_system::render_equip_job(const std::u16string &id, float x,
                                         float y) {
  // job
  auto jobs = equip_game_instance::load_equip_job(id);
  static auto job_node = wz_resource::ui->find(u"UIToolTip.img/Item/Equip/Job");
  wz::Node *node;
  SDL_FRect pos_rect;
  for (auto i : {
           equip_game_instance::job_type::BEGINNER,
           equip_game_instance::job_type::WARRIOR,
           equip_game_instance::job_type::MAGICIAN,
           equip_game_instance::job_type::BOWMAN,
           equip_game_instance::job_type::THIEF,
       }) {
    if (jobs.contains(i)) {
      node = job_node->get_child(u"enable");
    } else {
      node = job_node->get_child(u"disable");
    }
    node = node->get_child(std::to_string((uint8_t)i));
    auto origin = wz_resource::load_fpoint(node->get_child(u"origin"));
    auto texture = wz_resource::load_texture(node);
    pos_rect.x = x - origin.x;
    pos_rect.y = y - origin.y;
    pos_rect.w = texture->w;
    pos_rect.h = texture->h;
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }
}

void tooltip_ui_system::render_equip(game_equip &equip, float x, float y) {
  // backgrnd
  static auto backgrnd_node = wz_resource::ui->find(u"UIToolTip.img/Item");
  static auto top =
      wz_resource::load_texture(backgrnd_node->find(u"Frame/top"));
  static auto line =
      wz_resource::load_texture(backgrnd_node->find(u"Frame/line"));
  static auto bottom =
      wz_resource::load_texture(backgrnd_node->find(u"Frame/bottom"));
  const auto h = 180 + load_equip_bottom_h(equip.id);
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(top->w),
      static_cast<float>(top->h),
  };
  SDL_RenderTexture(window::renderer, top, nullptr, &pos_rect);

  pos_rect.y = y + h - bottom->h;
  pos_rect.h = bottom->h;
  SDL_RenderTexture(window::renderer, bottom, nullptr, &pos_rect);

  pos_rect.y = y + top->h;
  pos_rect.h = h - top->h - bottom->h;
  SDL_RenderTexture(window::renderer, line, nullptr, &pos_rect);

  //   render
  freetype::load_color(255, 255, 255, 255);
  freetype::load_aligned(true);

  static auto dot0 = wz_resource::load_texture(
      wz_resource::ui->find(u"Tooltip.img/Equip/Dot/0"));

  pos_rect = {x + 10, y + 20, static_cast<float>(dot0->w),
              static_cast<float>(dot0->h)};
  SDL_RenderTexture(window::renderer, dot0, nullptr, &pos_rect);

  auto equip_name = equip_game_instance::load_equip_name(equip.id);
  freetype::load_bold(true);
  freetype::load_size(15);
  freetype::draw_line(equip_name, x + 20, y + 10);
  freetype::load_bold(false);

  static auto item_base = wz_resource::load_texture(
      wz_resource::ui->find(u"UIToolTip.img/Item/ItemIcon/base"));
  SDL_FPoint base{10, 35};
  pos_rect = {x + base.x, y + base.y, static_cast<float>(item_base->w),
              static_cast<float>(item_base->h)};
  SDL_RenderTexture(window::renderer, item_base, nullptr, &pos_rect);

  static auto item_cover = wz_resource::load_texture(
      wz_resource::ui->find(u"UIToolTip.img/Item/ItemIcon/cover"));
  pos_rect = {x + base.x + 4, y + base.y + 4, static_cast<float>(item_cover->w),
              static_cast<float>(item_cover->h)};
  SDL_RenderTexture(window::renderer, item_cover, nullptr, &pos_rect);

  auto equip_info = equip_game_instance::load_equip_info(equip.id);
  auto eqp_texture = wz_resource::load_texture(equip_info->get_child(u"icon"));
  pos_rect.w = eqp_texture->w * 2;
  pos_rect.h = eqp_texture->h * 2;
  auto dx = 74 - pos_rect.w;
  auto dy = 74 - pos_rect.h;
  pos_rect.x += dx / 2;
  pos_rect.y += dy / 2;

  SDL_RenderTexture(window::renderer, eqp_texture, nullptr, &pos_rect);

  auto req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLevel"))
          ->get();
  auto val = character_game_instance::self.level;
  render_equip_req(req, val, u"reqLEV", x + base.x + 95, y + base.y + 6);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqSTR"))->get();
  val = character_stat_game_instance::str_point;
  render_equip_req(req, val, u"reqSTR", x + base.x + 95, y + base.y + 18);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqDEX"))->get();
  val = character_stat_game_instance::dex_point;
  render_equip_req(req, val, u"reqDEX", x + base.x + 95, y + base.y + 30);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqINT"))->get();
  val = character_stat_game_instance::int_point;
  render_equip_req(req, val, u"reqINT", x + base.x + 95, y + base.y + 42);

  req =
      static_cast<wz::Property<int> *>(equip_info->get_child(u"reqLUK"))->get();
  val = character_stat_game_instance::luk_point;
  render_equip_req(req, val, u"reqLUK", x + base.x + 95, y + base.y + 54);

  req = 0;
  val = 0;
  render_equip_req(req, val, u"reqPOP", x + base.x + 95, y + base.y + 66);

  render_equip_job(equip.id, x + base.x, y + base.y + 80);

  auto line_y = y + base.y + 112;
  SDL_SetRenderDrawColor(window::renderer, 255, 255, 255, 255);
  SDL_RenderLine(window::renderer, x + 5, line_y, x + 230, line_y);

  freetype::load_aligned(false);

  render_equip_bottom(equip, x + 22, line_y + 5);
}

void tooltip_ui_system::render_backgrnd(float x, float y, float w, float h) {
  // backgrnd
  static auto backgrnd_node =
      wz_resource::ui->find(u"UIToolTip.img/Skill/Frame");

  auto texture_c = wz_resource::load_texture(backgrnd_node->get_child(u"c"));
  auto texture_e =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"e")));
  auto texture_ne =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"ne")));
  auto texture_n =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"n")));
  auto texture_nw =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"nw")));
  auto texture_w =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"w")));
  auto texture_sw =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"sw")));
  auto texture_s =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"s")));
  auto texture_se =
      wz_resource::load_texture(static_cast<wz::Property<wz::WzCanvas> *>(
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
  SDL_RenderTexture(window::renderer, texture_nw, nullptr, &rect);

  // 左下
  SDL_FPoint p_sw{
      x,
      y + h - texture_sw->h,
  };
  rect.x = (p_sw.x);
  rect.y = (p_sw.y);
  rect.w = texture_sw->w;
  rect.h = texture_sw->h;
  SDL_RenderTexture(window::renderer, texture_sw, nullptr, &rect);

  // 右上
  SDL_FPoint p_ne{
      x + w - texture_ne->w,
      y,
  };
  rect.x = (p_ne.x);
  rect.y = (p_ne.y);
  rect.w = texture_ne->w;
  rect.h = texture_ne->h;
  SDL_RenderTexture(window::renderer, texture_ne, nullptr, &rect);

  // 右下
  SDL_FPoint p_se{
      x + w - texture_ne->w,
      y + h - texture_sw->h,
  };
  rect.x = (p_se.x);
  rect.y = (p_se.y);
  rect.w = texture_se->w;
  rect.h = texture_se->h;
  SDL_RenderTexture(window::renderer, texture_se, nullptr, &rect);

  rect.x = (x + texture_nw->w);
  rect.y = (y + texture_nw->h);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = p_sw.y - (p_nw.y + texture_nw->h);
  SDL_RenderTextureTiled(window::renderer, texture_c, nullptr, 1, &rect);

  // 竖着的两边
  rect.x = (p_nw.x);
  rect.y = (p_nw.y + texture_nw->h);
  rect.w = texture_w->w;
  rect.h = p_sw.y - (p_nw.y + texture_nw->h);
  SDL_RenderTextureTiled(window::renderer, texture_w, nullptr, 1, &rect);

  rect.x = (p_se.x);
  rect.y = (p_ne.y + texture_ne->h);
  rect.w = texture_e->w;
  rect.h = p_se.y - (p_ne.y + texture_ne->h);
  SDL_RenderTextureTiled(window::renderer, texture_e, nullptr, 1, &rect);

  // 横着的两边
  rect.x = (p_nw.x + texture_nw->w);
  rect.y = (p_ne.y);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = texture_n->h;
  SDL_RenderTextureTiled(window::renderer, texture_n, nullptr, 1, &rect);

  rect.x = (p_nw.x + texture_nw->w);
  rect.y = (p_sw.y);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = texture_s->h;
  SDL_RenderTextureTiled(window::renderer, texture_s, nullptr, 1, &rect);
}

void tooltip_ui_system::render_skill(std::u16string id, uint8_t level, float x,
                                     float y) {}

void tooltip_ui_system::render_item(game_item &item, float x, float y) {
  auto item_name = item_game_instance::load_item_name(item.id);
  freetype::load_bold(true);
  freetype::load_size(15);
  auto w = freetype::load_w(item_name) + 110;
  w = std::max(w, 320.0f);
  render_backgrnd(x, y, w, 128);
  freetype::load_color(255, 255, 255, 255);
  freetype::load_aligned(true);
  freetype::draw_line(item_name, x + 20, y + 10);
  static auto dot0 = wz_resource::load_texture(
      wz_resource::ui->find(u"Tooltip.img/Equip/Dot/0"));

  SDL_FRect pos_rect = {x + 10, y + 20, static_cast<float>(dot0->w),
                        static_cast<float>(dot0->h)};
  SDL_RenderTexture(window::renderer, dot0, nullptr, &pos_rect);

  static auto item_base = wz_resource::load_texture(
      wz_resource::ui->find(u"UIToolTip.img/Item/ItemIcon/base"));
  SDL_FPoint base{10, 35};
  pos_rect = {x + base.x, y + base.y, static_cast<float>(item_base->w),
              static_cast<float>(item_base->h)};
  SDL_RenderTexture(window::renderer, item_base, nullptr, &pos_rect);

  static auto item_cover = wz_resource::load_texture(
      wz_resource::ui->find(u"UIToolTip.img/Item/ItemIcon/cover"));
  pos_rect = {x + base.x + 4, y + base.y + 4, static_cast<float>(item_cover->w),
              static_cast<float>(item_cover->h)};
  SDL_RenderTexture(window::renderer, item_cover, nullptr, &pos_rect);

  auto item_info = item_game_instance::load_item_info(item.id);
  auto item_texture = wz_resource::load_texture(item_info->get_child(u"icon"));
  pos_rect.w = item_texture->w * 2;
  pos_rect.h = item_texture->h * 2;
  auto dx = 74 - pos_rect.w;
  auto dy = 74 - pos_rect.h;
  pos_rect.x += dx / 2;
  pos_rect.y += dy / 2;

  SDL_RenderTexture(window::renderer, item_texture, nullptr, &pos_rect);
  freetype::load_bold(false);

  auto item_desc = item_game_instance::load_item_desc(item.id);
  freetype::load_size(13);
  freetype::draw_str(item_desc, pos_rect.x + 74, pos_rect.y - 8, w - 112);
  freetype::load_aligned(false);
}

void tooltip_ui_system::render_world_map_info(uint32_t id, float x, float y) {
  auto map_name = minimap_ui_system::load_map_name(id).map_name;
  auto street_name = minimap_ui_system::load_map_name(id).street_name;
  auto map_desc = minimap_ui_system::load_map_name(id).map_desc;
  freetype::load_size(14);
  const auto bh = 60;
  const auto w = 300;
  if (map_desc != u"") {
    auto h = freetype::load_h(map_desc, 280);
    render_backgrnd(x, y, w, h + bh + 20);
  } else {
    render_backgrnd(x, y, w, bh);
  }
  freetype::load_color(240, 224, 104, 255);
  freetype::load_aligned(true);
  auto s1 = street_name;
  auto sw = freetype::load_w(s1);
  freetype::draw_line(s1, x + (w - sw) / 2, y + 8);

  auto s2 = map_name;
  sw = freetype::load_w(s2);
  freetype::load_color(255, 255, 255, 255);
  auto sh = y + freetype::load_lh() + 12;
  freetype::draw_line(s2, x + (w - sw) / 2, sh);

  if (map_desc != u"") {
    SDL_SetRenderDrawColor(window::renderer, 240, 224, 104, 255);
    sh += 25;
    SDL_RenderLine(window::renderer, x, sh, x + w, sh);

    auto s3 = map_desc;
    freetype::draw_str(s3, x + 10, sh + 5, w - 20);
  }
  freetype::load_aligned(false);
}