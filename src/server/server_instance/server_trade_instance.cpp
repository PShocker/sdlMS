#include "src/server/server_instance/server_trade_instance.h"

#include "src/client/game/game_popup_tip.h"
#include "src/client/game_instance/popup_tip_game_instance.h"
#include "src/client/system/ui/trade_ui_system.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/request/client_request.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_client_instance.h"

#include <flat_map>
#include <flat_set>
#include <memory>
#include <vector>

namespace sdlms {

namespace {

// 交易配对：client_id -> 对方 client_id
std::flat_map<uint64_t, uint64_t> g_trade_pair;
// 已确认（锁定）的 client_id
std::flat_set<uint64_t> g_trade_locked;
// 双方各自放入交易的物品
std::flat_map<uint64_t, std::vector<fbs::ItemUnionUnion>> g_trade_offers;
// 双方各自放入交易的金币
std::flat_map<uint64_t, uint64_t> g_trade_meso;

void ClearTrade(uint64_t a, uint64_t b) {
  g_trade_locked.erase(a);
  g_trade_locked.erase(b);
  g_trade_pair.erase(a);
  g_trade_pair.erase(b);
  g_trade_offers.erase(a);
  g_trade_offers.erase(b);
  g_trade_meso.erase(a);
  g_trade_meso.erase(b);
}

}  // namespace

void ServerTradeInstance::HandleClient(uint64_t client_id,
                                       fbs::ClientCharacterTradeT& r) {
  switch (r.step) {
    case 0: {
      // 邀请：把发起者信息转发给对方。
      if (!ServerClientInstance::clients_.contains(r.to_id)) {
        return;
      }
      fbs::ServerCharacterTradeT sct;
      sct.step = 0;
      sct.player = std::make_unique<fbs::PlayerT>(
          ServerClientInstance::clients_[client_id].player_t);
      ServerResponse::SendToClient(r.to_id, sct);
      return;
    }
    case 1: {
      // 接受：建立交易，双方各自收到对方玩家信息并开窗。
      if (!ServerClientInstance::clients_.contains(r.to_id)) {
        return;
      }
      g_trade_pair[client_id] = r.to_id;
      g_trade_pair[r.to_id] = client_id;

      fbs::ServerCharacterTradeT sct;
      sct.step = 1;
      sct.player = std::make_unique<fbs::PlayerT>(
          ServerClientInstance::clients_[client_id].player_t);
      ServerResponse::SendToClient(r.to_id, sct);
      sct.player = std::make_unique<fbs::PlayerT>(
          ServerClientInstance::clients_[r.to_id].player_t);
      ServerResponse::SendToClient(client_id, sct);
      return;
    }
    case 2: {
      // 确认/锁定：双方都确认后发送成交(step=4)，各自带上对方物品用于结算。
      g_trade_locked.insert(client_id);
      auto it = g_trade_pair.find(client_id);
      if (it == g_trade_pair.end()) {
        return;
      }
      const uint64_t other = it->second;
      if (!g_trade_locked.contains(other)) {
        return;
      }
      fbs::ServerCharacterTradeT sct;
      sct.step = 4;  // 成交
      // client_id 收到对方的物品与金币
      sct.payload = g_trade_offers[other];
      sct.meso = g_trade_meso[other];
      ServerResponse::SendToClient(client_id, sct);
      // other 收到 client_id 的物品与金币
      sct.payload = g_trade_offers[client_id];
      sct.meso = g_trade_meso[client_id];
      ServerResponse::SendToClient(other, sct);
      ClearTrade(client_id, other);
      return;
    }
    case 3: {
      // 取消：通知对方关闭。
      auto it = g_trade_pair.find(client_id);
      if (it == g_trade_pair.end()) {
        return;
      }
      const uint64_t other = it->second;
      fbs::ServerCharacterTradeT sct;
      sct.step = 5;  // 取消
      ServerResponse::SendToClient(other, sct);
      ClearTrade(client_id, other);
      return;
    }
    case 10: {
      // 更新报价：记录本方物品与金币，转发给对方显示。
      g_trade_offers[client_id] = r.payload;
      g_trade_meso[client_id] = r.meso;
      auto it = g_trade_pair.find(client_id);
      if (it == g_trade_pair.end()) {
        return;
      }
      fbs::ServerCharacterTradeT sct;
      sct.step = 6;  // 对方报价更新
      sct.payload = g_trade_offers[client_id];
      sct.meso = g_trade_meso[client_id];
      ServerResponse::SendToClient(it->second, sct);
      return;
    }
    default: {
      return;
    }
  }
}

void ServerTradeInstance::HandleServer(uint64_t client_id,
                                       fbs::ServerCharacterTradeT& r) {
  switch (r.step) {
    case 0: {
      // 收到邀请：弹出交易提示条，由玩家点击接受后才建立交易。
      if (r.player == nullptr) {
        return;
      }
      PopupTipGameInstance::AddTip(PopupTipEnums::kTrade, *r.player);
      break;
    }
    case 1: {
      // 建立交易：打开交易窗口，显示对方。
      if (r.player == nullptr) {
        return;
      }
      TradeUiSystem::other_ =
          ServerCharacterInstance::LoadGCharacter(r.player->character);
      TradeUiSystem::other_id_ = r.player->client_id;
      TradeUiSystem::Close();
      TradeUiSystem::Open();
      break;
    }
    case 6: {
      // 对方更新了报价：记录用于显示。
      TradeUiSystem::other_payload_ = std::move(r.payload);
      TradeUiSystem::other_meso_ = r.meso;
      break;
    }
    case 4: {
      // 成交：结算物品与金币并关闭。
      TradeUiSystem::other_payload_ = std::move(r.payload);
      TradeUiSystem::other_meso_ = r.meso;
      TradeUiSystem::ApplyTrade();
      TradeUiSystem::Close();
      break;
    }
    case 5: {
      TradeUiSystem::Close();
      break;
    }
    default: {
      break;
    }
  }
}

}  // namespace sdlms