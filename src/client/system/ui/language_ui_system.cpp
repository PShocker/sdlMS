#include "language_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

void language_ui_system::render_backgrnd() {
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"MultiLang.img/selectLanguage/backgrnd"));
  const auto &camera = camera_game_instance::camera;
  SDL_FRect pos_rect;
  pos_rect.x = (camera.w - t->w) / 2;
  pos_rect.x = (camera.w - t->h) / 2;
  pos_rect.w = t->w;
  pos_rect.h = t->h;
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
}

bool language_ui_system::render() {
  render_backgrnd();

  return true;
}