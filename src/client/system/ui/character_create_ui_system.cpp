#include "character_create_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "login_ui_system.h"
#include "src/client/game/game_character.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_choose_ui_system.h"
#include "src/client/system_instance/character_choose_system_instance.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cmath>
#include <string>
#include <vector>

void character_create_ui_system::render_button() {
  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"Login.img/NewChar/button:ok"),
      wz_resource::ui->find(u"Login.img/NewChar/button:cancel"),
  };
  auto pos = load_pos();
  auto &camera = camera_game_instance::camera;
  std::array buttons_rect = {
      SDL_FRect{66 - camera.x, -1154 - camera.y, 104, 52},
      SDL_FRect{160 - camera.x, -1156 - camera.y, 104, 52},
  };
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = wz_resource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = wz_resource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = wz_resource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

SDL_FPoint character_create_ui_system::load_pos() {
  SDL_FPoint pos;
  const auto w = 1366;
  const auto h = 768;
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - w) / 2;
  pos.y = (camera.h - h) / 2;
  return pos;
}

void character_create_ui_system::render_backgrnd() {
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/Common/classicFrame"));
  auto pos = load_pos();
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
}

void character_create_ui_system::render_stat() {
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/NewChar/stat/canvas:statback"));
  auto &camera = camera_game_instance::camera;
  SDL_FRect pos_rect{
      70 - camera.x,
      -1322 - camera.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);

  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:str_prev"),
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:str_next"),
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:dex_prev"),
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:dex_next"),
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:int_prev"),
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:int_next"),
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:luk_prev"),
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:luk_next"),
  };
  std::array buttons_rect = {
      SDL_FRect{pos_rect.x + 48, pos_rect.y + 4, 16, 17},
      SDL_FRect{pos_rect.x + 96, pos_rect.y + 4, 16, 17},
      SDL_FRect{pos_rect.x + 48, pos_rect.y + 27, 16, 17},
      SDL_FRect{pos_rect.x + 96, pos_rect.y + 27, 16, 17},
      SDL_FRect{pos_rect.x + 48, pos_rect.y + 50, 16, 17},
      SDL_FRect{pos_rect.x + 96, pos_rect.y + 50, 16, 17},
      SDL_FRect{pos_rect.x + 48, pos_rect.y + 73, 16, 17},
      SDL_FRect{pos_rect.x + 96, pos_rect.y + 73, 16, 17},
  };
  std::vector<bool> r = {
      str_point <= 4, str_point >= 12, dex_point <= 4, dex_point >= 12,
      int_point <= 4, int_point >= 12, luk_point <= 4, luk_point >= 12,
  };

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = window::mouse_pos;
    if (r[i]) {
      auto normal = wz_resource::load_texture(k->find(u"disabled/0"));
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    } else if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = wz_resource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = wz_resource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = wz_resource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void character_create_ui_system::render_character() {
  auto &camera = camera_game_instance::camera;
  g_character.pos.x = -84;
  g_character.pos.y = -1211;
  // animate
  character_logic_system::run_animate(g_character);
  character_render_system::render_character(g_character);
}

