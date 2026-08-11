#include "request_handler.h"
#include "SDL3/SDL_timer.h"
#include "server_instance/server_ball_instance.h"
#include "server_instance/server_client_instance.h"
#include "server_instance/server_drop_instance.h"
#include "server_instance/server_mob_instance.h"
#include "server_instance/server_party_instance.h"
#include "server_instance/server_scene_instance.h"
#include "server_system/server_heartbeat_system.h"
#include "server_system_instance/server_system_instance.h"
#include "src/client/game/game_character.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/system/ui/character_info_ui_system.h"
#include "src/client/system_instance/fade_system_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_trade_instance.h"
#include "src/server/server_main.h"
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <memory>
#include <utility>

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
  case NetPayload_ClientCharacterMv: {
    auto payload = packet->payload_as_ClientCharacterMv();
    fbs::ClientCharacterMvT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_mv(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterFlip: {
    auto payload = packet->payload_as_ClientCharacterFlip();
    fbs::ClientCharacterFlipT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_flip(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterAction: {
    auto payload = packet->payload_as_ClientCharacterAction();
    fbs::ClientCharacterActionT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_action(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterDie: {
    auto payload = packet->payload_as_ClientCharacterDie();
    fbs::ClientCharacterDieT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_die(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterFc: {
    auto payload = packet->payload_as_ClientCharacterFc();
    fbs::ClientCharacterFcT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_fc(client_id, r);
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
    server_drop_instance::handle_client_drop(client_id, r);
    break;
  }
  case NetPayload_ClientCharacter: {
    auto payload = packet->payload_as_ClientCharacter();
    fbs::ClientCharacterT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_character(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterPick: {
    auto payload = packet->payload_as_ClientCharacterPick();
    fbs::ClientCharacterPickT r;
    payload->UnPackTo(&r);
    server_drop_instance::handle_pick(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterInfo: {
    auto payload = packet->payload_as_ClientCharacterInfo();
    fbs::ClientCharacterInfoT r;
    payload->UnPackTo(&r);
    if (server_client_instance::clients.contains(r.payload)) {
      const auto &character =
          server_client_instance::clients[r.payload].player_t.character;
      ServerCharacterInfoT sct;
      sct.payload = std::make_unique<CharacterT>(*character);
      server_response::send_to_client(client_id, sct);
    }
    break;
  }
  case NetPayload_ClientCharacterTrade: {
    auto payload = packet->payload_as_ClientCharacterTrade();
    fbs::ClientCharacterTradeT r;
    payload->UnPackTo(&r);
    server_trade_instance::handle_client(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterBall: {
    auto payload = packet->payload_as_ClientCharacterBall();
    fbs::ClientCharacterBallT r;
    payload->UnPackTo(&r);
    server_ball_instance::handle_ball(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterParty: {
    auto payload = packet->payload_as_ClientCharacterParty();
    fbs::ClientCharacterPartyT r;
    payload->UnPackTo(&r);
    server_party_instance::handle_party(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterState: {
    auto payload = packet->payload_as_ClientCharacterState();
    fbs::ClientCharacterStateT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_state(client_id, r);
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
    if (r.fade) {
      scene_system_instance::enter(r.map_id);
      for (const auto &c : r.players) {
        server_character_instance::handle_server_playert(c);
      }
      for (const auto &m : r.mobs) {
        server_mob_instance::hanle_server_mob(m);
      }
      for (const auto &d : r.drops) {
        server_drop_instance::handle_server_scene_dt(*d);
      }
    } else {
      fade_system_instance::enter_in(scene_system_instance::enter_fade);
    }
    break;
  }
  case NetPayload_ServerCharacterIn: {
    auto payload = packet->payload_as_ServerCharacterIn();
    fbs::ServerCharacterInT r;
    payload->UnPackTo(&r);
    // 用户加入，需要把action设置默认jump
    r.player->character->state->action = "jump";
    server_character_instance::handle_server_playert(r.player);
    break;
  }
  case NetPayload_ServerCharacterMv: {
    auto payload = packet->payload_as_ServerCharacterMv();
    fbs::ServerCharacterMvT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_server_mv(client_id, r);
    break;
  }
  case NetPayload_ServerCharacterFlip: {
    auto payload = packet->payload_as_ServerCharacterFlip();
    fbs::ServerCharacterFlipT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_server_flip(client_id, r);
    break;
  }
  case NetPayload_ServerCharacterAction: {
    auto payload = packet->payload_as_ServerCharacterAction();
    fbs::ServerCharacterActionT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_server_action(client_id, r);
    break;
  }
  case NetPayload_ServerCharacterDie: {
    auto payload = packet->payload_as_ServerCharacterDie();
    fbs::ServerCharacterDieT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_server_die(client_id, r);
    break;
  }
  case NetPayload_ServerCharacterFc: {
    auto payload = packet->payload_as_ServerCharacterFc();
    fbs::ServerCharacterFcT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_server_fc(client_id, r);
    break;
  }
  case NetPayload_ServerCharacterOut: {
    auto payload = packet->payload_as_ServerCharacterOut();
    fbs::ServerCharacterOutT r;
    payload->UnPackTo(&r);
    character_game_instance::others.erase(r.client_id);
    break;
  }
  case NetPayload_ServerMobEvent: {
    auto payload = packet->payload_as_ServerMobEvent();
    fbs::ServerMobEventT r;
    payload->UnPackTo(&r);
    server_mob_instance::handle_server_event(r);
    break;
  }
  case NetPayload_ServerCharacterAttack: {
    auto payload = packet->payload_as_ServerCharacterAttack();
    fbs::ServerCharacterAttackT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_server_atk(client_id, r);
    break;
  }
  case NetPayload_ServerCharacterSkill: {
    auto payload = packet->payload_as_ServerCharacterSkill();
    fbs::ServerCharacterSkillT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_server_ski(client_id, r);
    break;
  }
  case NetPayload_ServerMobAttack: {
    auto payload = packet->payload_as_ServerMobAttack();
    fbs::ServerMobAttackT r;
    payload->UnPackTo(&r);
    server_mob_instance::handle_server_attack(r);
    break;
  }
  case NetPayload_ServerCharacterChat: {
    auto payload = packet->payload_as_ServerCharacterChat();
    fbs::ServerCharacterChatT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_server_chat(client_id, r);
    break;
  }
  case NetPayload_ServerCharacterDrop: {
    auto payload = packet->payload_as_ServerCharacterDrop();
    fbs::ServerCharacterDropT r;
    payload->UnPackTo(&r);
    server_drop_instance::handle_server_drop(client_id, r);
    break;
  }
  case NetPayload_ServerCharacter: {
    auto payload = packet->payload_as_ServerCharacter();
    fbs::ServerCharacterT r;
    payload->UnPackTo(&r);
    if (character_game_instance::others.contains(r.client_id)) {
      auto &g = character_game_instance::others[r.client_id].g_character;
      server_character_instance::load_g_character(g, r.payload);
    }
    break;
  }
  case NetPayload_ServerCharacterPick: {
    auto payload = packet->payload_as_ServerCharacterPick();
    fbs::ServerCharacterPickT r;
    payload->UnPackTo(&r);
    server_drop_instance::handle_server_pick(client_id, r);
    break;
  }
  case NetPayload_ServerCharacterInfo: {
    auto payload = packet->payload_as_ServerCharacterInfo();
    fbs::ServerCharacterInfoT r;
    payload->UnPackTo(&r);
    auto character = server_character_instance::load_g_character(r.payload);
    character_info_ui_system::character = character;
    character_info_ui_system::close();
    character_info_ui_system::open();
    break;
  }
  case NetPayload_ServerCharacterTrade: {
    auto payload = packet->payload_as_ServerCharacterTrade();
    fbs::ServerCharacterTradeT r;
    payload->UnPackTo(&r);
    server_trade_instance::handle_server(client_id, r);
    break;
  }
  case NetPayload_ServerCharacterBall: {
    auto payload = packet->payload_as_ServerCharacterBall();
    fbs::ServerCharacterBallT r;
    payload->UnPackTo(&r);
    server_ball_instance::handle_server_ball(client_id, r);
    break;
  }
  case NetPayload_ServerCharacterParty: {
    auto payload = packet->payload_as_ServerCharacterParty();
    fbs::ServerCharacterPartyT r;
    payload->UnPackTo(&r);
    server_party_instance::handle_server_party(client_id, r);
    break;
  }
  case NetPayload_ServerCharacterState: {
    auto payload = packet->payload_as_ServerCharacterState();
    fbs::ServerCharacterStateT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_server_state(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerDropFade: {
    auto payload = packet->payload_as_ServerDropFade();
    fbs::ServerDropFadeT r;
    payload->UnPackTo(&r);
    server_drop_instance::handle_server_drop_fade(r);
    break;
  }
  default:
    break;
  }
}