#include "map_info_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <string>

wz::Node *map_info_game_instance::load(uint32_t map_id) {
  auto map_node = wz_resource::load_map_node(map_id);
  auto map_info = map_node->get_child(u"info");
  return map_info;
}

SDL_FRect map_info_game_instance::load_vr_border(uint32_t map_id) {
  SDL_FRect r;
  auto map_info = load(map_id);
  r.x = static_cast<wz::Property<int> *>(map_info->get_child(u"VRTop"))->get();
  r.y = static_cast<wz::Property<int> *>(map_info->get_child(u"VRLeft"))->get();
  r.w =
      static_cast<wz::Property<int> *>(map_info->get_child(u"VRBottom"))->get();
  r.h =
      static_cast<wz::Property<int> *>(map_info->get_child(u"VRRight"))->get();
  return r;
}