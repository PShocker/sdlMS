#include "worldmap_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>

void worldmap_ui_system::render_backgrnd() {
  auto back_node = wz_resource::map->find(u"WorldMap/" + path + u"/BaseImg/0");
  auto texture = wz_resource::load_texture(back_node);
  auto w = texture->w + 12;
  auto h = texture->h + 44;
  static auto bord_node = wz_resource::ui->find(u"WorldMap.img/Border");
  static auto lt = wz_resource::load_texture(bord_node->get_child(u"0"));
  static auto t = wz_resource::load_texture(bord_node->get_child(u"1"));
  static auto rt = wz_resource::load_texture(bord_node->get_child(u"2"));
  static auto lm = wz_resource::load_texture(bord_node->get_child(u"3"));
  static auto rm = wz_resource::load_texture(bord_node->get_child(u"4"));
  static auto lb = wz_resource::load_texture(bord_node->get_child(u"5"));
  static auto b = wz_resource::load_texture(bord_node->get_child(u"6"));
  static auto rb = wz_resource::load_texture(bord_node->get_child(u"7"));
  static auto title =
      wz_resource::load_texture(wz_resource::ui->find(u"WorldMap.img/title"));

  SDL_FRect pos_rect = {pos.x, pos.y, (float)lt->w, (float)lt->h};
  SDL_RenderTexture(window::renderer, lt, nullptr, &pos_rect);
  pos_rect = {pos.x + 7, pos.y, static_cast<float>(w - 14), (float)t->h};
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  pos_rect = {pos.x + w - 7, pos.y, static_cast<float>(rt->w), (float)t->h};
  SDL_RenderTexture(window::renderer, rt, nullptr, &pos_rect);
  pos_rect = {pos.x, pos.y + 32, static_cast<float>(lm->w),
              (float)h - b->h - 32};
  SDL_RenderTexture(window::renderer, lm, nullptr, &pos_rect);
  pos_rect = {pos.x + w - 7, pos.y + 32, static_cast<float>(rm->w),
              (float)h - b->h - 32};
  SDL_RenderTexture(window::renderer, rm, nullptr, &pos_rect);
  pos_rect = {pos.x, pos.y + h - b->h, static_cast<float>(lb->w), (float)lb->h};
  SDL_RenderTexture(window::renderer, lb, nullptr, &pos_rect);
  pos_rect = {pos.x + 7, pos.y + h - b->h, static_cast<float>(w - 14),
              (float)b->h};
  SDL_RenderTexture(window::renderer, b, nullptr, &pos_rect);
  pos_rect = {pos.x + w - 7, pos.y + h - b->h, static_cast<float>(rb->w),
              (float)rb->h};
  SDL_RenderTexture(window::renderer, rb, nullptr, &pos_rect);
  pos_rect = {static_cast<float>((int32_t)pos.x + 10),
              static_cast<float>((int32_t)pos.y + 10),
              static_cast<float>(title->w), (float)title->h};
  SDL_RenderTexture(window::renderer, title, nullptr, &pos_rect);
}

void worldmap_ui_system::render_spot() {
}

void worldmap_ui_system::render_map() {
  auto back_node = wz_resource::map->find(u"WorldMap/" + path + u"/BaseImg/0");
  auto texture = wz_resource::load_texture(back_node);
  SDL_FRect pos_rect = {pos.x + 6, pos.y + 30, static_cast<float>(texture->w),
                        (float)texture->h};
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

bool worldmap_ui_system::render() {
  render_backgrnd();
  render_map();
  return true;
}

void worldmap_ui_system::init_pos() {
  auto back_node = wz_resource::map->find(u"WorldMap/" + path + u"/BaseImg/0");
  auto texture = wz_resource::load_texture(back_node);
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - texture->w) / 2 - 6;
  pos.y = (camera.h - texture->h) / 2 - 44;
}

void worldmap_ui_system::open() {
  auto camera = camera_game_instance::camera;
  worldmap_ui_system::path = u"WorldMap001.img";
  worldmap_ui_system::init_pos();
  system::render_systems.push_back(worldmap_ui_system::render);
  system::event_systems.push_back(worldmap_ui_system::event);
}

void worldmap_ui_system::close() {
  worldmap_ui_system::path.clear();
  std::erase(system::render_systems, worldmap_ui_system::render);
  std::erase(system::event_systems, worldmap_ui_system::event);
}

bool worldmap_ui_system::event(SDL_Event *event) { return true; }

void worldmap_ui_system::toggle() {
  auto fn = &worldmap_ui_system::render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}