#include "character_stat_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "notice_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <cstdint>
#include <string>

SDL_FPoint character_stat_ui_system::load_wh() { return {185, 281}; }

static const SDL_FRect detail_rect = {185, 58, 187, 223};

void character_stat_ui_system::render_backgrnd() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"CharacterStat.img/Main/backgrnd"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);
}

void character_stat_ui_system::render_text() {
  SDL_FPoint p{pos.x + 60, pos.y + 32};
  // name
  const auto &self = character_game_instance::self;
  auto &self_name = self.nametags[0].text;
  freetype::load_size(12);
  freetype::load_aligned(true);
  freetype::load_color(0, 0, 0, 255);
  freetype::draw_line(self_name, p.x, p.y);
  // job
  auto self_job = self.job;
  p = {pos.x + 61, pos.y + 55};
  static auto job_node = wz_resource::ui->find(u"CharacterStat.img/Main/Job");
  auto job_texture = wz_resource::load_texture(
      job_node->find(u"main/" + std::u16string{self_job[0]}));
  SDL_FRect pos_rect{static_cast<float>((int)p.x), static_cast<float>((int)p.y),
                     static_cast<float>(job_texture->w),
                     static_cast<float>(job_texture->h)};
  SDL_RenderTexture(window::renderer, job_texture, nullptr, &pos_rect);

  // sub job
  auto job_sub_node = job_node->find(u"sub/" + std::u16string{self_job[0]});
  if (self_job == u"000") {
    self_job = u"0";
  }
  auto sub_job_texture =
      wz_resource::load_texture(job_sub_node->get_child(self_job));
  pos_rect = {
      static_cast<float>((int)pos.x + 61),
      static_cast<float>((int)pos.y + 67),
      static_cast<float>(sub_job_texture->w),
      static_cast<float>(sub_job_texture->h),
  };
  SDL_RenderTexture(window::renderer, sub_job_texture, nullptr, &pos_rect);

  // level
  p = {pos.x + 60, pos.y + 79};
  auto level = self.level;
  auto level1 = std::to_string(level);
  auto level2 = std::u16string{level1.begin(), level1.end()};
  freetype::draw_line(level2, p.x, p.y);

  // hp
  p = {pos.x + 60, pos.y + 97};
  auto hp = character_stat_game_instance::hp_point;
  auto hp1 = std::to_string(hp);
  auto hp2 = std::u16string{hp1.begin(), hp1.end()};
  freetype::draw_line(hp2, p.x, p.y);

  // mp
  p = {pos.x + 60, pos.y + 115};
  auto mp = character_stat_game_instance::mp_point;
  auto mp1 = std::to_string(mp);
  auto mp2 = std::u16string{mp1.begin(), mp1.end()};
  freetype::draw_line(mp2, p.x, p.y);

  // exp
  p = {pos.x + 60, pos.y + 133};
  auto exp = character_stat_game_instance::exp_point;
  auto exp1 = std::to_string(exp);
  auto exp2 = std::u16string{exp1.begin(), exp1.end()};
  freetype::draw_line(exp2, p.x, p.y);

  // fame
  p = {pos.x + 60, pos.y + 151};
  auto fame = self.fame;
  auto fame1 = std::to_string(fame);
  auto fame2 = std::u16string{fame1.begin(), fame1.end()};
  freetype::draw_line(fame2, p.x, p.y);

  // str
  p = {pos.x + 60, pos.y + 175};
  auto str = character_stat_game_instance::str_point;
  auto str1 = std::to_string(str);
  auto str2 = std::u16string{str1.begin(), str1.end()};
  freetype::draw_line(str2, p.x, p.y);

  // dex
  p = {pos.x + 60, pos.y + 193};
  auto dex = character_stat_game_instance::dex_point;
  auto dex1 = std::to_string(dex);
  auto dex2 = std::u16string{dex1.begin(), dex1.end()};
  freetype::draw_line(dex2, p.x, p.y);

  // int
  p = {pos.x + 60, pos.y + 211};
  auto in = character_stat_game_instance::int_point;
  auto in1 = std::to_string(in);
  auto in2 = std::u16string{in1.begin(), in1.end()};
  freetype::draw_line(in2, p.x, p.y);

  // luk
  p = {pos.x + 60, pos.y + 229};
  auto luk = character_stat_game_instance::luk_point;
  auto luk1 = std::to_string(luk);
  auto luk2 = std::u16string{luk1.begin(), luk1.end()};
  freetype::draw_line(luk2, p.x, p.y);
}

