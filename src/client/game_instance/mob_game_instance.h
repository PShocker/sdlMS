#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_mob.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <vector>

using namespace fbs;

struct MobServerData {
  GameMob mob;
  std::vector<MovementT> mvs;
};

class MobGameInstance {
public:
  static std::u16string LoadMobName(const std::u16string &id);
  static wz::Node *LoadMobInfo(const std::u16string &id);
  static wz::Node *LoadLinkMobNode(const std::u16string &id);

  static void Load(uint32_t map_id);

  static inline std::flat_map<int32_t, MobServerData> data;
};