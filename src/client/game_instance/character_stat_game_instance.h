#pragma once

#include "src/client/game/game_save.h"
#include <cstdint>

class character_stat_game_instance {
private:
  // ============ 私有静态方法 ============
  static int load_primary_stat();
  static int load_secondary_stat();
  static bool upgrade();

  // ============ 基础战斗资源 ============
  static inline int32_t hp_point = INT32_MAX;
  static inline int32_t hp_point_max = INT32_MAX;
  static inline int32_t mp_point = INT32_MAX;
  static inline int32_t mp_point_max = INT32_MAX;
  static inline uint32_t exp_point = 0;
  static inline uint32_t exp_point_max = UINT32_MAX;

  // ============ 主属性 ============
  static inline uint32_t str_point = 0;
  static inline uint32_t dex_point = 0;
  static inline uint32_t int_point = 0;
  static inline uint32_t luk_point = 0;

  // ============ 攻击相关 ============
  static inline uint32_t min_atk = 0;
  static inline uint32_t max_atk = 0;
  static inline uint32_t crit_rate = 0;
  static inline uint32_t crit_damage = 0;

  // ============ 命中 / 回避 ============
  static inline uint32_t accuracy = 0; // 命中
  static inline uint32_t avoid = 0;    // 回避

  // ============ 防御 ============
  static inline int32_t attack_def = 0;
  static inline int32_t magic_def = 0;

  // ============ 攻击 / 魔法 ============
  static inline int32_t attack = 0;
  static inline int32_t magic = 0;

  // ============ AP 加点 ============
  static inline uint32_t hp_ap = 0;
  static inline uint32_t mp_ap = 0;
  static inline uint32_t str_ap = 0;
  static inline uint32_t dex_ap = 0;
  static inline uint32_t int_ap = 0;
  static inline uint32_t luk_ap = 0;

  // ============ 技能加成 (ski_) ============
  static inline int64_t ski_hp = 0;
  static inline int64_t ski_mp = 0;
  static inline int64_t ski_str = 0;
  static inline int64_t ski_dex = 0;
  static inline int64_t ski_int = 0;
  static inline int64_t ski_luk = 0;
  static inline int64_t ski_pro = 0; // 熟练度

  static inline uint32_t ski_accuracy = 0; // 命中
  static inline uint32_t ski_avoid = 0;    // 回避
  static inline uint32_t ski_exp = 0;

  static inline int32_t ski_attack_def = 0;
  static inline int32_t ski_magic_def = 0;

  // ============ 装备加成 (eqp_) ============
  static inline int64_t eqp_hp = 0;
  static inline int64_t eqp_mp = 0;
  static inline int64_t eqp_str = 0;
  static inline int64_t eqp_dex = 0;
  static inline int64_t eqp_int = 0;
  static inline int64_t eqp_luk = 0;
  static inline int64_t eqp_pro = 0; // 熟练度

  static inline uint32_t eqp_accuracy = 0; // 命中
  static inline uint32_t eqp_avoid = 0;    // 回避

  static inline uint32_t eqp_attack = 0; // pad
  static inline uint32_t eqp_magic = 0;

  static inline int32_t eqp_attack_def = 0;
  static inline int32_t eqp_magic_def = 0;

  static inline uint32_t eqp_exp = 0;

  // ============ 道具加成 (itm_) ============
  static inline int64_t itm_hp = 0;
  static inline int64_t itm_mp = 0;
  static inline int64_t itm_str = 0;
  static inline int64_t itm_dex = 0;
  static inline int64_t itm_int = 0;
  static inline int64_t itm_luk = 0;
  static inline int64_t itm_pro = 0; // 熟练度

  static inline uint32_t itm_accuracy = 0; // 命中
  static inline uint32_t itm_avoid = 0;    // 回避

  static inline uint32_t itm_attack = 0; // pad
  static inline uint32_t itm_magic = 0;
  static inline uint32_t itm_exp = 0;

  static inline int32_t itm_attack_def = 0;
  static inline int32_t itm_magic_def = 0;

  // ============ 剩余 AP ============
  static inline uint32_t remain_ap = 100;

public:
  // ============================================================
  // 经验 / 更新 / 加载
  // ============================================================
  static void add_exp(uint32_t num);
  static void update();
  static void load(const character_save &cs);

  // ============================================================
  // HP / MP
  // ============================================================
  static void set_hp(int32_t n);
  static void set_mp(int32_t n);

  static void set_crit_rate(uint32_t n);
  static void set_crit_damage(uint32_t n);

  static int32_t get_hp();
  static int32_t get_mp();
  static int32_t get_hp_max();
  static int32_t get_mp_max();
  static uint32_t get_exp();
  static uint32_t get_exp_max();

  // ============================================================
  // 剩余 AP
  // ============================================================
  static uint32_t get_remain_ap();

