#pragma once

#include "src/client/game/game_save.h"
#include <cstdint>
class character_stat_game_instance {
private:
  static int load_primary_stat();
  static int load_secondary_stat();
  static bool upgrade();

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

  static inline int32_t attack_def;
  static inline int32_t magic_def;

  static inline int32_t attack;
  static inline int32_t magic;

  static inline uint32_t hp_ap;
  static inline uint32_t mp_ap;
  static inline uint32_t str_ap;
  static inline uint32_t dex_ap;
  static inline uint32_t int_ap;
  static inline uint32_t luk_ap;

  static inline int64_t ski_hp;
  static inline int64_t ski_mp;
  static inline int64_t ski_str;
  static inline int64_t ski_dex;
  static inline int64_t ski_int;
  static inline int64_t ski_luk;
  static inline int64_t ski_pro; // 熟练度

  static inline uint32_t ski_accuracy; // 命中
  static inline uint32_t ski_avoid;    // 回避
  static inline uint32_t ski_exp;

  static inline int32_t ski_attack_def;
  static inline int32_t ski_magic_def;

  static inline int64_t eqp_hp;
  static inline int64_t eqp_mp;
  static inline int64_t eqp_str;
  static inline int64_t eqp_dex;
  static inline int64_t eqp_int;
  static inline int64_t eqp_luk;
  static inline int64_t eqp_pro; // 熟练度

  static inline uint32_t eqp_accuracy; // 命中
  static inline uint32_t eqp_avoid;    // 回避

  static inline uint32_t eqp_attack; // pad
  static inline uint32_t eqp_magic;

  static inline int32_t eqp_attack_def;
  static inline int32_t eqp_magic_def;

  static inline uint32_t eqp_exp;

  static inline int64_t itm_hp;
  static inline int64_t itm_mp;
  static inline int64_t itm_str;
  static inline int64_t itm_dex;
  static inline int64_t itm_int;
  static inline int64_t itm_luk;
  static inline int64_t itm_pro; // 熟练度

  static inline uint32_t itm_accuracy; // 命中
  static inline uint32_t itm_avoid;    // 回避

  static inline uint32_t itm_attack; // pad
  static inline uint32_t itm_magic;
  static inline uint32_t itm_exp;

  static inline int32_t itm_attack_def;
  static inline int32_t itm_magic_def;

  static inline uint32_t remain_ap = 100;

public:
  static void add_exp(uint32_t num);

  static void update();
  static void load(const character_save &cs);

  static void set_hp(int n);
  static void set_mp(int n);

  static int32_t get_hp();
  static int32_t get_mp();
  static int32_t get_hp_max();
  static int32_t get_mp_max();
  static uint32_t get_exp();
  static uint32_t get_exp_max();

  static uint32_t get_remain_ap();

  static void set_str_ap(int n);
  static void set_dex_ap(int n);
  static void set_int_ap(int n);
  static void set_luk_ap(int n);
  static void set_hp_ap(int n);
  static void set_mp_ap(int n);

  static uint32_t get_str_ap();
  static uint32_t get_dex_ap();
  static uint32_t get_int_ap();
  static uint32_t get_luk_ap();
  static uint32_t get_hp_ap();
  static uint32_t get_mp_ap();

  static void set_ski_hp(int n);
  static void set_ski_mp(int n);
  static void set_ski_str(int n);
  static void set_ski_dex(int n);
  static void set_ski_int(int n);
  static void set_ski_luk(int n);
  static void set_ski_pro(int n);
  static void set_ski_acc(int n);
  static void set_ski_avd(int n);
  static void set_ski_def(int n);
  static void set_ski_mdef(int n);
  static void set_ski_exp(int n);

  static void set_eqp_hp(int n);
  static void set_eqp_mp(int n);
  static void set_eqp_str(int n);
  static void set_eqp_dex(int n);
  static void set_eqp_int(int n);
  static void set_eqp_luk(int n);
  static void set_eqp_pro(int n);
  static void set_eqp_acc(int n);
  static void set_eqp_avd(int n);
  static void set_eqp_def(int n);
  static void set_eqp_mdef(int n);
  static void set_eqp_exp(int n);

  static void set_itm_hp(int n);
  static void set_itm_mp(int n);
  static void set_itm_str(int n);
  static void set_itm_dex(int n);
  static void set_itm_int(int n);
  static void set_itm_luk(int n);
  static void set_itm_pro(int n);
  static void set_itm_acc(int n);
  static void set_itm_avd(int n);
  static void set_itm_def(int n);
  static void set_itm_mdef(int n);
  static void set_itm_exp(int n);

  static uint32_t get_itm_hp();
  static uint32_t get_itm_mp();
  static uint32_t get_itm_str();
  static uint32_t get_itm_dex();
  static uint32_t get_itm_int();
  static uint32_t get_itm_luk();
  static uint32_t get_itm_pro();
  static uint32_t get_itm_acc();
  static uint32_t get_itm_avd();

  static uint32_t get_min_atk();
  static uint32_t get_max_atk();
  static int32_t get_def();
  static int32_t get_mdef();
  static uint32_t get_acc();
  static uint32_t get_avd();

  static int32_t get_attack();
  static int32_t get_magic();
};