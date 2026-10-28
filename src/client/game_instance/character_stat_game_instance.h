#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_CHARACTER_STAT_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_CHARACTER_STAT_GAME_INSTANCE_H_

#include "src/client/game/game_save.h"

#include <cstdint>
namespace sdlms {


class CharacterStatGameInstance {
 private:
  // ============ 私有静态方法 ============
  static uint32_t GetBaseHpMax();
  static uint32_t GetBaseMpMax();

  static int LoadPrimaryStat();
  static int LoadSecondaryStat();

  static bool Upgrade();

  // ============ 基础战斗资源 ============
  static inline int32_t hp_point_ = INT32_MAX;
  static inline int32_t hp_point_max_ = INT32_MAX;
  static inline int32_t mp_point_ = INT32_MAX;
  static inline int32_t mp_point_max_ = INT32_MAX;
  static inline uint64_t exp_point_ = 0;
  static inline uint64_t exp_point_max_ = UINT64_MAX;

  // ============ 主属性 ============
  static inline uint32_t str_point_ = 0;
  static inline uint32_t dex_point_ = 0;
  static inline uint32_t int_point_ = 0;
  static inline uint32_t luk_point_ = 0;

  // ============ 攻击相关 ============
  static inline uint32_t min_atk_ = 0;
  static inline uint32_t max_atk_ = 0;
  static inline uint32_t crit_rate_ = 0;
  static inline uint32_t crit_damage_ = 0;

  // ============ 命中 / 回避 ============
  static inline uint32_t accuracy_ = 0;  // 命中
  static inline uint32_t avoid_ = 0;     // 回避

  // ============ 防御 ============
  static inline int32_t attack_def_ = 0;
  static inline int32_t magic_def_ = 0;

  // ============ 攻击 / 魔法 ============
  static inline int32_t attack_ = 0;
  static inline int32_t magic_ = 0;

  // ============ AP 加点 ============
  static inline uint32_t hp_ap_ = 0;
  static inline uint32_t mp_ap_ = 0;
  static inline uint32_t str_ap_ = 0;
  static inline uint32_t dex_ap_ = 0;
  static inline uint32_t int_ap_ = 0;
  static inline uint32_t luk_ap_ = 0;

  // ============ 技能加成 (ski_) ============
  static inline int64_t ski_hp_ = 0;
  static inline int64_t ski_mp_ = 0;
  static inline int64_t ski_str_ = 0;
  static inline int64_t ski_dex_ = 0;
  static inline int64_t ski_int_ = 0;
  static inline int64_t ski_luk_ = 0;
  static inline int64_t ski_pro_ = 0;  // 熟练度

  static inline uint32_t ski_accuracy_ = 0;  // 命中
  static inline uint32_t ski_avoid_ = 0;     // 回避
  static inline uint32_t ski_exp_ = 0;

  static inline int32_t ski_attack_def_ = 0;
  static inline int32_t ski_magic_def_ = 0;

  // ============ 装备加成 (eqp_) ============
  static inline int64_t eqp_hp_ = 0;
  static inline int64_t eqp_mp_ = 0;
  static inline int64_t eqp_str_ = 0;
  static inline int64_t eqp_dex_ = 0;
  static inline int64_t eqp_int_ = 0;
  static inline int64_t eqp_luk_ = 0;
  static inline int64_t eqp_pro_ = 0;  // 熟练度

  static inline uint32_t eqp_accuracy_ = 0;  // 命中
  static inline uint32_t eqp_avoid_ = 0;     // 回避

  static inline uint32_t eqp_attack_ = 0;  // pad
  static inline uint32_t eqp_magic_ = 0;

  static inline int32_t eqp_attack_def_ = 0;
  static inline int32_t eqp_magic_def_ = 0;

  static inline uint32_t eqp_exp_ = 0;

  // ============ 道具加成 (itm_) ============
  static inline int64_t itm_hp_ = 0;
  static inline int64_t itm_mp_ = 0;
  static inline int64_t itm_str_ = 0;
  static inline int64_t itm_dex_ = 0;
  static inline int64_t itm_int_ = 0;
  static inline int64_t itm_luk_ = 0;
  static inline int64_t itm_pro_ = 0;  // 熟练度

  static inline uint32_t itm_accuracy_ = 0;  // 命中
  static inline uint32_t itm_avoid_ = 0;     // 回避

  static inline uint32_t itm_attack_ = 0;  // pad
  static inline uint32_t itm_magic_ = 0;
  static inline uint32_t itm_exp_ = 0;

  static inline int32_t itm_attack_def_ = 0;
  static inline int32_t itm_magic_def_ = 0;

  // ============ 剩余 AP ============
  static inline uint32_t remain_ap_ = 100;

  static void UpdateSfSkillStat();
  static void UpdateSfEquipStat();

 public:
  // ============================================================
  // 经验 / 更新 / 加载
  // ============================================================
  static void AddExp(uint32_t num);
  static void Update();
  static void Load(const CharacterSaveData& cs);

  // ============================================================
  // HP / MP
  // ============================================================
  static void SetHp(int32_t n);
  static void SetMp(int32_t n);

  static void SetCritRate(uint32_t n);
  static void SetCritDamage(uint32_t n);

  static int32_t GetHp();
  static int32_t GetMp();
  static int32_t GetHpMax();
  static int32_t GetMpMax();
  static uint64_t GetExp();
  static uint64_t GetExpMax();

