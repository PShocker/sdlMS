#pragma once

#include "src/client/game/game_mob.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>
#include <vector>

using namespace fbs;

struct mob_server_data {
  game_mob mob;
  std::flat_map<MobLogicType, std::vector<MobLogicTypeUnion>> logics;
};

class mob_game_instance {
public:
  static wz::Node *load_mob_info(const std::u16string &id);
  static wz::Node *load_link_mob_node(const std::u16string &id);

  static void load(uint32_t map_id);

  static void load_server_mob(const std::vector<std::unique_ptr<MobT>> &v);

  static void server_mob_logic(const ServerMobLogicT &v);

  static inline std::flat_map<int32_t, mob_server_data> data;
};