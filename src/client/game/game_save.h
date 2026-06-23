#pragma once

#include "game_equip.h"
#include "game_item.h"
#include "game_storage.h"
#include "src/client/game/game_character.h"
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

struct ap_save {
  uint32_t hp_ap;
  uint32_t mp_ap;
  uint32_t str_ap;
  uint32_t dex_ap;
  uint32_t int_ap;
  uint32_t luk_ap;
};

struct sp_save {
  std::flat_map<uint32_t, uint8_t> ski_sp;
};

struct package_save {
  uint32_t index;
  std::variant<game_equip, game_item> val;
};

struct character_save {
  game_character character;
  ap_save ap;
  sp_save sp;
  std::vector<package_save> package;
  uint64_t meso;
  uint32_t map_id;

  uint32_t hp;
  uint32_t mp;
  uint64_t exp;
};

class game_save {
public:
  std::string username;
  std::vector<character_save> characters;
  game_storage storage;
};