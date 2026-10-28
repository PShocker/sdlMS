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

ServerParty &ServerPartyInstance::load_party(uint64_t captain_id) {
  if (data.contains(captain_id)) {
    return data.at(captain_id);
  }
  data[captain_id] = {.captain = captain_id};
  return data[captain_id];
}

void ServerPartyInstance::handle_party(uint64_t client_id,
                                         ClientCharacterPartyT &r) {
  switch (r.step) {
  case 0: {
    auto to_id = r.to_id;
    if (ServerClientInstance::clients.contains(to_id)) {
      ServerCharacterPartyT scp;
      scp.step = 0;
      scp.to_id = client_id;
      ServerResponse::send_to_client(to_id, scp);
    }
    return;
  }
  case 1: {
    auto captain_id = r.to_id;
    auto party = load_party(captain_id);
    party.client_ids.push_back(client_id);
    ServerCharacterPartyT scp;
    scp.to_id = captain_id;
    scp.step = 1;
    const auto captain_t = ServerClientInstance::clients[captain_id].player_t;
    scp.players.push_back(std::make_unique<PlayerT>(captain_t));
    for (auto c_id : party.client_ids) {
      const auto player_t = ServerClientInstance::clients[c_id].player_t;
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

void ServerPartyInstance::handle_server_party(uint64_t client_id,
                                                ServerCharacterPartyT &r) {
  switch (r.step) {
  case 0: {
    if (!CharacterGameInstance::others.contains(r.to_id)) {
      return;
    }
    // show popup tip
    PopupTipGameInstance::add_tip(
        PopupTipEnums::kParty,
        CharacterGameInstance::others[r.to_id].player_t);
    return;
  }
  case 1: {
    PartyGameInstance::data = {};
    auto &pt = PartyGameInstance::data.value();
    pt.captain_id = r.to_id;
    for (const auto &p : r.players) {
      pt.clients.push_back(*p);
    }
    PartyUiSystem::open();
    return;
  }
  }
}

std::optional<ServerParty>
ServerPartyInstance::load_client_party(uint64_t client_id) {
  for (const auto [k, v] : data) {
    auto clients = v.client_ids;
    clients.push_back(k);
    if (std::ranges::contains(clients, client_id)) {
      return v;
    }
  }
  return std::nullopt;
}