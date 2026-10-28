#include "src/server/request_handler.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_drop_instance.h"
#include "src/server/server_instance/server_mob_instance.h"
#include "src/server/server_instance/server_party_instance.h"
#include "src/server/server_instance/server_reactor_instance.h"
#include "src/server/server_instance/server_region_skill_instance.h"
#include "src/server/server_instance/server_scene_instance.h"
#include "src/server/server_system/server_heartbeat_system.h"
#include "src/client/game/game_character.h"
#include "src/client/game_instance/character_game_instance.h"
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
#include "src/server/server_instance/server_trap_instance.h"
#include "src/server/server_main.h"
#include <cstdio>
#include <memory>
namespace sdlms {



void RequestHandler::HandleRequest(uint64_t client_id, void *buf,
                                     ssize_t nread) {
  auto packet = fbs::GetNetPacket(buf);
  if (!packet) {
    printf("error handle_request\n");
    return;
  }
  switch (packet->payload_type()) {
  case fbs::NetPayload_ClientHeartbeat: {
    ServerHeartbeatSystem::ReceiveClientHeartbeat(client_id);
    break;
  }
  case fbs::NetPayload_ClientScene: {
    auto payload = packet->payload_as_ClientScene();
    fbs::ClientSceneT r;
    payload->UnPackTo(&r);
    ServerSceneInstance::HandleScene(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterMv: {
    auto payload = packet->payload_as_ClientCharacterMv();
    fbs::ClientCharacterMvT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleMv(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterFlip: {
    auto payload = packet->payload_as_ClientCharacterFlip();
    fbs::ClientCharacterFlipT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleFlip(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterAction: {
    auto payload = packet->payload_as_ClientCharacterAction();
    fbs::ClientCharacterActionT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleAction(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterDie: {
    auto payload = packet->payload_as_ClientCharacterDie();
    fbs::ClientCharacterDieT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleDie(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterFc: {
    auto payload = packet->payload_as_ClientCharacterFc();
    fbs::ClientCharacterFcT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleFc(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterAttack: {
    auto payload = packet->payload_as_ClientCharacterAttack();
    fbs::ClientCharacterAttackT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleAttack(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterSkill: {
    auto payload = packet->payload_as_ClientCharacterSkill();
    fbs::ClientCharacterSkillT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleSkill(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientMobAttack: {
    auto payload = packet->payload_as_ClientMobAttack();
    fbs::ClientMobAttackT r;
    payload->UnPackTo(&r);
    ServerMobInstance::HandleAttack(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterChat: {
    auto payload = packet->payload_as_ClientCharacterChat();
    fbs::ClientCharacterChatT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleChat(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterDrop: {
    auto payload = packet->payload_as_ClientCharacterDrop();
    fbs::ClientCharacterDropT r;
    payload->UnPackTo(&r);
    ServerDropInstance::HandleClientDrop(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacter: {
    auto payload = packet->payload_as_ClientCharacter();
    fbs::ClientCharacterT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleCharacter(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterPick: {
    auto payload = packet->payload_as_ClientCharacterPick();
    fbs::ClientCharacterPickT r;
    payload->UnPackTo(&r);
    ServerDropInstance::HandlePick(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterInfo: {
    auto payload = packet->payload_as_ClientCharacterInfo();
    fbs::ClientCharacterInfoT r;
    payload->UnPackTo(&r);
    if (ServerClientInstance::clients_.contains(r.payload)) {
      const auto &character =
          ServerClientInstance::clients_[r.payload].player_t.character;
      fbs::ServerCharacterInfoT sct;
      sct.payload = std::make_unique<fbs::CharacterT>(*character);
      ServerResponse::send_to_client(client_id, sct);
    }
    break;
  }
  case fbs::NetPayload_ClientCharacterTrade: {
    auto payload = packet->payload_as_ClientCharacterTrade();
    fbs::ClientCharacterTradeT r;
    payload->UnPackTo(&r);
    ServerTradeInstance::HandleClient(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterBall: {
    auto payload = packet->payload_as_ClientCharacterBall();
    fbs::ClientCharacterBallT r;
    payload->UnPackTo(&r);
    ServerBallInstance::HandleBall(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterParty: {
    auto payload = packet->payload_as_ClientCharacterParty();
    fbs::ClientCharacterPartyT r;
    payload->UnPackTo(&r);
    ServerPartyInstance::HandleParty(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterState: {
    auto payload = packet->payload_as_ClientCharacterState();
    fbs::ClientCharacterStateT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleState(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientReactor: {
    auto payload = packet->payload_as_ClientReactor();
    fbs::ClientReactorT r;
    payload->UnPackTo(&r);
    ServerReactorInstance::HandleReactor(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCharacterLvUp: {
    auto payload = packet->payload_as_ClientCharacterLvUp();
    fbs::ClientCharacterLvUpT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleLvUp(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCreateMob: {
    auto payload = packet->payload_as_ClientCreateMob();
    fbs::ClientCreateMobT r;
    payload->UnPackTo(&r);
    ServerMobInstance::HandleCreateMob(r);
    break;
  }
  case fbs::NetPayload_ClientTrapAttack: {
    auto payload = packet->payload_as_ClientTrapAttack();
    fbs::ClientTrapAttackT r;
    payload->UnPackTo(&r);
    ServerTrapInstance::HandleAttack(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientDropFade: {
    auto payload = packet->payload_as_ClientDropFade();
    fbs::ClientDropFadeT r;
    payload->UnPackTo(&r);
    ServerDropInstance::HandleClientDropFade(r);
    break;
  }
  case fbs::NetPayload_ClientRSkill: {
    auto payload = packet->payload_as_ClientRSkill();
    fbs::ClientRSkillT r;
    payload->UnPackTo(&r);
    ServerRegionSkillInstance::HandleRskill(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientCreateDrop: {
    auto payload = packet->payload_as_ClientCreateDrop();
    fbs::ClientCreateDropT r;
    payload->UnPackTo(&r);
    ServerDropInstance::HandleClientCreateDrop(client_id, r);
    break;
  }
  case fbs::NetPayload_ClientMobDebuff: {
    auto payload = packet->payload_as_ClientMobDebuff();
    fbs::ClientMobDebuffT r;
    payload->UnPackTo(&r);
    ServerMobInstance::HandleDebuff(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerHeartbeat: {
    ServerHeartbeatSystem::ReceiveServerHeartbeat();
    break;
  }
  case fbs::NetPayload_ServerScene: {
    auto payload = packet->payload_as_ServerScene();
    fbs::ServerSceneT r;
    payload->UnPackTo(&r);
    if (r.fade) {
      SceneSystemInstance::Enter(r.map_id);
      for (const auto &c : r.players) {
        ServerCharacterInstance::HandleServerPlayert(c);
      }
      for (const auto &m : r.mobs) {
        ServerMobInstance::HanleServerMob(m);
      }
      for (const auto &d : r.drops) {
        ServerDropInstance::HandleServerSceneDt(*d);
      }
      for (const auto &r : r.reactors) {
        ServerReactorInstance::HandleSReactor(*r);
      }
    } else {
      Window::dt_time_end_ = Window::dt_now_;
      Window::dt_time_base_ =
          r.time + (Window::dt_time_end_ - Window::dt_time_start_) / 2;
      Window::dt_time_ = Window::dt_time_base_;

      ServerMain::local_addr_ = r.client_id;
      FadeSystemInstance::EnterIn(SceneSystemInstance::EnterFade);
    }
    break;
  }
  case fbs::NetPayload_ServerCharacterIn: {
    auto payload = packet->payload_as_ServerCharacterIn();
    fbs::ServerCharacterInT r;
    payload->UnPackTo(&r);
    // 用户加入，需要把action设置默认jump
    r.player->character->state->action = "jump";
    ServerCharacterInstance::HandleServerPlayert(r.player);
    break;
  }
  case fbs::NetPayload_ServerCharacterMv: {
    auto payload = packet->payload_as_ServerCharacterMv();
    fbs::ServerCharacterMvT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleServerMv(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerCharacterFlip: {
    auto payload = packet->payload_as_ServerCharacterFlip();
    fbs::ServerCharacterFlipT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleServerFlip(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerCharacterAction: {
    auto payload = packet->payload_as_ServerCharacterAction();
    fbs::ServerCharacterActionT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleServerAction(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerCharacterDie: {
    auto payload = packet->payload_as_ServerCharacterDie();
    fbs::ServerCharacterDieT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleServerDie(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerCharacterFc: {
    auto payload = packet->payload_as_ServerCharacterFc();
    fbs::ServerCharacterFcT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleServerFc(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerCharacterOut: {
    auto payload = packet->payload_as_ServerCharacterOut();
    fbs::ServerCharacterOutT r;
    payload->UnPackTo(&r);
    CharacterGameInstance::others_.erase(r.client_id);
    break;
  }
  case fbs::NetPayload_ServerMobEvent: {
    auto payload = packet->payload_as_ServerMobEvent();
    fbs::ServerMobEventT r;
    payload->UnPackTo(&r);
    ServerMobInstance::HandleServerEvent(r);
    break;
  }
  case fbs::NetPayload_ServerCharacterAttack: {
    auto payload = packet->payload_as_ServerCharacterAttack();
    fbs::ServerCharacterAttackT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleServerAtk(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerCharacterSkill: {
    auto payload = packet->payload_as_ServerCharacterSkill();
    fbs::ServerCharacterSkillT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleServerSki(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerMobAttack: {
    auto payload = packet->payload_as_ServerMobAttack();
    fbs::ServerMobAttackT r;
    payload->UnPackTo(&r);
    ServerMobInstance::HandleServerAttack(r);
    break;
  }
  case fbs::NetPayload_ServerCharacterChat: {
    auto payload = packet->payload_as_ServerCharacterChat();
    fbs::ServerCharacterChatT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleServerChat(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerCharacterDrop: {
    auto payload = packet->payload_as_ServerCharacterDrop();
    fbs::ServerCharacterDropT r;
    payload->UnPackTo(&r);
    ServerDropInstance::HandleServerDrop(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerCharacter: {
    auto payload = packet->payload_as_ServerCharacter();
    fbs::ServerCharacterT r;
    payload->UnPackTo(&r);
    if (CharacterGameInstance::others_.contains(r.client_id)) {
      auto &g = CharacterGameInstance::others_[r.client_id].g_character;
      ServerCharacterInstance::LoadGCharacter(g, r.payload);
    }
    break;
  }
  case fbs::NetPayload_ServerCharacterPick: {
    auto payload = packet->payload_as_ServerCharacterPick();
    fbs::ServerCharacterPickT r;
    payload->UnPackTo(&r);
    ServerDropInstance::HandleServerPick(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerCharacterInfo: {
    auto payload = packet->payload_as_ServerCharacterInfo();
    fbs::ServerCharacterInfoT r;
    payload->UnPackTo(&r);
    auto character = ServerCharacterInstance::LoadGCharacter(r.payload);
    CharacterInfoUiSystem::character_ = character;
    CharacterInfoUiSystem::Close();
    CharacterInfoUiSystem::Open();
    break;
  }
  case fbs::NetPayload_ServerCharacterTrade: {
    auto payload = packet->payload_as_ServerCharacterTrade();
    fbs::ServerCharacterTradeT r;
    payload->UnPackTo(&r);
    ServerTradeInstance::HandleServer(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerCharacterBall: {
    auto payload = packet->payload_as_ServerCharacterBall();
    fbs::ServerCharacterBallT r;
    payload->UnPackTo(&r);
    ServerBallInstance::HandleServerBall(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerCharacterParty: {
    auto payload = packet->payload_as_ServerCharacterParty();
    fbs::ServerCharacterPartyT r;
    payload->UnPackTo(&r);
    ServerPartyInstance::HandleServerParty(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerCharacterState: {
    auto payload = packet->payload_as_ServerCharacterState();
    fbs::ServerCharacterStateT r;
    payload->UnPackTo(&r);
    ServerCharacterInstance::HandleServerState(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerDropFade: {
    auto payload = packet->payload_as_ServerDropFade();
    fbs::ServerDropFadeT r;
    payload->UnPackTo(&r);
    ServerDropInstance::HandleServerDropFade(r);
    break;
  }
  case fbs::NetPayload_ServerReactor: {
    auto payload = packet->payload_as_ServerReactor();
    fbs::ServerReactorT r;
    payload->UnPackTo(&r);
    ServerReactorInstance::HandleServerReactor(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerReactorDrop: {
    auto payload = packet->payload_as_ServerReactorDrop();
    fbs::ServerReactorDropT r;
    payload->UnPackTo(&r);
    ServerReactorInstance::HandleServerReactorDrop(r);
    break;
  }
  case fbs::NetPayload_ServerCharacterLvUp: {
    auto payload = packet->payload_as_ServerCharacterLvUp();
    fbs::ServerCharacterLvUpT r;
    payload->UnPackTo(&r);
    if (CharacterGameInstance::others_.contains(r.client_id)) {
      auto &g = CharacterGameInstance::others_[r.client_id].g_character;
      ServerCharacterInstance::HandleLvUp(g);
    }
    break;
  }
  case fbs::NetPayload_ServerCreateMob: {
    auto payload = packet->payload_as_ServerCreateMob();
    fbs::ServerCreateMobT r;
    payload->UnPackTo(&r);
    ServerMobInstance::HandleSCreateMob(r);
    break;
  }
  case fbs::NetPayload_ServerTrapAttack: {
    auto payload = packet->payload_as_ServerTrapAttack();
    fbs::ServerTrapAttackT r;
    payload->UnPackTo(&r);
    ServerTrapInstance::HandleServerAttack(client_id, r);
    break;
  }
  case fbs::NetPayload_ServerRSkill: {
    auto payload = packet->payload_as_ServerRSkill();
    fbs::ServerRSkillT r;
    payload->UnPackTo(&r);
    ServerRegionSkillInstance::HandleSRskill(r);
    break;
  }
  case fbs::NetPayload_ServerCreateDrop: {
    auto payload = packet->payload_as_ServerCreateDrop();
    fbs::ServerCreateDropT r;
    payload->UnPackTo(&r);
    ServerDropInstance::HandleServerCreateDrop(client_id, r);
    break;
  }
  default:
    break;
  }
}

}  // namespace sdlms