void character_create_ui_system::render_banner() {
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/LoginStart/StepBanner/NewChar"));
  auto pos = load_pos();
  SDL_FRect pos_rect{
      pos.x + 160,
      pos.y + 85,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);

  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"Login.img/LoginStart/BtClassicPrev"),
  };
  std::array buttons_rect = {
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},
  };
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = wz_resource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = wz_resource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = wz_resource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void character_create_ui_system::render_custom() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/NewChar/customize/animOpen/3"));
  auto pos = load_pos();
  SDL_FRect pos_rect{
      pos.x + 160,
      pos.y + 85,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);

  static auto gender_t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/NewChar/customize/canvas:gender"));
  pos_rect = {
      pos_rect.x + 160,
      pos_rect.y + 85,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(window::renderer, gender_t, nullptr, &pos_rect);

  static auto face_t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/NewChar/customize/canvas:face"));
  pos_rect = {
      pos_rect.x + 160,
      pos_rect.y + 85,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(window::renderer, face_t, nullptr, &pos_rect);

  static auto hairstyle_t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/NewChar/customize/canvas:hairstyle"));
  pos_rect = {
      pos_rect.x + 160,
      pos_rect.y + 85,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(window::renderer, hairstyle_t, nullptr, &pos_rect);

  static auto haircolor_t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/NewChar/customize/canvas:haircolor"));
  pos_rect = {
      pos_rect.x + 160,
      pos_rect.y + 85,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(window::renderer, haircolor_t, nullptr, &pos_rect);

  static auto skincolor_t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/NewChar/customize/canvas:skincolor"));
  pos_rect = {
      pos_rect.x + 160,
      pos_rect.y + 85,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(window::renderer, skincolor_t, nullptr, &pos_rect);

  static auto top_t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/NewChar/customize/canvas:top"));
  pos_rect = {
      pos_rect.x + 160,
      pos_rect.y + 85,
      static_cast<float>(top_t->w),
      static_cast<float>(top_t->h),
  };
  SDL_RenderTexture(window::renderer, top_t, nullptr, &pos_rect);

  const auto &character = g_character;
  auto face_id = character.face.id;
  auto head_id = character.head;
  auto body_id = character.body;
  auto hair_id = character.hair;

  auto cap_id = character.cap->id;
  auto coat_id = character.coat->id;
  auto pant_id = character.pant->id;
  auto weapon_id = character.weapon->id;

  auto face_name = equip_game_instance::load_equip_name(face_id);
  auto skin_name = equip_game_instance::load_equip_name(head_id);
  auto hair_name = equip_game_instance::load_equip_name(hair_id);

  auto cap_name = equip_game_instance::load_equip_name(cap_id);
  auto coat_name = equip_game_instance::load_equip_name(coat_id);
  auto pant_name = equip_game_instance::load_equip_name(pant_id);
  auto weapon_name = equip_game_instance::load_equip_name(weapon_id);
}

bool character_create_ui_system::render() {
  render_stat();
  render_button();
  render_character();
  render_backgrnd();
  render_banner();
  return true;
}

bool character_create_ui_system::back_animate() {
  if (login_ui_system::camera_animate(-80, -479)) {
    character_choose_system_instance::enter();
    return false;
  }
  return true;
}

void character_create_ui_system::event_button_back() {
  system::logic_systems.push_back(back_animate);
  system::render_systems = {
      login_system_instance::render_game,
      character_choose_ui_system::render,
      cursor_render_system::render,
  };
  system::event_systems = {};
}

std::vector<std::u16string> character_create_ui_system::load_haircolors() {
  std::vector<std::u16string> r;
  static auto haircolors = wz_resource::ui->find(u"Login.img/HairColorName");
  for (auto [k, v] : *haircolors->get_children()) {
    r.push_back(static_cast<wz::Property<std::u16string> *>(v[0])->get());
  }
  return r;
}

void character_create_ui_system::reset_character(bool g) {
  gender = g;

  auto &g_character = character_create_ui_system::g_character;
  character_game_instance::add_body(g_character, u"00002000");
  character_game_instance::add_head(g_character, u"00012000");
  character_game_instance::add_ear(g_character, u"humanEar");
  character_game_instance::add_face(g_character, load_default_face()[0]);
  character_game_instance::add_hair(g_character, load_default_hair()[0]);
  character_game_instance::add_coat(g_character, load_default_top()[0]);
  character_game_instance::add_pants(g_character, load_default_bottom()[0]);

  character_game_instance::add_weapon(g_character, load_default_weapon()[0]);
  character_logic_system::run_stand_action(g_character);
  g_character.flip = 1;
}

void character_create_ui_system::event_button_gender_prev() {
  if (gender) {
    gender = true;
    reset_character(true);
  }
}

void character_create_ui_system::event_button_gender_next() {
  if (!gender) {
    gender = false;
    reset_character(false);
  }
}

std::vector<std::u16string> character_create_ui_system::load_default_face() {
  std::vector<std::u16string> faces;
  if (!gender) {
    faces = {u"00020000", u"00020001", u"00020002"};
  } else {
    faces = {u"00021000", u"00021001", u"00021002"};
  }
  return faces;
}

void character_create_ui_system::event_button_face_prev() {
  auto faces = load_default_face();
  auto face_id = g_character.face.id;
  auto it = std::ranges::find(faces, face_id);
  if (it != faces.end()) {
    auto index = std::distance(faces.begin(), it);
    if (index > 0) {
      index -= 1;
      character_game_instance::add_face(g_character, faces[index]);
    }
  }
}

void character_create_ui_system::event_button_face_next() {
  auto faces = load_default_face();
  auto face_id = g_character.face.id;
  auto it = std::ranges::find(faces, face_id);
  if (it != faces.end()) {
    auto index = std::distance(faces.begin(), it);
    if (index < faces.size() - 1) {
      index += 1;
      character_game_instance::add_face(g_character, faces[index]);
    }
  }
}

std::vector<std::u16string> character_create_ui_system::load_default_hair() {
  std::vector<std::u16string> hairs;
  if (!gender) {
    hairs = {u"00030000", u"00030020", u"00030030"};
  } else {
    hairs = {u"00031000", u"00031040", u"00031050"};
  }
  return hairs;
}

void character_create_ui_system::event_button_hair_prev() {
  auto hairs = load_default_hair();
  auto hair_id = g_character.hair;
  auto it = std::ranges::find(hairs, hair_id);
  if (it != hairs.end()) {
    auto index = std::distance(hairs.begin(), it);
    if (index < hairs.size() - 1) {
      index += 1;
      character_game_instance::add_hair(g_character, hairs[index]);
    }
  }
}

void character_create_ui_system::event_button_hair_next() {
  auto hairs = load_default_hair();
  auto hair_id = g_character.hair;
  auto it = std::ranges::find(hairs, hair_id);
  if (it != hairs.end()) {
    auto index = std::distance(hairs.begin(), it);
    if (index > 0) {
      index -= 1;
      character_game_instance::add_hair(g_character, hairs[index]);
    }
  }
}

void character_create_ui_system::event_button_hair_color_prev() {
  auto hair_id = g_character.hair;
  auto back = hair_id.back();
  if (back > u'0') {
    back -= 1;
  }
  hair_id.back() = back;
  character_game_instance::add_hair(g_character, hair_id);
}

void character_create_ui_system::event_button_hair_color_next() {
  auto hair_id = g_character.hair;
  auto back = hair_id.back();
  if (back < u'7') {
    back += 1;
  }
  hair_id.back() = back;
  character_game_instance::add_hair(g_character, hair_id);
}

void character_create_ui_system::event_button_skin_color_prev() {
  auto head_id = g_character.head;
  auto body_id = g_character.body;
  auto back = head_id.back();
  if (back > u'0') {
    back -= 1;
  }
  head_id.back() = back;
  body_id.back() = back;
  character_game_instance::add_head(g_character, head_id);
  character_game_instance::add_body(g_character, body_id);
}

void character_create_ui_system::event_button_skin_color_next() {
  auto &head_id = g_character.head;
  auto body_id = g_character.body;
  auto back = head_id.back();
  if (back < u'3') {
    back += 1;
  }
  head_id.back() = back;
  body_id.back() = back;
  character_game_instance::add_head(g_character, head_id);
  character_game_instance::add_body(g_character, body_id);
}

std::vector<std::u16string> character_create_ui_system::load_default_top() {
  std::vector<std::u16string> tops;
  if (!gender) {
    tops = {u"01040001", u"01040002", u"01040003"};
  } else {
    tops = {u"01041001", u"01041002", u"01041003", u"01041004"};
  }
  return tops;
}

void character_create_ui_system::event_button_top_prev() {
  auto tops = load_default_top();
  auto top_id = g_character.coat->id;
  auto it = std::ranges::find(tops, top_id);
  if (it != tops.end()) {
    auto index = std::distance(tops.begin(), it);
    if (index < tops.size() - 1) {
      index += 1;
      character_game_instance::add_coat(g_character, tops[index]);
    }
  }
}

void character_create_ui_system::event_button_top_next() {
  auto tops = load_default_top();
  auto top_id = g_character.coat->id;
  auto it = std::ranges::find(tops, top_id);
  if (it != tops.end()) {
    auto index = std::distance(tops.begin(), it);
    if (index > 0) {
      index -= 1;
      character_game_instance::add_coat(g_character, tops[index]);
    }
  }
}

std::vector<std::u16string> character_create_ui_system::load_default_bottom() {
  std::vector<std::u16string> bottom;
  if (!gender) {
    bottom = {u"01060001", u"01060002"};
  } else {
    bottom = {u"01061001", u"01061002"};
  }
  return bottom;
}

void character_create_ui_system::event_button_bottom_prev() {
  auto bottoms = load_default_bottom();
  auto pant_id = g_character.pant->id;
  auto it = std::ranges::find(bottoms, pant_id);
  if (it != bottoms.end()) {
    auto index = std::distance(bottoms.begin(), it);
    if (index < bottoms.size() - 1) {
      index += 1;
      character_game_instance::add_pants(g_character, bottoms[index]);
    }
  }
}

void character_create_ui_system::event_button_bottom_next() {
  auto bottoms = load_default_bottom();
  auto pant_id = g_character.pant->id;
  auto it = std::ranges::find(bottoms, pant_id);
  if (it != bottoms.end()) {
    auto index = std::distance(bottoms.begin(), it);
    if (index > 0) {
      index -= 1;
      character_game_instance::add_pants(g_character, bottoms[index]);
    }
  }
}

std::vector<std::u16string> character_create_ui_system::load_default_shoes() {
  std::vector<std::u16string> shoes;
  shoes = {u"00107200", u"00107201", u"00107202", u"00107203"};
  return shoes;
}

void character_create_ui_system::event_button_shoes_prev() {
  auto shoes = load_default_shoes();
  auto shoes_id = g_character.shoes->id;
  auto it = std::ranges::find(shoes, shoes_id);
  if (it != shoes.end()) {
    auto index = std::distance(shoes.begin(), it);
    if (index < shoes.size() - 1) {
      index += 1;
      character_game_instance::add_shoes(g_character, shoes[index]);
    }
  }
}

void character_create_ui_system::event_button_shoes_next() {
  auto shoes = load_default_shoes();
  auto shoes_id = g_character.shoes->id;
  auto it = std::ranges::find(shoes, shoes_id);
  if (it != shoes.end()) {
    auto index = std::distance(shoes.begin(), it);
    if (index > 0) {
      index -= 1;
      character_game_instance::add_shoes(g_character, shoes[index]);
    }
  }
}

std::vector<std::u16string> character_create_ui_system::load_default_weapon() {
  std::vector<std::u16string> weapon;
  weapon = {u"01302000", u"01312000", u"01322000"};
  return weapon;
}

void character_create_ui_system::event_button_weapon_prev() {
  auto weapons = load_default_weapon();
  auto weapon_id = g_character.weapon->id;
  auto it = std::ranges::find(weapons, weapon_id);
  if (it != weapons.end()) {
    auto index = std::distance(weapons.begin(), it);
    if (index < weapons.size() - 1) {
      index += 1;
      character_game_instance::add_weapon(g_character, weapons[index]);
    }
  }
}

void character_create_ui_system::event_button_weapon_next() {
  auto weapons = load_default_weapon();
  auto weapon_id = g_character.weapon->id;
  auto it = std::ranges::find(weapons, weapon_id);
  if (it != weapons.end()) {
    auto index = std::distance(weapons.begin(), it);
    if (index > 0) {
      index -= 1;
      character_game_instance::add_weapon(g_character, weapons[index]);
    }
  }
}

bool character_create_ui_system::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto &camera = camera_game_instance::camera;
  auto pos = load_pos();
  SDL_FPoint stat_point{
      70 - camera.x,
      -1322 - camera.y,
  };
  r = {
      SDL_FRect{stat_point.x + 48, stat_point.y + 4, 16, 17},
      SDL_FRect{stat_point.x + 96, stat_point.y + 4, 16, 17},
      SDL_FRect{stat_point.x + 48, stat_point.y + 27, 16, 17},
      SDL_FRect{stat_point.x + 96, stat_point.y + 27, 16, 17},
      SDL_FRect{stat_point.x + 48, stat_point.y + 50, 16, 17},
      SDL_FRect{stat_point.x + 96, stat_point.y + 50, 16, 17},
      SDL_FRect{stat_point.x + 48, stat_point.y + 73, 16, 17},
      SDL_FRect{stat_point.x + 96, stat_point.y + 73, 16, 17},
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},
  };
  fns = {
      {}, {}, {}, {}, {}, {}, {}, {}, event_button_back,
  };
  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      return false;
    }
  }

  return false;
}

bool character_create_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
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
      r = false;
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      event_button(event);
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    break;
  }
  default: {
    break;
  }
  }

  return r;
}