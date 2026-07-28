#pragma once

#include "src/client/game/game_save.h"
#include <cstdint>
class character_stat_game_instance {
public:
  static void update();
  static void load(const character_save &cs);

  static inline int32_t hp_point = INT32_MAX;
  static inline int32_t hp_point_max = INT32_MAX;
  static inline int32_t mp_point = INT32_MAX;
  static inline int32_t mp_point_max = INT32_MAX;
  static inline uint32_t exp_point;
  static inline uint32_t exp_point_max = UINT32_MAX;

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

  static inline uint32_t hp_ap;
  static inline uint32_t mp_ap;
  static inline uint32_t str_ap;
  static inline uint32_t dex_ap;
  static inline uint32_t int_ap;
  static inline uint32_t luk_ap;

  static inline uint32_t remain_ap = 100;
};