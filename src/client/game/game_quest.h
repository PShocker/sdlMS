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

class game_quest {
public:
  std::u16string quest_id;
  uint8_t index;
  bool complete = false;

  std::vector<quest_mob> mob;
  std::vector<quest_npc> npc;
  std::vector<quest_item> item;

  std::vector<quest_mob> check_mob;
  std::vector<quest_npc> check_npc;
  std::vector<quest_item> check_item;
};