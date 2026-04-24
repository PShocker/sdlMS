#include "server_foothold_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/server/server/server_foothold.h"

void server_foothold_instance::load_fh(server_scene &s_scene) {
  for (auto [k, v] : foothold_game_instance::load(s_scene.map_id)) {
    s_scene.fhs[k] = server_foothold{v};
  }
}