void character_stat_ui_system::render_remain_ap() {
  auto s = std::to_string(character_stat_game_instance::remain_ap);
  auto s2 = std::u16string{s.begin(), s.end()};
  freetype::load_size(12);
  freetype::load_aligned(true);
  freetype::load_color(0, 0, 0, 255);
  freetype::draw_line(s2, pos.x + 90, pos.y + 258);
}

void character_stat_ui_system::render_button() {
  // 构建按钮列表
  std::vector<wz::Node *> nodes = {
      wz_resource::ui->find(u"Basic.img/BtClose"),
      wz_resource::ui->find(u"CharacterStat.img/Main/button:detail"),
      wz_resource::ui->find(u"Basic.img/BtUP"), // hp
      wz_resource::ui->find(u"CharacterStat.img/Main/button:hpupall"),
      wz_resource::ui->find(u"Basic.img/BtUP"), // mp
      wz_resource::ui->find(u"CharacterStat.img/Main/button:hpupall"),
      wz_resource::ui->find(u"Basic.img/BtUP"), // str
      wz_resource::ui->find(u"CharacterStat.img/Main/button:hpupall"),
      wz_resource::ui->find(u"Basic.img/BtUP"), // dex
      wz_resource::ui->find(u"CharacterStat.img/Main/button:hpupall"),
      wz_resource::ui->find(u"Basic.img/BtUP"), // int
      wz_resource::ui->find(u"CharacterStat.img/Main/button:hpupall"),
      wz_resource::ui->find(u"Basic.img/BtUP"), // luk
      wz_resource::ui->find(u"CharacterStat.img/Main/button:hpupall"),
  };
  std::vector<SDL_FRect> rects = {
      {166, 6, 12, 12},   //
      {122, 256, 47, 18}, //
      {148, 99, 12, 12},  //
      {161, 99, 12, 12},  //
      {148, 117, 12, 12}, //
      {161, 117, 12, 12}, //
      {148, 177, 12, 12}, // str
      {161, 177, 12, 12}, // strall
      {148, 195, 12, 12}, // dex
      {161, 195, 12, 12}, // dexall
      {148, 213, 12, 12}, // dex
      {161, 213, 12, 12}, // dexall
      {148, 231, 12, 12}, // dex
      {161, 231, 12, 12}, // dexall
  };
  std::vector<bool> disable = {};
  if (character_stat_game_instance::remain_ap) {
    disable = {
        false, false, false, false, false, false, false,
        false, false, false, false, false, false, false,
    };
  } else {
    disable = {
        false, true, true, true, true, true, true,
        true,  true, true, true, true, true, true,
    };
  }

  if (detail) {
    nodes.push_back(wz_resource::ui->find(u"Basic.img/BtHide"));
    rects.push_back({164 + detail_rect.x, 203 + detail_rect.y, 13, 12});
    disable.push_back(false);
  }
  // 渲染所有按钮
  bool mouse_down = window::mouse_state & SDL_BUTTON_LMASK;
  bool cursor_on_ui = cursor_game_instance::cursor_ui == render;
  bool modal_blocked = cursor_game_instance::modal_overlay;

  for (size_t i = 0; i < nodes.size(); ++i) {
    SDL_FRect rect = {
        rects[i].x + (int)pos.x,
        rects[i].y + (int)pos.y,
        rects[i].w,
        rects[i].h,
    };

    std::u16string state = u"normal";
    bool mouse_in = SDL_PointInRectFloat(&window::mouse_pos, &rect);

    if (disable[i]) {
      state = u"disabled";
    } else if (cursor_on_ui && !modal_blocked && mouse_in) {
      state = mouse_down ? u"pressed" : u"mouseOver";
    }
    auto texture = wz_resource::load_texture(nodes[i]->find((state + u"/0")));
    SDL_RenderTexture(window::renderer, texture, nullptr, &rect);
  }
}

