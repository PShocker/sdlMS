#include "ui_texture.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

void ui_texture::render() {
  auto texture_node = wz_resource::ui->find(path);
  auto texture = wz_resource::load_texture(texture_node);
  SDL_FRect pos_rect = {
      .x = static_cast<float>(pos.x),
      .y = static_cast<float>(pos.y),
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}