#include "src/client/game_instance/storage_game_instance.h"

#include "src/client/game/game_item.h"
#include "src/client/game_instance/item_game_instance.h"

#include <memory>

namespace sdlms {

void StorageGameInstance::Load(const GameSaveData& gs) {
  storage_ = gs.storage;
  EnsureSlots();
  NormalizeTypes();
}

void StorageGameInstance::EnsureSlots() {
  auto& data = storage_.data;
  if (data.size() >= kSlotCount) {
    return;
  }
  const std::polymorphic<GameItem> empty =
      std::polymorphic<GameItem>(std::in_place_type<GameEtcItem>);
  data.resize(kSlotCount, empty);
}

void StorageGameInstance::NormalizeTypes() {
  // 旧存档里可叠加道具是按 id/num 写进 GameEtcItem 占位格的，类型会残留
  // 成 Etc，导致取回到 Etc 页签。这里按 id 前缀把类型纠正回来。
  for (auto& slot : storage_.data) {
    if (slot->id.empty() || slot->type != ItemEnum::kEtc) {
      continue;
    }
    const auto type = ItemGameInstance::LoadItemType(slot->id);
    if (type == u"Consume") {
      GameConsumeItem itm;
      itm.id = slot->id;
      itm.expire = slot->expire;
      itm.num = ItemGameInstance::LoadItemNum(slot);
      slot = std::polymorphic<GameItem>(itm);
    } else if (type == u"Ins") {
      GameInstallItem itm;
      itm.id = slot->id;
      itm.expire = slot->expire;
      slot = std::polymorphic<GameItem>(itm);
    } else if (type == u"Cash" || type == u"Pet") {
      GameCashItem itm;
      itm.id = slot->id;
      itm.expire = slot->expire;
      slot = std::polymorphic<GameItem>(itm);
    }
  }
}

}  // namespace sdlms
