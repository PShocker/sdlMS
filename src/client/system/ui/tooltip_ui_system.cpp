#include "tooltip_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

void tooltip_ui_system::render_equip(game_equip &equip, float x, float y) {
  // backgrnd
  static auto backgrnd_node = wz_resource::ui->find(u"UIToolTip.img/Item");
  static auto top =
      wz_resource::load_texture(backgrnd_node->find(u"Frame/top"));
  static auto line =
      wz_resource::load_texture(backgrnd_node->find(u"Frame/line"));
  static auto bottom =
      wz_resource::load_texture(backgrnd_node->find(u"Frame/bottom"));
  const auto h = 300;
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
}

void tooltip_ui_system::render_skill(std::u16string id, uint8_t level, float x,
                                     float y) {
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
  rect.w = 200;
  rect.h = 200;
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
      y + rect.h - texture_sw->h,
  };
  rect.x = (p_sw.x);
  rect.y = (p_sw.y);
  rect.w = texture_sw->w;
  rect.h = texture_sw->h;
  SDL_RenderTexture(window::renderer, texture_sw, nullptr, &rect);

  // 右上
  SDL_FPoint p_ne{
      x + rect.w - texture_ne->w,
      y,
  };
  rect.x = (p_ne.x);
  rect.y = (p_ne.y);
  rect.w = texture_ne->w;
  rect.h = texture_ne->h;
  SDL_RenderTexture(window::renderer, texture_ne, nullptr, &rect);

  // 右下
  SDL_FPoint p_se{
      x + rect.w - texture_ne->w,
      y + rect.h - texture_sw->h,
  };
  rect.x = (p_se.x);
  rect.y = (p_se.y);
  rect.w = texture_se->w;
  rect.h = texture_se->h;
  SDL_RenderTexture(window::renderer, texture_se, nullptr, &rect);
}

void tooltip_ui_system::render_item(game_item &item, float x, float y) {
  static auto node = wz_resource::string->find(u"Etc.img");
  auto node2 = node->get_child(item.id);
  auto name =
      static_cast<wz::Property<std::u16string> *>(node2->get_child(u"name"))
          ->get();
  auto desc =
      static_cast<wz::Property<std::u16string> *>(node2->get_child(u"desc"))
          ->get();
}

void tooltip_ui_system::render_world_map_info(uint32_t id, float x, float y) {
  // backgrnd
  static auto backgrnd_node = wz_resource::ui->find(u"UIToolTip.img/Item");
  static auto top =
      wz_resource::load_texture(backgrnd_node->find(u"Frame/top"));
  static auto line =
      wz_resource::load_texture(backgrnd_node->find(u"Frame/line"));
  static auto bottom =
      wz_resource::load_texture(backgrnd_node->find(u"Frame/bottom"));
  const auto h = 100;
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
}