void character_stat_ui_system::render_detail() {
  static auto backgrnd_detail = wz_resource::load_texture(
      wz_resource::ui->find(u"CharacterStat.img/Detail/backgrnd"));
  SDL_FRect pos_rect = {pos.x + detail_rect.x, pos.y + detail_rect.y,
                        static_cast<float>(backgrnd_detail->w),
                        static_cast<float>(backgrnd_detail->h)};
  SDL_RenderTexture(window::renderer, backgrnd_detail, nullptr, &pos_rect);

  auto dx = detail_rect.x + pos.x;
  auto dy = detail_rect.y + pos.y;
  // atk
  SDL_FPoint p = {dx + 78, dy + 10};
  auto min_atk = character_stat_game_instance::min_atk;
  auto max_atk = character_stat_game_instance::max_atk;
  auto s = std::to_string(min_atk) + " - " + std::to_string(max_atk);
  auto s2 = std::u16string{s.begin(), s.end()};
  freetype::draw_line(s2, p.x, p.y);
  // pdd
  p = {dx + 78, dy + 28};
  auto pdd = character_stat_game_instance::weapon_def;
  s = std::to_string(pdd);
  s2 = std::u16string{s.begin(), s.end()};
  freetype::draw_line(s2, p.x, p.y);
  // mad
  p = {dx + 78, dy + 46};
  auto mad = character_stat_game_instance::magic;
  s = std::to_string(mad);
  s2 = std::u16string{s.begin(), s.end()};
  freetype::draw_line(s2, p.x, p.y);
  // mdd
  p = {dx + 78, dy + 62};
  auto mdd = character_stat_game_instance::magic_def;
  s = std::to_string(mdd);
  s2 = std::u16string{s.begin(), s.end()};
  freetype::draw_line(s2, p.x, p.y);
  // acc
  p = {dx + 78, dy + 82};
  auto acc = character_stat_game_instance::accuracy;
  s = std::to_string(acc);
  s2 = std::u16string{s.begin(), s.end()};
  freetype::draw_line(s2, p.x, p.y);
  // eva
  p = {dx + 78, dy + 100};
  auto avoid = character_stat_game_instance::avoid;
  s = std::to_string(avoid);
  s2 = std::u16string{s.begin(), s.end()};
  freetype::draw_line(s2, p.x, p.y);
  // crtr
  p = {dx + 78, dy + 118};
  auto crit_rate = character_stat_game_instance::crit_rate;
  s = std::to_string(crit_rate);
  s2 = std::u16string{s.begin(), s.end()};
  freetype::draw_line(s2, p.x, p.y);
  // crtd
  p = {dx + 78, dy + 136};
  auto crit_damage = character_stat_game_instance::crit_damage;
  s = std::to_string(crit_damage);
  s2 = std::u16string{s.begin(), s.end()};
  freetype::draw_line(s2, p.x, p.y);
  // hspd
  p = {dx + 78, dy + 154};
  auto hspd = character_logic_system::self_hspeed_max;
  s = std::to_string((int)hspd);
  s2 = std::u16string{s.begin(), s.end()};
  freetype::draw_line(s2, p.x, p.y);
  // vspd
  p = {dx + 78, dy + 172};
  auto vspd = -character_logic_system::self_vspeed_min * 0.1;
  s = std::to_string((int)vspd);
  s2 = std::u16string{s.begin(), s.end()};
  freetype::draw_line(s2, p.x, p.y);
}

bool character_stat_ui_system::render() {
  render_backgrnd();
  render_text();
  render_remain_ap();
  if (detail) {
    render_detail();
  }
  render_button();
  return true;
}

void character_stat_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    detail = true;

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void character_stat_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);

  event_drag_end();
}

void character_stat_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

void character_stat_ui_system::event_button_detail_show() { detail = true; }

void character_stat_ui_system::event_button_detail_hide() { detail = false; }

void character_stat_ui_system::event_button_hp_inc() {
  character_stat_game_instance::hp_ap += 1;
  character_stat_game_instance::remain_ap -= 1;
}

void character_stat_ui_system::event_button_hp_inc_max() {
  notice_ui_system::data = &character_stat_game_instance::hp_ap;
  notice_ui_system::type = notice_ui_system::notice_enum::ap_inc;
  notice_ui_system::open();
}

void character_stat_ui_system::event_button_mp_inc() {
  character_stat_game_instance::mp_ap += 1;
  character_stat_game_instance::remain_ap -= 1;
}

void character_stat_ui_system::event_button_mp_inc_max() {
  notice_ui_system::data = &character_stat_game_instance::mp_ap;
  notice_ui_system::type = notice_ui_system::notice_enum::ap_inc;
  notice_ui_system::open();
}

