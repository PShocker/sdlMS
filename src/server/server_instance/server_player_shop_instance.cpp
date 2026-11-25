#include "src/server/server_instance/server_player_shop_instance.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/ui/player_shop_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_scene_instance.h"

#include <algorithm>
#include <memory>

namespace sdlms {

namespace {

bool ClientAlive(uint64_t client_id) {
  return client_id != 0 &&
         ServerClientInstance::clients_.contains(client_id);
}

void SendTo(uint64_t client_id, fbs::ServerCharacterShopT& m) {
  if (ClientAlive(client_id)) {
    ServerResponse::SendToClient(client_id, m);
  }
}

fbs::ServerCharacterShopT MakeStep(uint8_t step) {
  fbs::ServerCharacterShopT m;
  m.step = step;
  return m;
}

// 货架副本 → fbs（下发进店）。
void FillGoods(fbs::ServerCharacterShopT& m, const ServerPlayerShop& shop) {
  for (const auto& g : shop.goods) {
    fbs::ShopGoodsT gg;
    gg.item_id = g.item_id;
    gg.num = g.num;
    gg.price = g.price;
    m.goods.push_back(std::make_unique<fbs::ShopGoodsT>(gg));
  }
}

// 同图玩家广播：有人开摊/撤摊（095 摆摊招牌可见性）。
void BroadcastStall(uint64_t owner, bool open,
                    const std::u16string& title) {
  const auto& shop = ServerPlayerShopInstance::data_.at(owner);
  for (const auto& [key, scene] : ServerSceneInstance::scenes_) {
    if (scene.map_id != shop.map_id) {
      continue;
    }
    for (auto c_id : scene.clients) {
      if (c_id == owner) {
        continue;
      }
      fbs::ServerCharacterShopT m = MakeStep(5);
      m.to_id = owner;
      m.x = open ? 1 : 0;
      if (ServerClientInstance::clients_.contains(owner)) {
        m.player = std::make_unique<fbs::PlayerT>(
            ServerClientInstance::clients_.at(owner).player_t);
      }
      m.chat = {title.begin(), title.end()};
      SendTo(c_id, m);
    }
  }
}

}  // namespace

void ServerPlayerShopInstance::HandleClient(uint64_t client_id,
                                      fbs::ClientCharacterShopT& r) {
  if (!ClientAlive(client_id)) {
    return;
  }

  switch (r.step) {
    // 开摊（095 PutItem 后开张）。
    case 0: {
      if (data_.contains(client_id)) {
        return;  // 已有摊
      }
      // 找玩家所在地图。
      uint32_t map_id = 0;
      for (const auto& [key, scene] : ServerSceneInstance::scenes_) {
        if (scene.clients.contains(client_id)) {
          map_id = scene.map_id;
          break;
        }
      }
      if (map_id == 0) {
        return;
      }
      ServerPlayerShop shop;
      shop.owner = client_id;
      shop.map_id = map_id;
      shop.title = {r.chat.begin(), r.chat.end()};
      if (shop.title.empty()) {
        shop.title = u"商店";
      }
      for (const auto& g : r.goods) {
        if (shop.goods.size() >= ServerPlayerShop::kMaxGoods) {
          break;
        }
        if (g->item_id == 0 || g->num == 0) {
          continue;
        }
        ServerPlayerShop::Goods goods;
        goods.item_id = g->item_id;
        goods.num = g->num;
        goods.price = g->price;
        shop.goods.push_back(goods);
      }
      data_[client_id] = shop;

      // 摊主确认 + 同图招牌广播。
      fbs::ServerCharacterShopT ok = MakeStep(0);
      ok.to_id = client_id;
      ok.chat = {shop.title.begin(), shop.title.end()};
      SendTo(client_id, ok);
      BroadcastStall(client_id, true, shop.title);
      return;
    }
    // 进店浏览（095 OnEnter）。
    case 1: {
      auto it = data_.find(r.to_id);
      if (it == data_.end()) {
        fbs::ServerCharacterShopT m = MakeStep(6);
        std::u16string text = u"对方没有在摆摊。";
        m.chat = {text.begin(), text.end()};
        SendTo(client_id, m);
        return;
      }
      // 登记店内顾客（重复进店不重复登记）。
      if (client_id != it->second.owner &&
          !std::ranges::contains(it->second.visitors, client_id)) {
        it->second.visitors.push_back(client_id);
      }
      fbs::ServerCharacterShopT m = MakeStep(1);
      m.to_id = r.to_id;
      if (ServerClientInstance::clients_.contains(r.to_id)) {
        m.player = std::make_unique<fbs::PlayerT>(
            ServerClientInstance::clients_.at(r.to_id).player_t);
      }
      m.chat = {it->second.title.begin(), it->second.title.end()};
      FillGoods(m, it->second);
      SendTo(client_id, m);
      return;
    }
    // 购买（095 BuyItem）。
    case 2: {
      auto it = data_.find(r.to_id);
      if (it == data_.end()) {
        fbs::ServerCharacterShopT m = MakeStep(2);
        m.x = 0;
        m.y = 1;  // 货架空/无效
        SendTo(client_id, m);
        return;
      }
      auto& shop = it->second;
      if (r.x >= shop.goods.size()) {
        fbs::ServerCharacterShopT m = MakeStep(2);
        m.x = 0;
        m.y = 1;
        SendTo(client_id, m);
        return;
      }
      auto& goods = shop.goods[r.x];
      uint32_t buy = r.y == 0 ? 1 : r.y;
      if (buy > goods.num || buy > ServerPlayerShop::kMaxBuyNum) {
        fbs::ServerCharacterShopT m = MakeStep(2);
        m.x = 0;
        m.y = 1;  // 库存不足
        SendTo(client_id, m);
        return;
      }

      // 转发给摊主：售出通知（095 OnSoldItemResult）。
      // 摊主客户端本地加钱、扣物品。
      {
        fbs::ServerCharacterShopT m = MakeStep(4);
        m.to_id = client_id;  // 买家
        m.x = r.x;
        m.y = buy;
        m.meso = goods.price * buy;
        SendTo(shop.owner, m);
      }
      goods.num -= buy;
      goods.sold += buy;

      // 回买家成功（095 OnBuyResult）。
      {
        fbs::ServerCharacterShopT m = MakeStep(2);
        m.x = 1;
        m.y = r.x;  // 商品序号（客户端按已发请求本地结算）
        SendTo(client_id, m);
      }

      // 售罄则通知摊主下架（货架刷新）。
      if (goods.num == 0) {
        fbs::ServerCharacterShopT m = MakeStep(4);
        m.to_id = 0;
        m.x = r.x;
        m.y = 0;
        SendTo(shop.owner, m);
      }
      return;
    }
    // 关摊（摊主）/ 离店（顾客）。
    case 3: {
      auto it = data_.find(client_id);
      if (it != data_.end()) {
        // 摊主关摊：通知店内顾客。
        for (auto v : it->second.visitors) {
          fbs::ServerCharacterShopT m3 = MakeStep(3);
          m3.to_id = client_id;
          SendTo(v, m3);
        }
        std::u16string title = it->second.title;
        // 先广播撤摊（BroadcastStall 需要摊位数据），再删表。
        BroadcastStall(client_id, false, title);
        data_.erase(it);
        // 摊主客户端清理摊位状态。
        fbs::ServerCharacterShopT m = MakeStep(3);
        m.to_id = client_id;
        SendTo(client_id, m);
        return;
      }
      // 顾客离店：从店内名单移除。
      for (auto it2 = data_.begin(); it2 != data_.end(); ++it2) {
        std::erase_if(it2->second.visitors, [client_id](uint64_t v) {
          return v == client_id;
        });
      }
      return;
    }
    // 调整货架（仅摊主，重新上架完整货架）。
    case 4: {
      auto it = data_.find(client_id);
      if (it == data_.end()) {
        return;
      }
      it->second.goods.clear();
      for (const auto& g : r.goods) {
        if (it->second.goods.size() >= ServerPlayerShop::kMaxGoods) {
          break;
        }
        if (g->item_id == 0 || g->num == 0) {
          continue;
        }
        ServerPlayerShop::Goods goods;
        goods.item_id = g->item_id;
        goods.num = g->num;
        goods.price = g->price;
        it->second.goods.push_back(goods);
      }
      // 回摊主确认（重新同步货架）。
      fbs::ServerCharacterShopT m = MakeStep(0);
      m.to_id = client_id;
      m.chat = {it->second.title.begin(), it->second.title.end()};
      FillGoods(m, it->second);
      SendTo(client_id, m);
      return;
    }
    // 店内聊天（095 CMiniRoomBaseDlg::OnChat）：广播给摊主与店内顾客。
    case 5: {
      auto it = data_.find(r.to_id);
      if (it == data_.end() || r.chat.empty()) {
        return;
      }
      auto& shop = it->second;
      bool in_room = client_id == shop.owner ||
                     std::ranges::contains(shop.visitors, client_id);
      if (!in_room) {
        return;
      }
      fbs::ServerCharacterShopT m = MakeStep(7);
      if (ServerClientInstance::clients_.contains(client_id)) {
        m.player = std::make_unique<fbs::PlayerT>(
            ServerClientInstance::clients_.at(client_id).player_t);
      }
      m.chat = r.chat;
      SendTo(shop.owner, m);
      for (auto v : shop.visitors) {
        SendTo(v, m);
      }
      return;
    }
    default: {
      return;
    }
  }
}

bool ServerPlayerShopInstance::Run() {
  // 摊主掉线 → 撤摊；顾客掉线 → 移出店内名单。
  for (auto it = data_.begin(); it != data_.end();) {
    if (!ClientAlive(it->first)) {
      const uint64_t owner = it->first;
      const uint32_t map_id = it->second.map_id;
      // 店内顾客先收到关店通知。
      for (auto v : it->second.visitors) {
        fbs::ServerCharacterShopT m3 = MakeStep(3);
        m3.to_id = owner;
        SendTo(v, m3);
      }
      const std::u16string title = it->second.title;
      it = data_.erase(it);
      // owner 已不在线：撤摊广播只发给同图其它人。
      for (const auto& [key, scene] : ServerSceneInstance::scenes_) {
        if (scene.map_id != map_id) {
          continue;
        }
        for (auto c_id : scene.clients) {
          if (c_id == owner) {
            continue;
          }
          fbs::ServerCharacterShopT m = MakeStep(5);
          m.x = 0;
          m.to_id = owner;
          SendTo(c_id, m);
        }
      }
      (void)title;
      continue;
    }
    // 顾客掉线：移出店内名单。
    std::erase_if(it->second.visitors, [](uint64_t v) {
      return !ClientAlive(v);
    });
    ++it;
  }
  return true;
}

void ServerPlayerShopInstance::HandleServer(uint64_t client_id,
                                      fbs::ServerCharacterShopT& r) {
  // 客户端状态由 PlayerShopUiSystem 维护（与 Minigame 同款分流）。
  PlayerShopUiSystem::HandleServer(client_id, r);
}

}  // namespace sdlms