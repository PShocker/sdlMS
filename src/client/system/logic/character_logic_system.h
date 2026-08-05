#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_input.h"
#include "src/client/game/game_mob.h"
#include "src/client/game/game_triangle.h"
#include "src/common/flatbuffers/client.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>
#include <vector>

using namespace fbs;

struct check_mobs {
  struct mobs {
    game_mob mob;
    float x;
    float y;
    std::vector<int64_t> hits;
  };
  std::vector<mobs> data;
};

class character_logic_system {
private:
  static check_mobs run_shoot_check(game_character &g_character);

  static void run_climb_action(game_character &g_character);
  static void run_face_animate(game_character &g_character);
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

  static bool run_skill(game_character &g_character);
  static bool run_portal(game_character &g_character);
  static bool run_face(game_character &g_character);
  static void run_tomb(game_character &g_character);

  static void run_item(game_character &g_character);

  static void run_state_machine();
  static void run_color();
  enum class pos_type {
    land,
    swim,
    fly,
  };
  static pos_type load_pos_type(game_character &g_character);

  static void run_others_state_machine();
  static void run_others_mv();
  static void run_others();
  static void run_network_sync();
  static void run_network_sync_state();

  static void run_animate_chair(game_character &g_character);

public:
  static inline ClientCharacterStateT ccs;
  static inline ClientCharacterPickT ccp;

  static void run_chair(game_character &g_character, const std::u16string &c);
  static bool run_skill(game_character &g_character, const std::u16string &id);

  static check_mobs run_attack_check(game_character &g_character,
                                     SDL_FRect g_r);
  static check_mobs run_attack_check(game_character &g_character,
                                     game_triangle tri);
  static bool run_animate(game_character &g_character);
  static void run_network_die_sync(game_character &g_character);
  static bool run_action(game_character &g_character,
                         const std::u16string &action);
  static bool run_face_action(game_character &g_character,
                              const std::u16string &action);
  static void run_die_action(game_character &g_character);
  static void run_stand_action(game_character &g_character);
  static void run_walk_action(game_character &g_character);
  static bool run_attack_action(game_character &g_character);

  static void load_sfx(game_character &g_character);
  static float load_attack_speed(game_character &g_character);

  static SDL_FRect load_rect(game_character &g_character);
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
    dead,
  };
  static action_enum load_action_type(game_character &g_character);
  static bool run();
  static inline std::flat_set<std::string> character_action_input;
  static inline std::flat_set<std::string> character_skill_input;
  static inline std::flat_set<std::string> character_face_input;
  static inline std::flat_set<game_input> character_item_input;

  static inline int32_t self_fh; // foothold
  static inline int32_t self_lr; // ladderrope

  static inline float self_hforce = 0.0;
  static inline float self_vforce = 0.0;

  static inline float self_hspeed = 0.0;
  static inline float self_vspeed = 0.0;

  static inline float self_hspeed_min = -125.0f;
  static inline float self_hspeed_max = 125.0f;

  static inline float self_vspeed_min = -5550.0f;
  static inline float self_vspeed_max = 670.0f;

  static inline uint64_t self_alert_cooldown;
  static inline uint64_t self_ladderrope_cooldown;
  static inline uint64_t self_foothold_cooldown;
  static inline uint64_t self_sit_cooldown;
  static inline uint64_t self_face_cooldown;
  static inline uint64_t self_portal_cooldown;
  static inline uint64_t self_flip_cooldown;
  static inline uint64_t self_attack_cooldown;
  static inline uint64_t self_invincible_cooldown;

  static inline float self_fall_min;
};