  // ============================================================
  // 剩余 AP
  // ============================================================
  static uint32_t GetRemainAp();

  // ============================================================
  // AP 加点 setter / getter
  // ============================================================
  static void SetStrAp(uint32_t n);
  static void SetDexAp(uint32_t n);
  static void SetIntAp(uint32_t n);
  static void SetLukAp(uint32_t n);
  static void SetHpAp(uint32_t n);
  static void SetMpAp(uint32_t n);

  static void SetRemainAp(uint32_t n);

  static uint32_t GetStrAp();
  static uint32_t GetDexAp();
  static uint32_t GetIntAp();
  static uint32_t GetLukAp();
  static uint32_t GetHpAp();
  static uint32_t GetMpAp();

  // ============================================================
  // 主属性 getter
  // ============================================================
  static uint32_t GetStrPoint();
  static uint32_t GetDexPoint();
  static uint32_t GetIntPoint();
  static uint32_t GetLukPoint();

  // ============================================================
  // 最终战斗属性 getter
  // ============================================================
  static uint32_t GetMinAtk();
  static uint32_t GetMaxAtk();
  static uint32_t GetCritRate();
  static uint32_t GetCritDamage();
  static uint32_t GetAcc();
  static uint32_t GetAvd();
  static int32_t GetDef();
  static int32_t GetMdef();
  static int32_t GetAttack();
  static int32_t GetMagic();

  // ============================================================
  // 技能加成 setter (ski_)
  // ============================================================
  static void SetSkiHp(int64_t n);
  static void SetSkiMp(int64_t n);
  static void SetSkiStr(int64_t n);
  static void SetSkiDex(int64_t n);
  static void SetSkiInt(int64_t n);
  static void SetSkiLuk(int64_t n);
  static void SetSkiPro(int64_t n);
  static void SetSkiAcc(uint32_t n);
  static void SetSkiAvd(uint32_t n);
  static void SetSkiDef(int32_t n);
  static void SetSkiMdef(int32_t n);
  static void SetSkiExp(uint32_t n);

  // ============================================================
  // 技能加成 getter (ski_)
  // ============================================================
  static int64_t GetSkiHp();
  static int64_t GetSkiMp();
  static int64_t GetSkiStr();
  static int64_t GetSkiDex();
  static int64_t GetSkiInt();
  static int64_t GetSkiLuk();
  static int64_t GetSkiPro();
  static uint32_t GetSkiAcc();
  static uint32_t GetSkiAvd();
  static int32_t GetSkiDef();
  static int32_t GetSkiMdef();
  static uint32_t GetSkiExp();

  // ============================================================
  // 装备加成 setter (eqp_)
  // ============================================================
  static void SetEqpHp(int64_t n);
  static void SetEqpMp(int64_t n);
  static void SetEqpStr(int64_t n);
  static void SetEqpDex(int64_t n);
  static void SetEqpInt(int64_t n);
  static void SetEqpLuk(int64_t n);
  static void SetEqpPro(int64_t n);
  static void SetEqpAcc(uint32_t n);
  static void SetEqpAvd(uint32_t n);
  static void SetEqpAttack(uint32_t n);
  static void SetEqpMagic(uint32_t n);
  static void SetEqpDef(int32_t n);
  static void SetEqpMdef(int32_t n);
  static void SetEqpExp(uint32_t n);

  // ============================================================
  // 装备加成 getter (eqp_)
  // ============================================================
  static int64_t GetEqpHp();
  static int64_t GetEqpMp();
  static int64_t GetEqpStr();
  static int64_t GetEqpDex();
  static int64_t GetEqpInt();
  static int64_t GetEqpLuk();
  static int64_t GetEqpPro();
  static uint32_t GetEqpAcc();
  static uint32_t GetEqpAvd();
  static int32_t GetEqpDef();
  static int32_t GetEqpMdef();
  static uint32_t GetEqpExp();

  // ============================================================
  // 道具加成 setter (itm_)
  // ============================================================
  static void SetItmHp(int64_t n);
  static void SetItmMp(int64_t n);
  static void SetItmStr(int64_t n);
  static void SetItmDex(int64_t n);
  static void SetItmInt(int64_t n);
  static void SetItmLuk(int64_t n);
  static void SetItmPro(int64_t n);
  static void SetItmAcc(uint32_t n);
  static void SetItmAvd(uint32_t n);
  static void SetItmAttack(uint32_t n);
  static void SetItmMagic(uint32_t n);
  static void SetItmDef(int32_t n);
  static void SetItmMdef(int32_t n);
  static void SetItmExp(uint32_t n);

  // ============================================================
  // 道具加成 getter (itm_)
  // ============================================================
  static int64_t GetItmHp();
  static int64_t GetItmMp();
  static int64_t GetItmStr();
  static int64_t GetItmDex();
  static int64_t GetItmInt();
  static int64_t GetItmLuk();
  static int64_t GetItmPro();
  static uint32_t GetItmAcc();
  static uint32_t GetItmAvd();
  static uint32_t GetItmAttack();
  static uint32_t GetItmMagic();
  static uint32_t GetItmExp();
  static int32_t GetItmDef();
  static int32_t GetItmMdef();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_CHARACTER_STAT_GAME_INSTANCE_H_
