#pragma once

#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <string>

struct quest_mob {
  std::u16string id;
  uint32_t count;
};

struct quest_npc {
  std::u16string id;
};

class game_quest {
public:
  std::u16string quest_id;
  uint8_t index;
  int8_t area;
  bool complete = false;

  std::flat_map<std::u16string, quest_mob> mob;
  std::flat_map<std::u16string, quest_npc> npc;
};