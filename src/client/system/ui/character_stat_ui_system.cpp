#include "character_stat_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
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

  static auto backgrnd_detail = wz_resource::load_texture(
      wz_resource::ui->find(u"CharacterStat.img/Detail/backgrnd"));
  pos_rect = {pos.x + detail_rect.x, pos.y + detail_rect.y,
              static_cast<float>(backgrnd_detail->w),
              static_cast<float>(backgrnd_detail->h)};
  SDL_RenderTexture(window::renderer, backgrnd_detail, nullptr, &pos_rect);
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
  auto self_job = job_skill_game_instance::self_job;
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
  auto level = character_stat_game_instance::level;
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
  auto fame = character_stat_game_instance::fame;
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

  if (detail) {
    auto dx = detail_rect.x + pos.x;
    auto dy = detail_rect.y + pos.y;
    // atk
    p = {dx + 78, dy + 10};
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

  freetype::load_aligned(false);
}

bool character_stat_ui_system::render() {
  render_backgrnd();
  render_text();
  return true;
}

void character_stat_ui_system::open() {
  detail = true;

  auto wh = load_wh();
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
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

bool character_stat_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
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

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
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
    pos_rect.w = pos.y + detail_rect.w;
    pos_rect.h = pos.y + detail_rect.h;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      return true;
    }
  }
  return false;
}