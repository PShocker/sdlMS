#pragma once

#include "src/client/game/game_character.h"
#include "src/client/game/game_item.h"
#include "src/client/game_instance/job_skill_game_instance.h"

#include <wz/Node.h>

#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

class EquipGameInstance {
 public:
  enum class WeaponType : uint8_t {
    kNone = 0,
    kS1A1M1D = 1,
    kSpear = 2,
    kBow = 3,
    kCrossbow = 4,
    kS2A2M2 = 5,
    kWand = 6,
    kClaw = 7,
    kGun = 9,
  };
  enum class WeaponEnum : int {
    kNone = 0,
    kSword1H = 30,
    kAxe1H = 31,
    kMace1H = 32,
    kDagger = 33,
    kWand = 37,
    kStaff = 38,
    kSword2H = 40,
    kAxe2H = 41,
    kMace2H = 42,
    kSpear = 43,
    kPolearm = 44,
    kBow = 45,
    kCrossbow = 46,
    kClaw = 47,
    kKnuckle = 48,
    kGun = 49,
  };
  enum class IncType : uint8_t {
    kWeaponSpeed,
    kWeaponPad,
    kWeaponMad,
    kStr,
    kDex,
    kInt,
    kLuk,
    kPdd,
    kMad,
    kMdd,
    kAcc,
    kEva,
    kCrtr,
    kCrtd,
    kSpeed,
    kJump,
    kMhp,
    kMmp,
  };
  static bool CheckEquip(const std::u16string& id);
  static std::polymorphic<GameItem> LoadItem(const std::u16string& id);

  static WeaponType LoadWeaponType(const GameCharacter& g_character);
  // 由武器道具 id（如 u"01300000"）解析武器大类：
  // 大类码 = id / 10000 后取低两位（130 → SWORD_1H），供伤害公式选倍率
  static WeaponEnum LoadWeaponEnum(const std::u16string& id);
  static wz::Node* LoadEquipInfo(const std::u16string& id);
  // 095 CUser::SetAbilityEquip：戒指槽装备 info 的 nameTag / chatBalloon
  // 字段（点装效果）。点装道具在 Item.wz/Cash/<段>.img/<id>/info，
  // 普通戒指在 Character.wz/Ring/<id>.img/info。返回 0 = 无效果。
  static int32_t LoadRingStyle(const std::u16string& id,
                               const char16_t* field);
  // 按佩戴的一对点装戒指（ring0/ring1 deco）解析效果样式，
  // ring0 优先；都没有时返回 def。
  static int32_t LoadRingStyle(const GameCharacter& g, const char16_t* field,
                               int32_t def);
  static std::u16string LoadEquipName(const std::u16string& id);
  static std::u16string LoadEquipDesc(const std::u16string& id);
  static std::flat_set<JobType> LoadEquipJob(const std::u16string& id);
  static std::u16string LoadEquipType(const std::u16string& id);
  static uint8_t LoadEquipTuc(const std::u16string& id);
  // 095 GW_ItemSlotEquip::GetItemGrade（option & 3）：
  //   0=未强化（白名）1=卷轴成功（蓝名）2=诅咒卷成功（紫名）。
  // 从装备的 scroll 记录推导（任一成功诅咒卷 → 2；否则任一成功 → 1）。
  static int LoadEquipGrade(const GameEquipItem& equip);

  static void AddEquip(GameEquipItem& equip, GameCharacter& character,
                       int slot);
  static bool AddEquipLimit(GameEquipItem& equip, GameCharacter& character,
                            int slot);

  static bool AddEquipDeco(GameDecoItem& deco, GameCharacter& character,
                           int slot);
  static bool AddEquipDecoLimit(GameDecoItem& deco, GameCharacter& character,
                                int slot);

  static std::vector<GameEquipItem> LoadEquipSlot(const std::u16string& id,
                                                  GameCharacter& character);

  static std::vector<GameDecoItem> LoadDecoSlot(const std::u16string& id,
                                                GameCharacter& character);

  static std::flat_map<IncType, int> LoadEquipInc(const std::u16string& id);
  static std::flat_map<IncType, int> LoadScrollInc(const std::u16string& id);

  static std::vector<GameEquipItem> LoadEquips(const GameCharacter& c);
  static std::vector<GameDecoItem> LoadDecos(const GameCharacter& c);

  static int UseEquip(int i, int slot = 0);
  static int UseDeco(int i, int slot = 0);
  static int UseEquipScroll(GameEquipItem& eqp, GameConsumeItem& s);

  static int UnuseEquip(int i);
  static int UnuseDeco(int i);

  static std::optional<GameEquipItem>* LoadEquip(int index);
  static std::optional<GameDecoItem>* LoadDeco(int index);
};

}  // namespace sdlms
