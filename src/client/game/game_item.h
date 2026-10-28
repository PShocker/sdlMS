#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_scroll.h"
#include <cstdint>
#include <string>
#include <vector>
namespace sdlms {


enum class ItemEnum { kEquip, kConsume, kInstall, kEtc, kCash, kDeco };

struct GameItem{
  ItemEnum type;
  std::u16string id;
  uint64_t expire = 0;
};

struct GameEquipItem : public GameItem {
public:
  GameEquipItem() { type = ItemEnum::kEquip; }
  std::vector<GameScroll> scroll;
};

struct GameConsumeItem : public GameItem {
public:
  GameConsumeItem() { type = ItemEnum::kConsume; }
  uint32_t num = 0;
};

struct GameEtcItem : public GameItem {
public:
  GameEtcItem() { type = ItemEnum::kEtc; }
  uint32_t num = 0;
};

class GameInstallItem : public GameItem {
public:
  GameInstallItem() { type = ItemEnum::kInstall; }
};

class GameCashItem : public GameItem {
public:
  GameCashItem() { type = ItemEnum::kCash; }
};

class GameDecoItem : public GameItem {
public:
  GameDecoItem() { type = ItemEnum::kDeco; }
};

}  // namespace sdlms
