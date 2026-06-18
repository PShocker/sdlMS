#pragma once

#include "game_equip.h"
#include "game_item.h"
#include "game_storage.h"
#include "src/client/game/game_character.h"
#include <variant>
#include <vector>
class game_save {
public:
  std::vector<game_character> characters;
  game_storage storage;
};