void character_stat_ui_system::event_button_str_inc() {
  character_stat_game_instance::str_ap += 1;
  character_stat_game_instance::str_point += 1;
  character_stat_game_instance::remain_ap -= 1;
}

void character_stat_ui_system::event_button_str_inc_max() {
  notice_ui_system::data = &character_stat_game_instance::str_ap;
  notice_ui_system::type = notice_ui_system::notice_enum::ap_inc;
  notice_ui_system::open();
}

void character_stat_ui_system::event_button_dex_inc() {
  character_stat_game_instance::dex_ap += 1;
  character_stat_game_instance::dex_point += 1;
  character_stat_game_instance::remain_ap -= 1;
}

void character_stat_ui_system::event_button_dex_inc_max() {
  notice_ui_system::data = &character_stat_game_instance::dex_ap;
  notice_ui_system::type = notice_ui_system::notice_enum::ap_inc;
  notice_ui_system::open();
}

void character_stat_ui_system::event_button_int_inc() {
  character_stat_game_instance::int_ap += 1;
  character_stat_game_instance::int_point += 1;
  character_stat_game_instance::remain_ap -= 1;
}

void character_stat_ui_system::event_button_int_inc_max() {
  notice_ui_system::data = &character_stat_game_instance::int_ap;
  notice_ui_system::type = notice_ui_system::notice_enum::ap_inc;
  notice_ui_system::open();
}

void character_stat_ui_system::event_button_luk_inc() {
  character_stat_game_instance::luk_ap += 1;
  character_stat_game_instance::luk_point += 1;
  character_stat_game_instance::remain_ap -= 1;
}

void character_stat_ui_system::event_button_luk_inc_max() {
  notice_ui_system::data = &character_stat_game_instance::luk_ap;
  notice_ui_system::type = notice_ui_system::notice_enum::ap_inc;
  notice_ui_system::open();
}

bool character_stat_ui_system::event_button(SDL_Event *event) {
  std::vector<void (*)()> fns;
  auto wh = load_wh();
  std::vector<SDL_FRect> buttons_rect = {
      {166, 6, 12, 12},   // close
      {122, 256, 47, 18}, // detail
      {148, 99, 12, 12},  // hp
      {161, 99, 12, 12},  // hpm
      {148, 117, 12, 12}, // mp
      {161, 117, 12, 12}, // mpm
      {148, 177, 12, 12}, // str
      {161, 177, 12, 12}, // strall
      {148, 195, 12, 12}, // dex
      {161, 195, 12, 12}, // dexall
      {148, 213, 12, 12}, // dex
      {161, 213, 12, 12}, // dexall
      {148, 231, 12, 12}, // dex
      {161, 231, 12, 12}, // dexall
  };
  fns = {
      close,
      event_button_detail_show,
      event_button_hp_inc,
      event_button_hp_inc_max,
      event_button_mp_inc,
      event_button_mp_inc_max,
      event_button_str_inc,
      event_button_str_inc_max,
      event_button_dex_inc,
      event_button_dex_inc_max,
      event_button_int_inc,
      event_button_int_inc_max,
      event_button_luk_inc,
      event_button_luk_inc_max,
  };

  if (detail) {
    buttons_rect.push_back({164 + detail_rect.x, 203 + detail_rect.y, 13, 12});
    fns.push_back(event_button_detail_hide);
  }

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

bool character_stat_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      close();
      return false;
      break;
    }
    default: {
      break;
    }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        event_top();
        event_drag_start(event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        r = !event_button(event);
      }
      event_drag_end();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_drag_move(event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

void character_stat_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

void character_stat_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void character_stat_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void character_stat_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto wh = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - wh.x);
    pos.y = std::clamp(pos.y, (float)0, camera.h - wh.y);
  }
  return;
}

bool character_stat_ui_system::cursor_in() {
  auto wh = load_wh();
  SDL_FRect pos_rect{pos.x, pos.y, wh.x, wh.y};
  if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
    return true;
  }
  if (detail) {
    pos_rect.x = pos.x + detail_rect.x;
    pos_rect.y = pos.y + detail_rect.y;
    pos_rect.w = detail_rect.w;
    pos_rect.h = detail_rect.h;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      return true;
    }
  }
  return false;
}