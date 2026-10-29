#include "src/server/server_instance/server_trade_instance.h"

#include "src/client/game_instance/popup_tip_game_instance.h"
#include "src/client/system/ui/trade_ui_system.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_client_instance.h"

#include <memory>

namespace sdlms {

void ServerTradeInstance::HandleClient(uint64_t client_id,
                                       fbs::ClientCharacterTradeT& r) {
  switch (r.step) {
    case 0: {
      if (ServerClientInstance::clients_.contains(r.to_id)) {
        fbs::ServerCharacterTradeT sct;
        sct.step = 0;
        const auto client_t =
            ServerClientInstance::clients_[client_id].player_t;
        sct.player = std::make_unique<fbs::PlayerT>(client_t);
        ServerResponse::SendToClient(r.to_id, sct);
      }
      return;
    }
    case 1: {
      // 建立交易
      const auto to_t = ServerClientInstance::clients_[r.to_id].player_t;
      const auto client_t = ServerClientInstance::clients_[client_id].player_t;
      fbs::ServerCharacterTradeT sct;
      sct.step = 1;
      sct.player = std::make_unique<fbs::PlayerT>(client_t);
      ServerResponse::SendToClient(r.to_id, sct);
      sct.player = std::make_unique<fbs::PlayerT>(to_t);
      ServerResponse::SendToClient(client_id, sct);
      return;
    }
  }
}

void ServerTradeInstance::HandleServer(uint64_t client_id,
                                       fbs::ServerCharacterTradeT& r) {
  switch (r.step) {
    case 0: {
      PopupTipGameInstance::AddTip(PopupTipEnums::kTrade, *r.player);
      break;
    }
    case 1: {
      //  建立交易
      TradeUiSystem::other_ =
          ServerCharacterInstance::LoadGCharacter(r.player->character);
      TradeUiSystem::Close();
      TradeUiSystem::Open();
      break;
    }
  }
}

}  // namespace sdlms
