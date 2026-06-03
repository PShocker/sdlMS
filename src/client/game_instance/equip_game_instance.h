#pragma once

#include "src/client/game/game_character.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>
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
  enum class job_type : uint8_t {
    BEGINNER,
    WARRIOR,
    MAGICIAN,
    BOWMAN,
    THIEF,
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
  static weapon_type load_weapon_type(const game_character &g_character);
  static wz::Node *load_equip_info(const std::u16string &id);
  static std::u16string load_equip_name(const std::u16string &id);
  static std::flat_set<job_type> load_equip_job(const std::u16string &id);
  static std::u16string load_equip_type(const std::u16string &id);
  static uint8_t load_equip_tuc(const std::u16string &id);

  static std::flat_map<inc_type, int> load_equip_inc(const std::u16string &id);
};