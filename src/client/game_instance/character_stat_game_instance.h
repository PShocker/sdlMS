#pragma once

#include <cstdint>
class character_stat_game_instance {
public:
  static void load();

  static inline float hspeed_max;
  static inline float vspeed_min;

  static inline float hspeed;
  static inline float vspeed;

  static inline uint32_t hp_point;
  static inline uint32_t mp_point;

  static inline uint32_t str_point;
  static inline uint32_t dex_point;
  static inline uint32_t int_point;
  static inline uint32_t luk_point;

  static inline uint32_t min_atk;
  static inline uint32_t max_atk;

  static inline uint32_t weapon_def;

  static inline uint32_t magic;
  static inline uint32_t magic_def;

  static inline uint32_t accuracy;
  static inline uint32_t avoid;

  static inline uint32_t crit_rate;
  static inline uint32_t crit_damage;

  static inline uint32_t speed;
  static inline uint32_t jump;
};