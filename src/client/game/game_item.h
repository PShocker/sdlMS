#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_scroll.h"
#include <cstdint>
#include <string>
#include <vector>

enum class item_enum { equip, consume, etc, install, cash };

class game_item {
public:
  item_enum type;
  std::u16string id;
  uint64_t expire = 0;
};

class game_equip_item : public game_item {
public:
  game_equip_item() { type = item_enum::equip; }
  std::vector<game_scroll> scroll;
};

class game_consume_item : public game_item {
public:
  game_consume_item() { type = item_enum::consume; }
  uint32_t num = 0;
};

class game_etc_item : public game_item {
public:
  game_etc_item() { type = item_enum::etc; }
  uint32_t num = 0;
};

class game_install_item : public game_item {
public:
  game_install_item() { type = item_enum::install; }
};

class game_cash_item : public game_item {
public:
  game_cash_item() { type = item_enum::cash; }
};
