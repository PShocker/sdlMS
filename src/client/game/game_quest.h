#pragma once

#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>

struct quest_mob {
  std::u16string id;
  uint32_t count;
};

struct quest_item {
  std::u16string id;
  uint32_t count;
};

struct quest_npc {
  std::u16string id;
};

enum class quest_enum : uint8_t {
  progress,
  complete,
  decline,
};

class game_quest {
public:
  std::u16string quest_id;
  uint8_t index;

  quest_enum type;

  std::flat_map<std::u16string, quest_mob> mob;
  std::flat_map<std::u16string, quest_npc> npc;
  std::flat_map<std::u16string, quest_item> item;

  std::flat_map<std::u16string, quest_mob> check_mob;
  std::flat_map<std::u16string, quest_npc> check_npc;
  std::flat_map<std::u16string, quest_item> check_item;
};