#pragma once

#include "src/client/game/game_character.h"
#include "wz/Node.h"
#include <flat_set>
#include <string>
class equip_game_instance {
private:
  static std::u16string load_equip_type(const std::u16string &id);

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
  static weapon_type load_weapon_type(const game_character &g_character);
  static wz::Node *load_equip_info(const std::u16string &id);
  static std::u16string load_equip_name(const std::u16string &id);
  static std::flat_set<job_type> load_equip_job(const std::u16string &id);
};