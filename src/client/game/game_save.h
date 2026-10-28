#pragma once

#include "game_item.h"
#include "game_storage.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_quest.h"
#include <cstdint>
#include <string>
#include <vector>

struct ApSave {
  uint32_t hp_ap;
  uint32_t mp_ap;
  uint32_t str_ap;
  uint32_t dex_ap;
  uint32_t int_ap;
  uint32_t luk_ap;
};

struct SpSave {
  std::flat_map<uint32_t, uint8_t> ski_sp;
};

struct PackageSaveData {
  uint32_t index;
  std::polymorphic<GameItem> val;
};

struct KeySave {
  uint16_t scan_code;
  std::string type;
  std::string val;
  std::string sub_val;
};

struct CharacterSaveData {
  GameCharacter character;
  ApSave ap;
  uint32_t remain_ap;
  SpSave sp;
  std::vector<uint32_t> remain_sp;
  std::vector<PackageSaveData> package;
  uint64_t meso;
  uint32_t map_id;
  uint32_t return_map_id;
  uint32_t portal_id;

  uint32_t hp;
  uint32_t mp;
  uint64_t exp;

  std::vector<GameQuest> quests;
  std::vector<KeySave> keys;
};

class GameSaveData {
public:
  std::string username;
  std::vector<CharacterSaveData> characters;
  GameStorage storage;
};