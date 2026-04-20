#include "worldmap_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

void worldmap_ui_system::render_backgrnd() {
  auto back_node = wz_resource::map->find(u"WorldMap/" + path + u"/BaseImg/0");
  auto texture = wz_resource::load_texture(back_node);
  auto w = texture->w + 12;
  auto h = texture->h + 44;
  back_node = wz_resource::ui->find(u"WorldMap.img/Border");
  auto lt = wz_resource::load_texture(back_node->get_child(u"0"));
  auto t = wz_resource::load_texture(back_node->get_child(u"1"));
  auto rt = wz_resource::load_texture(back_node->get_child(u"2"));
  auto lm = wz_resource::load_texture(back_node->get_child(u"3"));
  auto rm = wz_resource::load_texture(back_node->get_child(u"4"));
  auto lb = wz_resource::load_texture(back_node->get_child(u"5"));
  auto b = wz_resource::load_texture(back_node->get_child(u"6"));
  auto rb = wz_resource::load_texture(back_node->get_child(u"7"));

  SDL_FRect pos_rect = {pos.x, pos.y, (float)lt->w, (float)lt->h};
  SDL_RenderTexture(window::renderer, lt, nullptr, &pos_rect);
  pos_rect = {pos.x + 7, pos.y, static_cast<float>(w - 14), (float)t->h};
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  pos_rect = {pos.x + w - 7, pos.y, static_cast<float>(rt->w), (float)t->h};
  SDL_RenderTexture(window::renderer, rt, nullptr, &pos_rect);
  pos_rect = {pos.x, pos.y + 32, static_cast<float>(lm->w),
              (float)h - b->h - 32};
  SDL_RenderTexture(window::renderer, lm, nullptr, &pos_rect);
}

bool worldmap_ui_system::render() {
  render_backgrnd();
  return true;
}

void worldmap_ui_system::open() {
  auto camera = camera_game_instance::camera;
  worldmap_ui_system::pos = {};
  worldmap_ui_system::path = u"WorldMap000.img";
  system::render_systems.push_back(worldmap_ui_system::render);
  system::event_systems.push_back(worldmap_ui_system::event);
}

void worldmap_ui_system::close() {
  worldmap_ui_system::path.clear();
  std::erase(system::render_systems, worldmap_ui_system::render);
  std::erase(system::event_systems, worldmap_ui_system::event);
}

bool worldmap_ui_system::event(SDL_Event *event) { return true; }
