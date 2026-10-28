#include "server_foothold_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/server/server/server_foothold.h"

void ServerFootholdInstance::LoadFh(ServerSceneState &scene) {
  for (auto [k, v] : FootholdGameInstance::Load(scene.map_id)) {
    scene.fhs[k] = ServerFoothold{v};
  }
}