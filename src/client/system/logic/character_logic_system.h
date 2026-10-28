#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_input.h"
#include "src/client/game/game_mob.h"
#include "src/client/game/game_reactor.h"
#include "src/client/game/game_triangle.h"
#include "src/common/flatbuffers/client.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>
#include <vector>

using namespace fbs;

struct CheckMobs {
  struct Mobs {
    GameMob mob;
    float x;
    float y;
    std::vector<int64_t> hits;
    uint64_t delay = 0;
  };
  std::vector<Mobs> data;
};

struct CheckReactors {
  struct Reactors {
    GameReactor r;
    float x;
    float y;
  };
  std::vector<Reactors> data;
};

class CharacterLogicSystem {
private:
  static CheckReactors run_reactor_check(GameCharacter &g, SDL_FRect g_r);

  static void run_climb_action(GameCharacter &g_character);
  static void run_face_animate(GameCharacter &g_character);
  static bool run_flip(GameCharacter &g_character);
  static void run_pick(GameCharacter &g_character);
  static bool run_walk(GameCharacter &g_character);
  static bool run_fall(GameCharacter &g_character);
  static bool run_jump(GameCharacter &g_character);
  static bool run_prone(GameCharacter &g_character);
  static bool run_climb(GameCharacter &g_character);
  static bool run_climbing(GameCharacter &g_character);
  static bool run_sit(GameCharacter &g_character);
  static bool run_sitting(GameCharacter &g_character);

  static bool run_attack(GameCharacter &g_character);

  static bool run_skill(GameCharacter &g_character);
  static bool run_portal(GameCharacter &g_character);
  static bool run_face(GameCharacter &g_character);
  static void run_tomb(GameCharacter &g_character);

  static void run_item(GameCharacter &g_character);

  static void run_state_machine();
  static void run_color();
  enum class PosType {
    land,
    swim,
    fly,
  };
  static PosType load_pos_type(GameCharacter &g_character);

  static void run_others_state_machine();
  static void run_others_mv();
  static void run_others();
  static void run_network_sync();
  static void run_network_sync_action();
  static void run_network_sync_pos();
  static void run_network_sync_flip();
  static void run_network_sync_face();
  static void run_network_sync_self();
  static void run_network_sync_hp();
  static void run_network_sync_max_hp();
  static void run_network_sync_state();

  static void run_animate_chair(GameCharacter &g_character);

public:
  static inline ClientCharacterStateT ccs;
  static inline ClientCharacterPickT ccp;
  static inline ClientCharacterT cct;

  static bool run_harvest(GameCharacter &g_character); //or npc talk

  static void run_being_hit(float x, uint64_t num);

  static void run_sit_chair(GameCharacter &g_character,
                            const std::u16string &id);
  static void run_unsit_chair(GameCharacter &g_character);

  static bool run_skill(GameCharacter &g_character, const std::u16string &id);

  static std::vector<uint64_t> run_buff_check(GameCharacter &g_character,
                                              SDL_FRect g_r);
  static CheckMobs run_attack_check(SDL_FPoint pos, SDL_FRect g_r, int num);
  static CheckMobs run_attack_check(GameCharacter &g_character, SDL_FRect g_r,
                                     int num);
  static CheckMobs run_attack_check(GameCharacter &g_character,
                                     SDL_FRect g_r);
  static CheckMobs run_attack_check(GameCharacter &g_character,
                                     GameTriangle tri, int num);
  static CheckMobs run_attack_check(GameCharacter &g_character,
                                     GameTriangle tri);
  static bool run_animate(GameCharacter &g_character);
  static void run_network_die_sync(GameCharacter &g_character);
  static bool run_action(GameCharacter &g_character,
                         const std::u16string &action);
  static bool run_face_action(GameCharacter &g_character,
                              const std::u16string &action);
  static void run_die_action(GameCharacter &g_character);
  static void run_stand_action(GameCharacter &g_character);
  static void run_walk_action(GameCharacter &g_character);
  static bool run_attack_action(GameCharacter &g_character, bool shoot);

  static void load_sfx(GameCharacter &g_character);
  static float load_attack_speed(GameCharacter &g_character);

  static std::u16string load_morph_type(GameCharacter &g_character);
  static SDL_FRect load_rect(GameCharacter &g_character);
  enum class ActionEnum {
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
  static ActionEnum load_action_type(GameCharacter &g_character);
  static bool run();
  static inline std::flat_set<std::string> character_action_input;
  static inline std::flat_set<std::string> character_skill_input;
  static inline std::flat_set<std::string> character_face_input;
  static inline std::flat_set<GameInput> character_item_input;

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
  static inline uint64_t self_two_jump_cooldown;

  static inline uint64_t self_climb_animate_cooldown; // 绳子放技能,动作时间

  static inline float self_fall_min;
  static inline int32_t self_fall_fh;
};