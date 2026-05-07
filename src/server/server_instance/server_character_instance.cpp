#include "server_character_instance.h"
#include "server_client_instance.h"
#include "server_scene_instance.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include <ranges>
#include <utility>

void server_character_instance::save_character_state(
    uint64_t client_id, fbs::ClientCharacterMoveT m) {
  auto &s_client = server_client_instance::clients.at(client_id);
  s_client.fbs_player.character->state->action = m.movement->action;
  s_client.fbs_player.character->state->x = m.movement->x2;
  s_client.fbs_player.character->state->y = m.movement->y2;
  s_client.fbs_player.character->state->flip = m.movement->flip;
  s_client.fbs_player.character->state->fh = m.movement->fh;
}

void server_character_instance::send_character_move(
    uint64_t client_id, fbs::ClientCharacterMoveT m) {
  const auto &s_client = server_client_instance::clients.at(client_id);
  const auto client_map_id = s_client.map_id;
  auto clients = server_scene_instance::scenes[client_map_id].clients;
  clients.erase(client_id);
  for (auto c : clients | std::views::values) {
    fbs::ServerCharacterMoveT t;
    t.client_id = client_id;
    t.movement = std::move(m.movement);
    server_response::server_character_move_response(c.client_id, t);
  }
}

void server_character_instance::handle_character_move(
    uint64_t client_id, fbs::ClientCharacterMoveT m) {
  save_character_state(client_id, m);
  send_character_move(client_id, m);
}