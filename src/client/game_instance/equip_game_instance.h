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
class equip_game_instance {
public:
  enum class weapon_type : uint8_t {
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
  enum class weapon_enum : int {
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
  enum class inc_type : uint8_t {
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

  static weapon_type load_weapon_type(const game_character &g_character);
  static wz::Node *load_equip_info(const std::u16string &id);
  static std::u16string load_equip_name(const std::u16string &id);
  static std::u16string load_equip_desc(const std::u16string &id);
  static std::flat_set<job_type> load_equip_job(const std::u16string &id);
  static std::u16string load_equip_type(const std::u16string &id);
  static uint8_t load_equip_tuc(const std::u16string &id);

  static void add_equip(game_equip_item &equip, game_character &character,
                        int slot);
  static bool add_equip_limit(game_equip_item &equip, game_character &character,
                              int slot);

  static void add_equip_deco(game_deco_item &deco, game_character &character,
                             int slot);
  static bool add_equip_deco_limit(game_deco_item &deco,
                                   game_character &character, int slot);

  static std::vector<game_equip_item>
  load_equip_slot(game_equip_item &equip, game_character &character);

  static std::flat_map<inc_type, int> load_equip_inc(const std::u16string &id);
  static std::flat_map<inc_type, int> load_scroll_inc(const std::u16string &id);
};