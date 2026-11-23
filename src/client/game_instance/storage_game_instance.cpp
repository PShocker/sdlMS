#include "src/client/game_instance/storage_game_instance.h"

#include "src/client/game/game_item.h"

#include <memory>

namespace sdlms {

void StorageGameInstance::Load(const GameSaveData& gs) {
  storage_ = gs.storage;
  EnsureSlots();
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

}  // namespace sdlms
