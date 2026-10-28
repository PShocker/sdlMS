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
    NONE = 0,
    S1A1M1D = 1,
    SPEAR = 2,
    BOW = 3,
    CROSSBOW = 4,
    S2A2M2 = 5,
    WAND = 6,
    CLAW = 7,
    GUN = 9,
  };
  enum class WeaponEnum : int {
    NONE = 0,
    SWORD_1H = 30,
    AXE_1H = 31,
    MACE_1H = 32,
    DAGGER = 33,
    WAND = 37,
    STAFF = 38,
    SWORD_2H = 40,
    AXE_2H = 41,
    MACE_2H = 42,
    SPEAR = 43,
    POLEARM = 44,
    BOW = 45,
    CROSSBOW = 46,
    CLAW = 47,
    KNUCKLE = 48,
    GUN = 49,
  };
  enum class IncType : uint8_t {
    WEAPON_SPEED,
    WEAPON_PAD,
    WEAPON_MAD,
    STR,
    DEX,
    INT,
    LUK,
    PDD,
    MAD,
    MDD,
    ACC,
    EVA,
    CRTR,
    CRTD,
    SPEED,
    JUMP,
    MHP,
    MMP,
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