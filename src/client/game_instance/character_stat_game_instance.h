#pragma once

#include "src/client/game/game_save.h"
#include <cstdint>
class character_stat_game_instance {
private:
  static int load_primary_stat();
  static int load_secondary_stat();

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

  static inline uint32_t accuracy; // 命中
  static inline uint32_t avoid;    // 回避

  static inline uint32_t crit_rate;
  static inline uint32_t crit_damage;

  static inline uint32_t hp_ap;
  static inline uint32_t mp_ap;
  static inline uint32_t str_ap;
  static inline uint32_t dex_ap;
  static inline uint32_t int_ap;
  static inline uint32_t luk_ap;

  static inline uint32_t remain_ap = 100;

  static inline int64_t ski_hp;
  static inline int64_t ski_mp;
  static inline int64_t ski_str;
  static inline int64_t ski_dex;
  static inline int64_t ski_int;
  static inline int64_t ski_luk;
  static inline int64_t ski_pro; // 熟练度

  static inline uint32_t ski_accuracy; // 命中
  static inline uint32_t ski_avoid;    // 回避

  static inline int64_t eqp_hp;
  static inline int64_t eqp_mp;
  static inline int64_t eqp_str;
  static inline int64_t eqp_dex;
  static inline int64_t eqp_int;
  static inline int64_t eqp_luk;
  static inline int64_t eqp_pro; // 熟练度

  static inline uint32_t eqp_accuracy; // 命中
  static inline uint32_t eqp_avoid;    // 回避

  static inline uint32_t eqp_attack;
  static inline uint32_t eqp_magic;

  static inline uint32_t eqp_attack_def;
  static inline uint32_t eqp_magic_def;

  static inline int64_t itm_hp;
  static inline int64_t itm_mp;
  static inline int64_t itm_str;
  static inline int64_t itm_dex;
  static inline int64_t itm_int;
  static inline int64_t itm_luk;
  static inline int64_t itm_pro; // 熟练度

  static inline uint32_t itm_accuracy; // 命中
  static inline uint32_t itm_avoid;    // 回避

  static inline uint32_t itm_attack;
  static inline uint32_t itm_magic;
};