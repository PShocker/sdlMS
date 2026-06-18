#include "character_create_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
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
#include "src/client/system_instance/chatacter_create_system_instance.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cmath>
#include <cstdint>
#include <optional>
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
      str_point <= 4, str_point > 12 || remain_point == 0,
      dex_point <= 4, dex_point > 12 || remain_point == 0,
      int_point <= 4, int_point > 12 || remain_point == 0,
      luk_point <= 4, luk_point > 12 || remain_point == 0,
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

  freetype::load_size(13);
  freetype::load_aligned(true);
  freetype::load_color(0, 0, 0, 255);
  auto str1 = std::to_string(str_point);
  auto str2 = std::u16string{str1.begin(), str1.end()};
  auto dx = freetype::load_w(str2);
  freetype::draw_line(str2, buttons_rect[0].x + 32 - dx / 2,
                      buttons_rect[0].y - 2);

  auto dex1 = std::to_string(dex_point);
  auto dex2 = std::u16string{dex1.begin(), dex1.end()};
  dx = freetype::load_w(dex2);
  freetype::draw_line(dex2, buttons_rect[2].x + 32 - dx / 2,
                      buttons_rect[2].y - 2);

  auto int1 = std::to_string(int_point);
  auto int2 = std::u16string{int1.begin(), int1.end()};
  dx = freetype::load_w(int2);
  freetype::draw_line(int2, buttons_rect[4].x + 32 - dx / 2,
                      buttons_rect[4].y - 2);

  auto luk1 = std::to_string(luk_point);
  auto luk2 = std::u16string{luk1.begin(), luk1.end()};
  dx = freetype::load_w(luk2);
  freetype::draw_line(luk2, buttons_rect[6].x + 32 - dx / 2,
                      buttons_rect[6].y - 2);
  freetype::load_bold(true);
  freetype::load_size(21);
  auto r1 = std::to_string(remain_point);
  auto r2 = std::u16string{r1.begin(), r1.end()};
  freetype::draw_line(r2, buttons_rect[6].x + 98, buttons_rect[6].y - 28);
  freetype::load_bold(false);

  freetype::load_aligned(false);
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

