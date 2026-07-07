#include "server_trade_instance.h"
#include "server_client_instance.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"

void server_trade_instance::handle_client(uint64_t client_id,
                                          ClientCharacterTradeT &r) {
  if (server_client_instance::clients.contains(r.to_id)) {
    ServerCharacterTradeT sct;
    sct.request = r.request;
    sct.confirm = r.confirm;
    sct.payload = std::move(r.payload);
    if (sct.request) {
      const auto player = server_client_instance::clients[client_id].player_t;
      sct.player = std::make_unique<PlayerT>(player);
    } else {
      const auto player = server_client_instance::clients[r.to_id].player_t;
      sct.player = std::make_unique<PlayerT>(player);
    }
    server_response::send_to_client(r.to_id, sct);
  }
}

void server_trade_instance::handle_server(uint64_t client_id,
                                          ServerCharacterTradeT &r) {
  if (r.confirm) {

  } else if (r.request) {
    // show tip
    auto tips_size = statusbar_ui_system::tips.size();
    game_popup_tip tip{
        .type = popup_tip_enums::trade,
        .pos = {0, 0},
        .player = *r.player,
    };
    statusbar_ui_system::tips.emplace_back(tip);
  } else {
    // reply tip
  }
}