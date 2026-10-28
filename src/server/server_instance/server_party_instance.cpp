#include "server_party_instance.h"
#include "server_client_instance.h"
#include "src/client/game/game_party.h"
#include "src/client/game/game_popup_tip.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/party_game_instance.h"
#include "src/client/game_instance/popup_tip_game_instance.h"
#include "src/client/system/ui/party_ui_system.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include <memory>
#include <optional>
namespace sdlms {


ServerParty &ServerPartyInstance::LoadParty(uint64_t captain_id) {
  if (data_.contains(captain_id)) {
    return data_.at(captain_id);
  }
  data_[captain_id] = {.captain = captain_id};
  return data_[captain_id];
}

void ServerPartyInstance::HandleParty(uint64_t client_id,
                                         ClientCharacterPartyT &r) {
  switch (r.step) {
  case 0: {
    auto to_id = r.to_id;
    if (ServerClientInstance::clients_.contains(to_id)) {
      ServerCharacterPartyT scp;
      scp.step = 0;
      scp.to_id = client_id;
      ServerResponse::send_to_client(to_id, scp);
    }
    return;
  }
  case 1: {
    auto captain_id = r.to_id;
    auto party = LoadParty(captain_id);
    party.client_ids.push_back(client_id);
    ServerCharacterPartyT scp;
    scp.to_id = captain_id;
    scp.step = 1;
    const auto captain_t = ServerClientInstance::clients_[captain_id].player_t;
    scp.players.push_back(std::make_unique<PlayerT>(captain_t));
    for (auto c_id : party.client_ids) {
      const auto player_t = ServerClientInstance::clients_[c_id].player_t;
      scp.players.push_back(std::make_unique<PlayerT>(player_t));
      ServerResponse::send_to_client(c_id, scp);
    }
    scp.to_id = 0;
    ServerResponse::send_to_client(captain_id, scp);
    return;
  }
  }
  return;
}

void ServerPartyInstance::HandleServerParty(uint64_t client_id,
                                                ServerCharacterPartyT &r) {
  switch (r.step) {
  case 0: {
    if (!CharacterGameInstance::others_.contains(r.to_id)) {
      return;
    }
    // show popup tip
    PopupTipGameInstance::AddTip(
        PopupTipEnums::kParty,
        CharacterGameInstance::others_[r.to_id].player_t);
    return;
  }
  case 1: {
    PartyGameInstance::data_ = {};
    auto &pt = PartyGameInstance::data_.value();
    pt.captain_id = r.to_id;
    for (const auto &p : r.players) {
      pt.clients.push_back(*p);
    }
    PartyUiSystem::Open();
    return;
  }
  }
}

std::optional<ServerParty>
ServerPartyInstance::LoadClientParty(uint64_t client_id) {
  for (const auto [k, v] : data_) {
    auto clients = v.client_ids;
    clients.push_back(k);
    if (std::ranges::contains(clients, client_id)) {
      return v;
    }
  }
  return std::nullopt;
}

}  // namespace sdlms
