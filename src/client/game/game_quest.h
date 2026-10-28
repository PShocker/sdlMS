#ifndef SDLMS_SRC_CLIENT_GAME_GAME_QUEST_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_QUEST_H_

#include <wz/Node.h>
#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>
namespace sdlms {


struct QuestMob {
  std::u16string id;
  uint32_t count;
};

struct QuestItem {
  std::u16string id;
  uint32_t count;
};

struct QuestNpc {
  std::u16string id;
};

enum class QuestEnum : uint8_t {
  kProgress,
  kComplete,
  kDecline,
};

struct GameQuest{
  std::u16string quest_id;
  uint8_t index;

  QuestEnum type;

  std::flat_map<std::u16string, QuestMob> mob;
  std::flat_map<std::u16string, QuestItem> item;

  std::flat_map<std::u16string, QuestMob> check_mob;
  std::flat_map<std::u16string, QuestItem> check_item;

  bool mob_bool;
  bool item_bool;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_QUEST_H_
