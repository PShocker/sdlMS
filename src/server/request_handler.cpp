#include "request_handler.h"
#include "SDL3/SDL_timer.h"
#include "server_instance/server_client_instance.h"
#include "server_instance/server_drop_instance.h"
#include "server_instance/server_mob_instance.h"
#include "server_instance/server_scene_instance.h"
#include "server_system/server_heartbeat_system.h"
#include "server_system_instance/server_system_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_main.h"
#include <algorithm>
#include <cstdio>
#include <ctime>

using namespace fbs;

void request_handler::handle_request(uint64_t client_id, void *buf,
                                     ssize_t nread) {
  auto packet = GetNetPacket(buf);
  if (!packet) {
    printf("error handle_request\n");
    return;
  }
  switch (packet->payload_type()) {
  case NetPayload_ClientHeartbeat: {
    server_heartbeat_system::receive_client_heartbeat(client_id);
    break;
  }
  case NetPayload_ClientScene: {
    auto payload = packet->payload_as_ClientScene();
    fbs::ClientSceneT r;
    payload->UnPackTo(&r);
    server_scene_instance::handle_scene(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterLogic: {
    auto payload = packet->payload_as_ClientCharacterLogic();
    fbs::ClientCharacterLogicT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_logic(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterAttack: {
    auto payload = packet->payload_as_ClientCharacterAttack();
    fbs::ClientCharacterAttackT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_attack(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterSkill: {
    auto payload = packet->payload_as_ClientCharacterSkill();
    fbs::ClientCharacterSkillT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_skill(client_id, r);
    break;
  }
  case NetPayload_ClientMobAttack: {
    auto payload = packet->payload_as_ClientMobAttack();
    fbs::ClientMobAttackT r;
    payload->UnPackTo(&r);
    server_mob_instance::handle_attack(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterChat: {
    auto payload = packet->payload_as_ClientCharacterChat();
    fbs::ClientCharacterChatT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_chat(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterDrop: {
    auto payload = packet->payload_as_ClientCharacterDrop();
    fbs::ClientCharacterDropT r;
    payload->UnPackTo(&r);
    server_drop_instance::handle_drop(client_id, r);
    break;
  }
  case NetPayload_ClientCharacter: {
    auto payload = packet->payload_as_ClientCharacter();
    fbs::ClientCharacterT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_character(client_id, r);
    break;
  }
  case NetPayload_ServerHeartbeat: {
    server_heartbeat_system::receive_server_heartbeat();
    break;
  }
  case NetPayload_ServerScene: {
    auto payload = packet->payload_as_ServerScene();
    fbs::ServerSceneT r;
    payload->UnPackTo(&r);
    scene_system_instance::enter(r.map_id);
    for (const auto &c : r.players) {
      character_game_instance::load_others_character(c);
    }
    mob_game_instance::load_server_mob(r.mobs);
    break;
  }
  case NetPayload_ServerCharacterIn: {
    auto payload = packet->payload_as_ServerCharacterIn();
    fbs::ServerCharacterInT r;
    payload->UnPackTo(&r);
    // 用户加入，需要把action设置默认jump
    r.player->character->state->action = "jump";
    character_game_instance::load_others_character(r.player);
    break;
  }
  case NetPayload_ServerCharacterLogic: {
    auto payload = packet->payload_as_ServerCharacterLogic();
    fbs::ServerCharacterLogicT r;
    payload->UnPackTo(&r);
    if (character_game_instance::others.contains(r.payload->client_id)) {
      auto &logics =
          character_game_instance::others[r.payload->client_id].logics;
      auto type = r.payload->payload.type;
      logics[type].push_back(r.payload->payload);
    }
    break;
  }
  case NetPayload_ServerCharacterOut: {
    auto payload = packet->payload_as_ServerCharacterOut();
    fbs::ServerCharacterOutT r;
    payload->UnPackTo(&r);
    character_game_instance::exit_others_character(r.client_id);
    break;
  }
  case NetPayload_ServerMobLogic: {
    auto payload = packet->payload_as_ServerMobLogic();
    fbs::ServerMobLogicT r;
    payload->UnPackTo(&r);
    mob_game_instance::server_mob_logic(r);
    break;
  }
  case NetPayload_ServerCharacterAttack: {
    auto payload = packet->payload_as_ServerCharacterAttack();
    fbs::ServerCharacterAttackT r;
    payload->UnPackTo(&r);
    if (character_game_instance::others.contains(r.client_id)) {
      auto &g_character =
          character_game_instance::others.at(r.client_id).g_character;
      character_game_instance::load_character_attack(r.payload, g_character);
    }

    break;
  }
  case NetPayload_ServerCharacterSkill: {
    auto payload = packet->payload_as_ServerCharacterSkill();
    fbs::ServerCharacterSkillT r;
    payload->UnPackTo(&r);
    if (character_game_instance::others.contains(r.client_id)) {
      auto &g_character =
          character_game_instance::others.at(r.client_id).g_character;
      character_game_instance::load_character_skill(r.ski_id, r.payload,
                                                    g_character);
    }
    break;
  }
  case NetPayload_ServerMobAttack: {
    auto payload = packet->payload_as_ServerMobAttack();
    fbs::ServerMobAttackT r;
    payload->UnPackTo(&r);
    mob_game_instance::load_mob_attack(r.client_id, r.payload.get());
    break;
  }
  case NetPayload_ServerCharacterDrop: {
    auto payload = packet->payload_as_ServerCharacterDrop();
    fbs::ServerCharacterDropT r;
    payload->UnPackTo(&r);
    cursor_game_instance::server_cursor_drop(*r.payload);
    drop_game_instance::load_drop(*r.payload);
    break;
  }
  case NetPayload_ServerCharacter: {
    auto payload = packet->payload_as_ServerCharacter();
    fbs::ServerCharacterT r;
    payload->UnPackTo(&r);
    if (character_game_instance::others.contains(r.client_id)) {
      auto &g_character =
          character_game_instance::others.at(r.client_id).g_character;
      character_game_instance::load_others_character(r.payload, g_character);
    }
    break;
  }
  default:
    break;
  }
}