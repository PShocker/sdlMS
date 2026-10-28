#pragma once

#include "src/client/game/game_character.h"
#include "src/client/game/game_item.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <vector>
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
  static bool check_equip(const std::u16string &id);
  static std::polymorphic<GameItem> load_item(const std::u16string &id);

  static WeaponType load_weapon_type(const GameCharacter &g_character);
  // 由武器道具 id（如 u"01300000"）解析武器大类：
  // 大类码 = id / 10000 后取低两位（130 → SWORD_1H），供伤害公式选倍率
  static WeaponEnum load_weapon_enum(const std::u16string &id);
  static wz::Node *load_equip_info(const std::u16string &id);
  static std::u16string load_equip_name(const std::u16string &id);
  static std::u16string load_equip_desc(const std::u16string &id);
  static std::flat_set<JobType> load_equip_job(const std::u16string &id);
  static std::u16string load_equip_type(const std::u16string &id);
  static uint8_t load_equip_tuc(const std::u16string &id);

  static void add_equip(GameEquipItem &equip, GameCharacter &character,
                        int slot);
  static bool add_equip_limit(GameEquipItem &equip, GameCharacter &character,
                              int slot);

  static bool add_equip_deco(GameDecoItem &deco, GameCharacter &character,
                             int slot);
  static bool add_equip_deco_limit(GameDecoItem &deco,
                                   GameCharacter &character, int slot);

  static std::vector<GameEquipItem>
  load_equip_slot(const std::u16string &id, GameCharacter &character);

  static std::vector<GameDecoItem> load_deco_slot(const std::u16string &id,
                                                    GameCharacter &character);

  static std::flat_map<IncType, int> load_equip_inc(const std::u16string &id);
  static std::flat_map<IncType, int> load_scroll_inc(const std::u16string &id);

  static std::vector<GameEquipItem> load_equips(const GameCharacter &c);
  static std::vector<GameDecoItem> load_decos(const GameCharacter &c);

  static int use_equip(int i, int slot = 0);
  static int use_deco(int i, int slot = 0);
  static int use_equip_scroll(GameEquipItem &eqp, GameConsumeItem &s);

  static int unuse_equip(int i);
  static int unuse_deco(int i);

  static std::optional<GameEquipItem> *load_equip(int index);
  static std::optional<GameDecoItem> *load_deco(int index);
};