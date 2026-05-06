#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_input.h"
#include <cstdint>
#include <flat_set>
#include <string>
class character_logic_system {
private:
  enum class pos_type {
    land,
    swim,
    fly,
  };

  static bool run_action(game_character &g_character,
                         const std::u16string &action);
  static void run_walk_action(game_character &g_character);
  static void run_stand_action(game_character &g_character);
  static bool run_animate(game_character &g_character);
  static bool run_flip(game_character &g_character);
  static void run_pick(game_character &g_character);
  static bool run_walk(game_character &g_character);
  static void run_state_machine(game_character &g_character);
  static pos_type load_pos_type(game_character &g_character);

public:
  static bool run();
  static inline std::flat_set<std::string> character_action_input;
  static inline std::flat_set<std::string> character_skill_input;

  static inline float self_hforce = 0.0;
  static inline float self_vforce = 0.0;

  static inline float self_hspeed = 0.0;
  static inline float self_vspeed = 0.0;

  static inline float self_hspeed_min = 0.0;
  static inline float self_vspeed_min = 0.0;

  static inline float self_hspeed_max = 0.0;
  static inline float self_vspeed_max = 0.0;

  static inline uint64_t self_alert_cooldown;
};