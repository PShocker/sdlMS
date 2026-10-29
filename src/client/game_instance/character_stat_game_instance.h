#pragma once

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

  // ============ 装备加成 (equip_) ============
  static inline int64_t equip_hp_ = 0;
  static inline int64_t equip_mp_ = 0;
  static inline int64_t equip_str_ = 0;
  static inline int64_t equip_dex_ = 0;
  static inline int64_t equip_int_ = 0;
  static inline int64_t equip_luk_ = 0;
  static inline int64_t equip_pro_ = 0;  // 熟练度

  static inline uint32_t equip_accuracy_ = 0;  // 命中
  static inline uint32_t equip_avoid_ = 0;     // 回避

  static inline uint32_t equip_attack_ = 0;  // pad
  static inline uint32_t equip_magic_ = 0;

  static inline int32_t equip_attack_def_ = 0;
  static inline int32_t equip_magic_def_ = 0;

  static inline uint32_t equip_exp_ = 0;

  // ============ 道具加成 (item_) ============
  static inline int64_t item_hp_ = 0;
  static inline int64_t item_mp_ = 0;
  static inline int64_t item_str_ = 0;
  static inline int64_t item_dex_ = 0;
  static inline int64_t item_int_ = 0;
  static inline int64_t item_luk_ = 0;
  static inline int64_t item_pro_ = 0;  // 熟练度

  static inline uint32_t item_accuracy_ = 0;  // 命中
  static inline uint32_t item_avoid_ = 0;     // 回避

  static inline uint32_t item_attack_ = 0;  // pad
  static inline uint32_t item_magic_ = 0;
  static inline uint32_t item_exp_ = 0;

  static inline int32_t item_attack_def_ = 0;
  static inline int32_t item_magic_def_ = 0;

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
  // 装备加成 setter (equip_)
  // ============================================================
  static void SetEquipHp(int64_t n);
  static void SetEquipMp(int64_t n);
  static void SetEquipStr(int64_t n);
  static void SetEquipDex(int64_t n);
  static void SetEquipInt(int64_t n);
  static void SetEquipLuk(int64_t n);
  static void SetEquipPro(int64_t n);
  static void SetEquipAcc(uint32_t n);
  static void SetEquipAvd(uint32_t n);
  static void SetEquipAttack(uint32_t n);
  static void SetEquipMagic(uint32_t n);
  static void SetEquipDef(int32_t n);
  static void SetEquipMdef(int32_t n);
  static void SetEquipExp(uint32_t n);

  // ============================================================
  // 装备加成 getter (equip_)
  // ============================================================
  static int64_t GetEquipHp();
  static int64_t GetEquipMp();
  static int64_t GetEquipStr();
  static int64_t GetEquipDex();
  static int64_t GetEquipInt();
  static int64_t GetEquipLuk();
  static int64_t GetEquipPro();
  static uint32_t GetEquipAcc();
  static uint32_t GetEquipAvd();
  static int32_t GetEquipDef();
  static int32_t GetEquipMdef();
  static uint32_t GetEquipExp();

  // ============================================================
  // 道具加成 setter (item_)
  // ============================================================
  static void SetItemHp(int64_t n);
  static void SetItemMp(int64_t n);
  static void SetItemStr(int64_t n);
  static void SetItemDex(int64_t n);
  static void SetItemInt(int64_t n);
  static void SetItemLuk(int64_t n);
  static void SetItemPro(int64_t n);
  static void SetItemAcc(uint32_t n);
  static void SetItemAvd(uint32_t n);
  static void SetItemAttack(uint32_t n);
  static void SetItemMagic(uint32_t n);
  static void SetItemDef(int32_t n);
  static void SetItemMdef(int32_t n);
  static void SetItemExp(uint32_t n);

  // ============================================================
  // 道具加成 getter (item_)
  // ============================================================
  static int64_t GetItemHp();
  static int64_t GetItemMp();
  static int64_t GetItemStr();
  static int64_t GetItemDex();
  static int64_t GetItemInt();
  static int64_t GetItemLuk();
  static int64_t GetItemPro();
  static uint32_t GetItemAcc();
  static uint32_t GetItemAvd();
  static uint32_t GetItemAttack();
  static uint32_t GetItemMagic();
  static uint32_t GetItemExp();
  static int32_t GetItemDef();
  static int32_t GetItemMdef();
};

}  // namespace sdlms
