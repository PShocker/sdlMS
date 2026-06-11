#include "login_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

SDL_FPoint login_ui_system::load_pos() {
  SDL_FPoint pos;
  const auto w = 1366;
  const auto h = 768;
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - w) / 2;
  pos.y = (camera.w - h) / 2;
  return pos;
}

void login_ui_system::render_button() {
  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"Login.img/ClassicIntro/button:login"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/button:login_saved"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/button:find_id"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/button:find_pw"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/button:register"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/button:homepage"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/button:quit"),
  };
  std::array buttons_rect = {
      SDL_FRect{640, 237, 122, 61}, // login
      SDL_FRect{450, 328, 101, 29}, // login_saved
      SDL_FRect{584, 324, 101, 29}, // find_id
      SDL_FRect{677, 324, 101, 29}, // find_pw
      SDL_FRect{391, 389, 101, 29}, // register
      SDL_FRect{518, 389, 101, 29}, // homepage
      SDL_FRect{645, 389, 108, 55}, // quit

  };
}

void login_ui_system::render_backgrnd() {
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

void login_ui_system::render_effect() {
  auto pos = load_pos();
  const static std::array effect_nodes = {
      wz_resource::ui->find(u"Login.img/ClassicIntro/animation:effect0"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/animation:effect1"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/animation:effect2"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/animation:effect3"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/animation:effect4"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/animation:effect5"),
  };
  const static std::vector<std::vector<uint32_t>> delays = {
      {1500, 1500, 1000}, //
      {2500, 2500},       //
      {1700, 1700, 1700}, //
      {3000, 3000},       //
      {2800, 2800},       //
      {1000, 1000, 2500}, //
  };
  for (auto i = 0; i < effect_nodes.size(); i++) {
    auto ds = delays[i];
    auto sum = std::accumulate(ds.begin(), ds.end(), 0u);
    auto offset = window::dt_now % sum; // 取余，得到周期内偏移
    uint32_t accumulated = 0;
    uint8_t render_index = 0;
    for (size_t i = 0; i < ds.size(); i++) {
      if (offset < accumulated + ds[i]) {
        render_index = i;
        break;
      }
      accumulated += ds[i];
    }
    // render
    auto index = std::to_string(render_index);
    auto node = effect_nodes[i]->get_child(index);
    uint8_t a0 = 255;
    if (node->get_child(u"a0")) {
      a0 = static_cast<wz::Property<int> *>(node->get_child(u"a0"))->get();
    }
    uint8_t a1 = 255;
    if (node->get_child(u"a1")) {
      a1 = static_cast<wz::Property<int> *>(node->get_child(u"a1"))->get();
    }
    auto ani_time = offset - accumulated;
    float t = (float)ani_time / (float)ds[i];
    auto alpha = a0 + (a1 - a0) * t;
    auto origin = wz_resource::load_fpoint(node->get_child(u"origin"));
    auto texture = wz_resource::load_texture(node);
    SDL_FRect pos_rect {
      pos.x - origin.x, pos.y - origin.y, static_cast<float>(texture->w),
          static_cast<float>(texture->h),
    };
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }
}