void character_create_ui_system::render_custom_item(float cx, float cy,
                                                    choose_type type) {
  std::u16string item_name;
  SDL_FPoint pos;
  SDL_Texture *t;
  bool choose = false;
  switch (type) {
  case choose_type::gender: {
    if (gender) {
      item_name = u"Female";
    } else {
      item_name = u"Male";
    }
    if (choose_index == 0) {
      choose = true;
      t = wz_resource::load_texture(wz_resource::ui->find(
          "Login.img/NewChar/customize/canvas:gender_selected"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find("Login.img/NewChar/customize/canvas:gender"));
    }
    pos = {35, 30};
    break;
  }
  case choose_type::face: {
    item_name = equip_game_instance::load_equip_name(g_character.face.id);
    if (choose_index == 1) {
      choose = true;
      t = wz_resource::load_texture(wz_resource::ui->find(
          "Login.img/NewChar/customize/canvas:face_selected"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find("Login.img/NewChar/customize/canvas:face"));
    }
    pos = {35, 48};
    break;
  }
  case choose_type::hairstyle: {
    item_name = equip_game_instance::load_equip_name(g_character.hair);
    if (choose_index == 2) {
      choose = true;
      t = wz_resource::load_texture(wz_resource::ui->find(
          "Login.img/NewChar/customize/canvas:hairstyle_selected"));
    } else {
      t = wz_resource::load_texture(wz_resource::ui->find(
          "Login.img/NewChar/customize/canvas:hairstyle"));
    }
    pos = {35, 66};
    break;
  }
  case choose_type::haircolor: {
    static auto haircolor_node =
        wz_resource::ui->find(u"Login.img/HairColorName");
    auto back = g_character.hair.back();
    auto node = haircolor_node->get_child(std::u16string{back});
    item_name = static_cast<wz::Property<std::u16string> *>(node)->get();
    if (choose_index == 3) {
      choose = true;
      t = wz_resource::load_texture(wz_resource::ui->find(
          "Login.img/NewChar/customize/canvas:haircolor_selected"));
    } else {
      t = wz_resource::load_texture(wz_resource::ui->find(
          "Login.img/NewChar/customize/canvas:haircolor"));
    }
    pos = {35, 84};
    break;
  }
  case choose_type::skin: {
    item_name = equip_game_instance::load_equip_name(g_character.head);
    if (choose_index == 4) {
      choose = true;
      t = wz_resource::load_texture(wz_resource::ui->find(
          "Login.img/NewChar/customize/canvas:skincolor_selected"));
    } else {
      t = wz_resource::load_texture(wz_resource::ui->find(
          "Login.img/NewChar/customize/canvas:skincolor"));
    }
    pos = {35, 102};
    break;
  }
  case choose_type::top: {
    item_name = equip_game_instance::load_equip_name(g_character.coat->id);
    if (choose_index == 5) {
      choose = true;
      t = wz_resource::load_texture(wz_resource::ui->find(
          "Login.img/NewChar/customize/canvas:top_selected"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find("Login.img/NewChar/customize/canvas:top"));
    }
    pos = {35, 120};
    break;
  }
  case choose_type::bottom: {
    item_name = equip_game_instance::load_equip_name(g_character.pant->id);
    if (choose_index == 6) {
      choose = true;
      t = wz_resource::load_texture(wz_resource::ui->find(
          "Login.img/NewChar/customize/canvas:bottom_selected"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find("Login.img/NewChar/customize/canvas:bottom"));
    }
    pos = {35, 138};
    break;
  }
  case choose_type::shoes: {
    item_name = equip_game_instance::load_equip_name(g_character.shoes->id);
    if (choose_index == 7) {
      choose = true;
      t = wz_resource::load_texture(wz_resource::ui->find(
          "Login.img/NewChar/customize/canvas:shoes_selected"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find("Login.img/NewChar/customize/canvas:shoes"));
    }
    pos = {35, 156};
    break;
  }
  case choose_type::weapon: {
    item_name = equip_game_instance::load_equip_name(g_character.weapon->id);
    if (choose_index == 8) {
      choose = true;
      t = wz_resource::load_texture(wz_resource::ui->find(
          "Login.img/NewChar/customize/canvas:weapon_selected"));
    } else {
      t = wz_resource::load_texture(
          wz_resource::ui->find("Login.img/NewChar/customize/canvas:weapon"));
    }
    pos = {35, 174};
    break;
  }
  }
  SDL_FRect pos_rect{
      cx + pos.x,
      cy + pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);

  freetype::load_size(13);
  freetype::load_aligned(true);
  freetype::load_color(0, 0, 0, 255);
  auto dx = freetype::load_w(item_name);
  freetype::draw_line(item_name, pos_rect.x + 175 - dx / 2, pos_rect.y);
  freetype::load_aligned(false);

  static auto prev_button =
      wz_resource::ui->find("Login.img/NewChar/customize/button:gender_prev");

  static auto next_button =
      wz_resource::ui->find("Login.img/NewChar/customize/button:gender_next");
  if (choose) {
    std::vector<wz::Node *> buttons_nodes = {prev_button, next_button};
    std::vector<SDL_FRect> buttons_rect = {
        SDL_FRect{cx + pos.x + 95, cy + pos.y, 16, 17},  //
        SDL_FRect{cx + pos.x + 235, cy + pos.y, 16, 17}, //
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
}

void character_create_ui_system::render_custom() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/NewChar/customize/animOpen/3"));
  auto pos = load_pos();
  auto &camera = camera_game_instance::camera;
  auto cx = -440 - camera.x;
  auto cy = -1370 - camera.y;
  SDL_FRect pos_rect{
      cx,
      cy,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);

  render_custom_item(cx, cy, choose_type::gender);
  render_custom_item(cx, cy, choose_type::face);
  render_custom_item(cx, cy, choose_type::hairstyle);
  render_custom_item(cx, cy, choose_type::haircolor);
  render_custom_item(cx, cy, choose_type::skin);
  render_custom_item(cx, cy, choose_type::top);
  render_custom_item(cx, cy, choose_type::bottom);
  render_custom_item(cx, cy, choose_type::shoes);
  render_custom_item(cx, cy, choose_type::weapon);
}

bool character_create_ui_system::render() {
  render_stat();
  render_button();
  render_character();
  render_custom();
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
  chatacter_create_system_instance::enter();
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
  character_game_instance::add_shoes(g_character, load_default_shoes()[0]);

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
    // 计算前一个迭代器（循环）
    auto prev_it = (it == faces.begin()) ? (faces.end() - 1) : (it - 1);
    character_game_instance::add_face(g_character, *prev_it);
  }
}

void character_create_ui_system::event_button_face_next() {
  auto faces = load_default_face();
  auto face_id = g_character.face.id;
  auto it = std::ranges::find(faces, face_id);
  if (it != faces.end()) {
    auto next_it = (it + 1 == faces.end()) ? faces.begin() : (it + 1);
    character_game_instance::add_face(g_character, *next_it);
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
    auto prev_it = (it == hairs.begin()) ? (hairs.end() - 1) : (it - 1);
    character_game_instance::add_hair(g_character, *prev_it);
  }
}

void character_create_ui_system::event_button_hair_next() {
  auto hairs = load_default_hair();
  auto hair_id = g_character.hair;
  auto it = std::ranges::find(hairs, hair_id);
  if (it != hairs.end()) {
    auto next_it = (it + 1 == hairs.end()) ? hairs.begin() : (it + 1);
    character_game_instance::add_hair(g_character, *next_it);
  }
}

void character_create_ui_system::event_button_hair_color_prev() {
  auto hair_id = g_character.hair;
  auto back = hair_id.back();
  if (back > u'0') {
    back -= 1;
  } else {
    back = u'7';
  }
  hair_id.back() = back;
  character_game_instance::add_hair(g_character, hair_id);
}

void character_create_ui_system::event_button_hair_color_next() {
  auto hair_id = g_character.hair;
  auto back = hair_id.back();
  if (back < u'7') {
    back += 1;
  } else {
    back = u'0';
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
  } else {
    back = u'3';
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
  } else {
    back = u'0';
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
    auto prev_it = (it == tops.begin()) ? (tops.end() - 1) : (it - 1);
    character_game_instance::add_coat(g_character, *prev_it);
  }
}

void character_create_ui_system::event_button_top_next() {
  auto tops = load_default_top();
  auto top_id = g_character.coat->id;
  auto it = std::ranges::find(tops, top_id);
  if (it != tops.end()) {
    auto next_it = (it + 1 == tops.end()) ? tops.begin() : (it + 1);
    character_game_instance::add_coat(g_character, *next_it);
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
    auto prev_it = (it == bottoms.begin()) ? (bottoms.end() - 1) : (it - 1);
    character_game_instance::add_pants(g_character, *prev_it);
  }
}

void character_create_ui_system::event_button_bottom_next() {
  auto bottoms = load_default_bottom();
  auto pant_id = g_character.pant->id;
  auto it = std::ranges::find(bottoms, pant_id);
  if (it != bottoms.end()) {
    auto next_it = (it + 1 == bottoms.end()) ? bottoms.begin() : (it + 1);
    character_game_instance::add_pants(g_character, *next_it);
  }
}

std::vector<std::u16string> character_create_ui_system::load_default_shoes() {
  std::vector<std::u16string> shoes;
  shoes = {u"01072000", u"01072001", u"01072002", u"01072003"};
  return shoes;
}

void character_create_ui_system::event_button_shoes_prev() {
  auto shoes = load_default_shoes();
  auto shoes_id = g_character.shoes->id;
  auto it = std::ranges::find(shoes, shoes_id);
  if (it != shoes.end()) {
    auto prev_it = (it == shoes.begin()) ? (shoes.end() - 1) : (it - 1);
    character_game_instance::add_shoes(g_character, *prev_it);
  }
}

void character_create_ui_system::event_button_shoes_next() {
  auto shoes = load_default_shoes();
  auto shoes_id = g_character.shoes->id;
  auto it = std::ranges::find(shoes, shoes_id);
  if (it != shoes.end()) {
    auto next_it = (it + 1 == shoes.end()) ? shoes.begin() : (it + 1);
    character_game_instance::add_shoes(g_character, *next_it);
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
    auto prev_it = (it == weapons.begin()) ? (weapons.end() - 1) : (it - 1);
    character_game_instance::add_weapon(g_character, *prev_it);
  }
}

void character_create_ui_system::event_button_weapon_next() {
  auto weapons = load_default_weapon();
  auto weapon_id = g_character.weapon->id;
  auto it = std::ranges::find(weapons, weapon_id);
  if (it != weapons.end()) {
    auto next_it = (it + 1 == weapons.end()) ? weapons.begin() : (it + 1);
    character_game_instance::add_weapon(g_character, *next_it);
  }
}

void character_create_ui_system::event_button_str_inc() {
  if (remain_point > 0) {
    str_point++;
    remain_point--;
  }
}

void character_create_ui_system::event_button_str_dec() {
  if (str_point > 4) {
    str_point--;
    remain_point++;
  }
}

void character_create_ui_system::event_button_dex_inc() {
  if (remain_point > 0) {
    dex_point++;
    remain_point--;
  }
}

void character_create_ui_system::event_button_dex_dec() {
  if (dex_point > 4) {
    dex_point--;
    remain_point++;
  }
}

void character_create_ui_system::event_button_int_inc() {
  if (remain_point > 0) {
    int_point++;
    remain_point--;
  }
}

void character_create_ui_system::event_button_int_dec() {
  if (int_point > 4) {
    int_point--;
    remain_point++;
  }
}

void character_create_ui_system::event_button_luk_inc() {
  if (remain_point > 0) {
    luk_point++;
    remain_point--;
  }
}

void character_create_ui_system::event_button_luk_dec() {
  if (luk_point > 4) {
    luk_point--;
    remain_point++;
  }
}

bool character_create_ui_system::event_button_custom(SDL_Event *event) {
  auto &camera = camera_game_instance::camera;
  auto cx = -440 - camera.x;
  auto cy = -1370 - camera.y;
  cx += 35;
  cy += 30;
  for (uint8_t i = 0; i <= 8; i++) {
    SDL_FRect pos_rect{cx, cy + 18 * i, 256, 17};
    auto &mouse_pos = window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      if (choose_index != i) {
        choose_index = i;
        return true;
      } else {
        std::vector<SDL_FRect> buttons_rect = {
            SDL_FRect{cx + 95, cy + 18 * i, 16, 17},  //
            SDL_FRect{cx + 235, cy + 18 * i, 16, 17}, //
        };
        std::optional<bool> left;
        if (SDL_PointInRectFloat(&mouse_pos, &buttons_rect[0])) {
          left = true;
        } else if (SDL_PointInRectFloat(&mouse_pos, &buttons_rect[1])) {
          left = false;
        }
        if (left.has_value()) {
          switch (choose_index) {
          case 0: {
            reset_character(!gender);
            break;
          }
          case 1: {
            // face
            if (left.value()) {
              event_button_face_prev();
            } else {
              event_button_face_next();
            }
            break;
          }
          case 2: {
            if (left.value()) {
              event_button_hair_prev();
            } else {
              event_button_hair_next();
            }
            break;
          }
          case 3: {
            if (left.value()) {
              event_button_hair_color_prev();
            } else {
              event_button_hair_color_next();
            }
            break;
          }
          case 4: {
            if (left.value()) {
              event_button_skin_color_prev();
            } else {
              event_button_skin_color_next();
            }
            break;
          }
          case 5: {
            if (left.value()) {
              event_button_top_prev();
            } else {
              event_button_top_next();
            }
            break;
          }
          case 6: {
            if (left.value()) {
              event_button_bottom_prev();
            } else {
              event_button_bottom_next();
            }
            break;
          }
          case 7: {
            if (left.value()) {
              event_button_shoes_prev();
            } else {
              event_button_shoes_next();
            }
            break;
          }
          case 8: {
            if (left.value()) {
              event_button_weapon_prev();
            } else {
              event_button_weapon_next();
            }
            break;
          }
          }
        }
      }
    }
  }
  return false;
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
      event_button_str_dec, event_button_str_inc, event_button_dex_dec,
      event_button_dex_inc, event_button_int_dec, event_button_int_inc,
      event_button_luk_dec, event_button_luk_inc, event_button_back,
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
      event_button_custom(event);
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