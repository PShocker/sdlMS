#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_MOB_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_MOB_GAME_INSTANCE_H_

#include "src/client/game/game_mob.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>

#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <vector>
namespace sdlms {


struct MobServerData {
  GameMob mob;
  std::vector<fbs::MovementT> mvs;
};

class MobGameInstance {
 public:
  static std::u16string LoadMobName(const std::u16string& id);
  static wz::Node* LoadMobInfo(const std::u16string& id);
  static wz::Node* LoadLinkMobNode(const std::u16string& id);

  static void Load(uint32_t map_id);

  static inline std::flat_map<int32_t, MobServerData> data_;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_MOB_GAME_INSTANCE_H_
