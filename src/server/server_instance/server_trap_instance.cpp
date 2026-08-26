#include "server_trap_instance.h"
#include "server_client_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_effect.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"

void server_trap_instance::handle_attack(uint64_t client_id,
                                         ClientTrapAttackT &r) {
  if (!server_client_instance::clients.contains(client_id)) {
    return;
  }
  // 转发
  auto map_id = r.map_id;
  auto clients = server_scene_instance::scenes.at(map_id).clients;
  clients.erase(client_id);
  ServerTrapAttackT t;
  t.client_id = client_id;
  t.payload = std::move(r.payload);
  for (auto c : clients) {
    server_response::send_to_client(c, t);
  }
}

void server_trap_instance::handle_s_attack(uint64_t client_id, AttackT &at) {
  const auto &attack = at;
  // 伤害数字
  damage_data data = {
      .num = attack.num,
      .type = (damage_data::t)attack.type,
  };
  game_effect d = {
      .id = u"",
      .index = 0,
      .time = 0,
      .delay = attack.delay,
      .type = game_effect::effect_type::damage,
      .pos = SDL_FPoint{attack.x, attack.y - 10},
      .z = true,
      .flip = false,
      .data = data,
  };
  effect_game_instance::data[7].emplace_back(d);
}

void server_trap_instance::handle_server_attack(uint64_t client_id,
                                                ServerTrapAttackT &r) {
  handle_s_attack(client_id, *r.payload);
}