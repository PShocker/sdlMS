#include "server_trade_instance.h"
#include "server_client_instance.h"
#include "src/client/game_instance/popup_tip_game_instance.h"
#include "src/client/system/ui/trade_ui_system.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_character_instance.h"
#include <memory>

void ServerTradeInstance::handle_client(uint64_t client_id,
                                          ClientCharacterTradeT &r) {
  switch (r.step) {
  case 0: {
    if (ServerClientInstance::clients.contains(r.to_id)) {
      ServerCharacterTradeT sct;
      sct.step = 0;
      const auto client_t = ServerClientInstance::clients[client_id].player_t;
      sct.player = std::make_unique<PlayerT>(client_t);
      ServerResponse::send_to_client(r.to_id, sct);
    }
    return;
  }
  case 1: {
    // 建立交易
    const auto to_t = ServerClientInstance::clients[r.to_id].player_t;
    const auto client_t = ServerClientInstance::clients[client_id].player_t;
    ServerCharacterTradeT sct;
    sct.step = 1;
    sct.player = std::make_unique<PlayerT>(client_t);
    ServerResponse::send_to_client(r.to_id, sct);
    sct.player = std::make_unique<PlayerT>(to_t);
    ServerResponse::send_to_client(client_id, sct);
    return;
  }
  }
}

void ServerTradeInstance::handle_server(uint64_t client_id,
                                          ServerCharacterTradeT &r) {
  switch (r.step) {
  case 0: {
    PopupTipGameInstance::add_tip(PopupTipEnums::kTrade, *r.player);
    break;
  }
  case 1: {
    //  建立交易
    TradeUiSystem::other =
        ServerCharacterInstance::load_g_character(r.player->character);
    TradeUiSystem::close();
    TradeUiSystem::open();
    break;
  }
  }
}