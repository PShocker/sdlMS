#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_input.h"
#include <cstdint>
#include <flat_set>
#include <string>
class character_logic_system {
public:
  static bool run();
  static inline std::flat_set<std::string> character_action_input;
  static inline std::flat_set<std::string> character_skill_input;

  static inline float self_hforce = 0.0;
  static inline float self_vforce = 0.0;
  static inline uint64_t self_alert_cooldown;
};