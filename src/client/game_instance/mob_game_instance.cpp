#include "mob_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "foothold_game_instance.h"
#include "src/client/game/game_mob.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

wz::Node *mob_game_instance::load_link_mob_node(const std::u16string &id) {
  auto mob_node = wz_resource::mob->find(id + u".img");
  while (mob_node->find(u"info/link")) {
    auto link_node = mob_node->find(u"info/link");
    auto link = static_cast<wz::Property<std::u16string> *>(link_node)->get();
    mob_node = wz_resource::mob->find(link + u".img");
  }
  return mob_node;
}

std::optional<SDL_FRect>
mob_game_instance::load_mob_rect(const game_mob &g_mob) {
  auto mob_node = load_link_mob_node(g_mob.id);
  auto mob_action_node = mob_node->get_child(g_mob.action);
  bool zigzag = mob_action_node->get_child(u"zigzag") == nullptr ? false : true;
  int32_t canvas_count = mob_action_node->children_count();
  if (zigzag) {
    canvas_count -= 1;
  }
  std::string frame_index;
  if (zigzag && g_mob.ani_index >= canvas_count) {
    frame_index =
        std::to_string(canvas_count - 2 - (g_mob.ani_index % canvas_count));
  } else {
    frame_index = std::to_string(g_mob.ani_index);
  }
  auto texture_node = mob_action_node->get_child(frame_index);
  if (texture_node->get_child(u"lt") == nullptr) {
    return std::nullopt;
  }
  auto lt = wz_resource::load_fpoint(texture_node->get_child(u"lt"));
  auto rb = wz_resource::load_fpoint(texture_node->get_child(u"rb"));
  return SDL_FRect{
      .x = lt.x,
      .y = lt.y,
      .w = rb.x - lt.x,
      .h = rb.y - lt.y,
  };
}

void mob_game_instance::load(uint32_t map_id) {
  data = {};

  auto map_node = wz_resource::load_map_node(map_id);
  auto map_life_node = map_node->get_child(u"life");
  auto fhs = foothold_game_instance::load(map_id);

  for (auto [key, val] : *map_life_node->get_children()) {
    auto type =
        static_cast<wz::Property<std::u16string> *>(val[0]->get_child(u"type"))
            ->get();
    if (type == u"n") {
      continue;
    }
    game_mob g_mob;
    auto mob_node = val[0];
    g_mob.index = std::stoi(std::string{key.begin(), key.end()});
    g_mob.id =
        static_cast<wz::Property<std::u16string> *>(mob_node->get_child(u"id"))
            ->get();
    g_mob.ani_index = 0;
    g_mob.ani_time = 0;
    g_mob.ani_animate = true;

    data[g_mob.index] = {g_mob, {}};
  }
}

void mob_game_instance::load_server_mob(
    const std::vector<std::unique_ptr<fbs::MobT>> &v) {
  for (const auto &m : v) {
    if (data.contains(m->mob_index)) {
      auto &mob = data.at(m->mob_index).mob;
      const auto &state = m->state;
      mob.action = std::u16string{state->action.begin(), state->action.end()};
      mob.page = state->page;
      mob.flip = state->flip;
      mob.pos.x = state->x;
      mob.pos.y = state->y;
    } else {
      // summon mob
    }
  }
}

void mob_game_instance::server_mob_logic(const ServerMobLogicT &v) {
  if (v.map_id != scene_system_instance::map_id) {
    return;
  }
  for (const auto &m : v.payload) {
    if (data.contains(m->mob_index)) {
      auto &logics = data.at(m->mob_index).logics;
      logics[m->payload.type].push_back(m->payload);
    }
  }
}