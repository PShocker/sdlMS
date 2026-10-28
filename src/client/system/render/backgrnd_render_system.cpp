#include "src/client/system/render/backgrnd_render_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_render.h>
#include <wz/Property.h>

#include <cstdint>
#include <string>
namespace sdlms {


bool BackgrndRenderSystem::Render(GameBackgrnd& g_backgrnd) {
  auto delta_time = Window::delta_time_;

  auto viewprot_x = CameraGameInstance::camera_.x;
  auto viewprot_y = CameraGameInstance::camera_.y;
  auto viewprot_w = CameraGameInstance::camera_.w;
  auto viewprot_h = CameraGameInstance::camera_.h;

  SDL_Texture* texture;
  auto back_node = WzResource::map_->find(g_backgrnd.path);
  if (g_backgrnd.ani) {
    auto index =
        std::to_string(g_backgrnd.ani_index % back_node->children_count());
    back_node = back_node->get_child(index);
  }
  texture = WzResource::LoadTexture(back_node);

  auto v =
      static_cast<wz::Property<wz::WzVec2D>*>(back_node->get_child(u"origin"))
          ->get();
  auto origin = SDL_FPoint{static_cast<float>(v.x), static_cast<float>(v.y)};

  SDL_FPoint point{g_backgrnd.pos.x - v.x, g_backgrnd.pos.y - v.y};

  point.x += g_backgrnd.offset_x;
  point.y += g_backgrnd.offset_y;

  auto cx = g_backgrnd.cx;
  auto cy = g_backgrnd.cy;

  auto tile_cnt_x = 1;
  if (g_backgrnd.htile && cx > 0) {
    auto tile_start_right = int32_t(point.x + texture->w - viewprot_x) % cx;
    if (tile_start_right <= 0) {
      tile_start_right = tile_start_right + cx;
    }
    tile_start_right = tile_start_right + viewprot_x;

    auto tile_start_left = tile_start_right - texture->w;
    if (tile_start_left >= viewprot_x + viewprot_w) {
      tile_cnt_x = 0;
    } else {
      tile_cnt_x =
          ceil((viewprot_x + viewprot_w - tile_start_left) / float(cx));
      point.x = tile_start_left;
    }
  }

  auto tile_cnt_y = 1;
  if (g_backgrnd.vtile && cy > 0) {
    auto tile_start_bottom = int32_t(point.y + texture->h - viewprot_y) % cy;
    if (tile_start_bottom <= 0) {
      tile_start_bottom = tile_start_bottom + cy;
    }
    tile_start_bottom = tile_start_bottom + viewprot_y;

    auto tile_start_top = tile_start_bottom - texture->h;
    if (tile_start_top >= viewprot_y + viewprot_h) {
      tile_cnt_y = 0;
    } else {
      tile_cnt_y = ceil((viewprot_y + viewprot_h - tile_start_top) / float(cy));
      point.y = tile_start_top;
    }
  }

  if (cx == texture->w && cy == texture->h && g_backgrnd.flip == 0) {
    SDL_FRect dst_rect = {
        (float)point.x - viewprot_x,  // x 起始位置
        (float)point.y - viewprot_y,  // y 起始位置
        (float)tile_cnt_x * cx,       // 总宽度
        (float)tile_cnt_y * cy        // 总高度
    };
    SDL_RenderTextureTiled(Window::renderer_, texture, nullptr, 1, &dst_rect);
  } else {
    for (int i = 0; i < tile_cnt_y; i++) {
      for (int j = 0; j < tile_cnt_x; j++) {
        SDL_FRect dst_rect = {
            (float)point.x + j * cx - viewprot_x,  // x 起始位置
            (float)point.y + i * cy - viewprot_y,  // y 起始位置
            (float)texture->w,                     // 总宽度
            (float)texture->h                      // 总高度
        };
        SDL_RenderTextureRotated(Window::renderer_, texture, nullptr, &dst_rect,
                                 0, nullptr, (SDL_FlipMode)g_backgrnd.flip);
      }
    }
  }

  return true;
}

}  // namespace sdlms
