#include "server_foothold_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/server/server/server_foothold.h"

void ServerFootholdInstance::load_fh(ServerSceneState &scene) {
  for (auto [k, v] : FootholdGameInstance::load(scene.map_id)) {
    scene.fhs[k] = ServerFoothold{v};
  }
}