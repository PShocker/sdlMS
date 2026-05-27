#pragma once

#include <cstdint>
class character_stat_game_instance {
public:
  static void load();

  static inline uint32_t level;

  static inline int32_t hp_point;
  static inline int32_t hp_point_max;
  static inline int32_t mp_point;
  static inline int32_t mp_point_max;
  static inline uint32_t exp_point;
  static inline uint32_t exp_point_max;

  static inline uint32_t fame;

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