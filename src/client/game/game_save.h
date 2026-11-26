#pragma once

#include "src/client/game/game_character.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_quest.h"
#include "src/client/game/game_storage.h"

#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>

namespace sdlms {

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

struct CraftSave {
  uint8_t type;
  uint8_t level;
  uint32_t exp;
};

struct QuestAlarmSave {
  std::string id;
};

// 宠物存档（095 CPet）：item_id 是宠物道具 id（8 位，如 5000000），
// name 是宠物名字，fullness 是饱食度 0~100。
// 角色可以拥有多只宠物，用 pets 列表保存，active 只有一只是 true。
struct PetSaveData {
  uint32_t item_id = 0;
  std::u16string name;
  uint8_t level = 1;
  uint8_t fullness = 100;
  // 亲密度（095 CPet m_nCloseness）：宠物食品的 incTameness 增加。
  uint8_t closeness = 0;
  bool active = false;
  // 宠物装备道具 id（如 05010000）；0 = 没有穿戴。穿戴时随宠物消息
  // 同步给其他玩家（远端据此画宠物装备层并计算能力位）。
  uint32_t equip_id = 0;
};

// 好友存档：id 为好友的人物唯一标识（高32位创建时间戳+低32位随机数）。
struct FriendSave {
  uint64_t id = 0;
  std::u16string name;
};

struct CharacterSaveData {
  GameCharacter character;
  ApSave ap;
  uint32_t remain_ap;
  SpSave sp;
  std::vector<uint32_t> remain_sp;
  std::vector<PackageSaveData> package;
  std::vector<PetSaveData> pets;
  std::vector<FriendSave> friends;
  uint64_t meso;
  uint32_t map_id;
  uint32_t return_map_id;
  uint32_t portal_id;

  uint32_t hp;
  uint32_t mp;
  uint64_t exp;

  std::vector<GameQuest> quests;
  std::vector<QuestAlarmSave> quest_alarms;
  std::vector<KeySave> keys;
  std::vector<CraftSave> craft;
};

struct GameSaveData {
  std::string username;
  std::vector<CharacterSaveData> characters;
  GameStorage storage;
  // 选项窗口的设置，key = "<tab>/<element>"
  std::flat_map<std::u16string, int32_t> options;
};

}  // namespace sdlms
