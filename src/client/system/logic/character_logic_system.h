#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_input.h"
#include <cstdint>
#include <flat_set>
#include <string>
class character_logic_system {
private:
  static void run_network_sync(game_character &g_character,
                               game_character &o_character);
  static bool run_action(game_character &g_character,
                         const std::u16string &action);
  static void run_walk_action(game_character &g_character);
  static void run_stand_action(game_character &g_character);
  static void run_climb_action(game_character &g_character);
  static bool run_animate(game_character &g_character);
  static bool run_flip(game_character &g_character);
  static void run_pick(game_character &g_character);
  static bool run_walk(game_character &g_character);
  static bool run_fall(game_character &g_character);
  static bool run_jump(game_character &g_character);
  static bool run_prone(game_character &g_character);
  static bool run_climb(game_character &g_character);
  static bool run_climbing(game_character &g_character);
  static bool run_sit(game_character &g_character);
  static bool run_sitting(game_character &g_character);
  static bool run_attack(game_character &g_character);
  static bool run_portal(game_character &g_character);
  static void run_face(game_character &g_character);
  static void run_state_machine(game_character &g_character);
  enum class pos_type {
    land,
    swim,
    fly,
  };
  static pos_type load_pos_type(game_character &g_character);
  enum class action_enum {
    stand,
    alert,
    walk,
    prone,
    jump,
    climb,
    sit,
    attack,
    skill,
  };
  static action_enum load_action_type(game_character &g_character);

  static void run_others_movement();
  static void run_others_animate();
  static void run_others();

public:
  static bool run();
  static inline std::flat_set<std::string> character_action_input;
  static inline std::flat_set<std::string> character_skill_input;

  static inline int32_t self_fh; // foothold
  static inline int32_t self_lr; // ladderrope

  static inline float self_hforce = 0.0;
  static inline float self_vforce = 0.0;

  static inline float self_hspeed = 0.0;
  static inline float self_vspeed = 0.0;

  static inline float self_hspeed_min = -125.0f;
  static inline float self_hspeed_max = 125.0f;

  static inline float self_vspeed_min = -5000.0f;
  static inline float self_vspeed_max = 670.0f;

  static inline uint64_t self_alert_cooldown;
  static inline uint64_t self_ladderrope_cooldown;
  static inline uint64_t self_foothold_cooldown;
  static inline uint64_t self_sit_cooldown;
  static inline uint64_t self_face_cooldown;
  static inline uint64_t self_portal_cooldown;

  static inline float self_fall_min;
};