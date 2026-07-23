#include "server_party_instance.h"
#include "server_client_instance.h"
#include "src/client/game/game_popup_tip.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/popup_tip_game_instance.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"

void server_party_instance::handle_party(uint64_t client_id,
                                         ClientCharacterPartyT &r) {
  if (r.step == 0) {
    auto to_id = r.to_id;
    if (server_client_instance::clients.contains(to_id)) {
      ServerCharacterPartyT scp;
      scp.to_id = client_id;
      server_response::send_to_client(to_id, scp);
    }
    return;
  }
  if (r.confirm) {
    auto captain = r.to_id;
    // create party
  }
}

void server_party_instance::handle_server_party(uint64_t client_id,
                                                ServerCharacterPartyT &r) {
  if (r.step == 0) {
    if (!character_game_instance::others.contains(r.to_id)) {
      return;
    }
    // show popup tip
    popup_tip_game_instance::load_tip(
        popup_tip_enums::party,
        character_game_instance::others[r.to_id].player_t);
  }
}