  // ============================================================
  // AP 加点 setter / getter
  // ============================================================
  static void set_str_ap(uint32_t n);
  static void set_dex_ap(uint32_t n);
  static void set_int_ap(uint32_t n);
  static void set_luk_ap(uint32_t n);
  static void set_hp_ap(uint32_t n);
  static void set_mp_ap(uint32_t n);

  static void set_remain_ap(uint32_t n);

  static uint32_t get_str_ap();
  static uint32_t get_dex_ap();
  static uint32_t get_int_ap();
  static uint32_t get_luk_ap();
  static uint32_t get_hp_ap();
  static uint32_t get_mp_ap();

  // ============================================================
  // 主属性 getter
  // ============================================================
  static uint32_t get_str_point();
  static uint32_t get_dex_point();
  static uint32_t get_int_point();
  static uint32_t get_luk_point();

  // ============================================================
  // 最终战斗属性 getter
  // ============================================================
  static uint32_t get_min_atk();
  static uint32_t get_max_atk();
  static uint32_t get_crit_rate();
  static uint32_t get_crit_damage();
  static uint32_t get_acc();
  static uint32_t get_avd();
  static int32_t get_def();
  static int32_t get_mdef();
  static int32_t get_attack();
  static int32_t get_magic();

  // ============================================================
  // 技能加成 setter (ski_)
  // ============================================================
  static void set_ski_hp(int64_t n);
  static void set_ski_mp(int64_t n);
  static void set_ski_str(int64_t n);
  static void set_ski_dex(int64_t n);
  static void set_ski_int(int64_t n);
  static void set_ski_luk(int64_t n);
  static void set_ski_pro(int64_t n);
  static void set_ski_acc(uint32_t n);
  static void set_ski_avd(uint32_t n);
  static void set_ski_def(int32_t n);
  static void set_ski_mdef(int32_t n);
  static void set_ski_exp(uint32_t n);

  // ============================================================
  // 技能加成 getter (ski_)
  // ============================================================
  static int64_t get_ski_hp();
  static int64_t get_ski_mp();
  static int64_t get_ski_str();
  static int64_t get_ski_dex();
  static int64_t get_ski_int();
  static int64_t get_ski_luk();
  static int64_t get_ski_pro();
  static uint32_t get_ski_acc();
  static uint32_t get_ski_avd();
  static int32_t get_ski_def();
  static int32_t get_ski_mdef();
  static uint32_t get_ski_exp();

  // ============================================================
  // 装备加成 setter (eqp_)
  // ============================================================
  static void set_eqp_hp(int64_t n);
  static void set_eqp_mp(int64_t n);
  static void set_eqp_str(int64_t n);
  static void set_eqp_dex(int64_t n);
  static void set_eqp_int(int64_t n);
  static void set_eqp_luk(int64_t n);
  static void set_eqp_pro(int64_t n);
  static void set_eqp_acc(uint32_t n);
  static void set_eqp_avd(uint32_t n);
  static void set_eqp_attack(uint32_t n);
  static void set_eqp_magic(uint32_t n);
  static void set_eqp_def(int32_t n);
  static void set_eqp_mdef(int32_t n);
  static void set_eqp_exp(uint32_t n);

  // ============================================================
  // 装备加成 getter (eqp_)
  // ============================================================
  static int64_t get_eqp_hp();
  static int64_t get_eqp_mp();
  static int64_t get_eqp_str();
  static int64_t get_eqp_dex();
  static int64_t get_eqp_int();
  static int64_t get_eqp_luk();
  static int64_t get_eqp_pro();
  static uint32_t get_eqp_acc();
  static uint32_t get_eqp_avd();
  static int32_t get_eqp_def();
  static int32_t get_eqp_mdef();
  static uint32_t get_eqp_exp();

  // ============================================================
  // 道具加成 setter (itm_)
  // ============================================================
  static void set_itm_hp(int64_t n);
  static void set_itm_mp(int64_t n);
  static void set_itm_str(int64_t n);
  static void set_itm_dex(int64_t n);
  static void set_itm_int(int64_t n);
  static void set_itm_luk(int64_t n);
  static void set_itm_pro(int64_t n);
  static void set_itm_acc(uint32_t n);
  static void set_itm_avd(uint32_t n);
  static void set_itm_attack(uint32_t n);
  static void set_itm_magic(uint32_t n);
  static void set_itm_def(int32_t n);
  static void set_itm_mdef(int32_t n);
  static void set_itm_exp(uint32_t n);

  // ============================================================
  // 道具加成 getter (itm_)
  // ============================================================
  static int64_t get_itm_hp();
  static int64_t get_itm_mp();
  static int64_t get_itm_str();
  static int64_t get_itm_dex();
  static int64_t get_itm_int();
  static int64_t get_itm_luk();
  static int64_t get_itm_pro();
  static uint32_t get_itm_acc();
  static uint32_t get_itm_avd();
  static uint32_t get_itm_attack();
  static uint32_t get_itm_magic();
  static uint32_t get_itm_exp();
  static int32_t get_itm_def();
  static int32_t get_itm_mdef();
};