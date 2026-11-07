#include "src/client/game_instance/character_stat_game_instance.h"

#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_character_instance.h"

#include <wz/Property.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <utility>

namespace sdlms {

// ============================================================
// load
// ============================================================
void CharacterStatGameInstance::Load(const CharacterSaveData& cs) {
  hp_ap_ = cs.ap.hp_ap;
  mp_ap_ = cs.ap.mp_ap;
  str_ap_ = cs.ap.str_ap;
  dex_ap_ = cs.ap.dex_ap;
  int_ap_ = cs.ap.int_ap;
  luk_ap_ = cs.ap.luk_ap;
  exp_point_ = cs.exp;

  remain_ap_ = cs.remain_ap;


  JobSkillGameInstance::remain_point_ = {100, 100};

  Update();
  // MeowDB：总 AP = 25 + 5 × (level - 1)。
  const uint32_t total_ap =
      25 + 5 * (std::max<uint32_t>(1, CharacterGameInstance::self_.level) - 1);
  const uint32_t used_ap =
      hp_ap_ + mp_ap_ + str_ap_ + dex_ap_ + int_ap_ + luk_ap_;
  remain_ap_ = total_ap > used_ap ? total_ap - used_ap : 0;
  hp_point_ = std::clamp<int32_t>(
      static_cast<int32_t>(std::min<uint32_t>(cs.hp, INT32_MAX)), 1,
      hp_point_max_);
  mp_point_ = std::clamp<int32_t>(
      static_cast<int32_t>(std::min<uint32_t>(cs.mp, INT32_MAX)), 1,
      mp_point_max_);

  hp_point_ = 2000;
  mp_point_ = 2000;
}

namespace {

struct HpMpCurve {
  int hp_per_level;
  int mp_per_level;
  int hp_advance;
  int mp_advance;
};

// MeowDB HP/MP 表：10 级转职后的每级成长与转职奖励。
HpMpCurve LoadHpMpCurve(JobType job) {
  switch (job) {
    case JobType::kWarrior:
      return {28, 12, 350, 150};
    case JobType::kBowman:
    case JobType::kThief:
      return {22, 17, 250, 250};
    case JobType::kMagician:
      return {16, 22, 150, 350};
    default:
      return {16, 12, 0, 0};
  }
}

}  // namespace

uint32_t CharacterStatGameInstance::GetBaseHpMax() {
  auto& sf = CharacterGameInstance::self_;
  auto jobs = JobSkillGameInstance::LoadSkiTree(sf.job);
  const int level = std::max(1, static_cast<int>(sf.level));
  if (jobs.size() == 1 || level < 10) {
    return static_cast<uint32_t>(50 + (level - 1) * 16 + hp_ap_ * 16);
  }
  const auto curve = LoadHpMpCurve(jobs[1]);
  // 10 级转职前按 9 级的 Beginner 基础值。
  const int base_hp = 50 + 8 * 16;
  const bool second_advance = jobs.size() >= 3 && level >= 30;
  const int advance_mul = 1 + (second_advance ? 1 : 0);
  return static_cast<uint32_t>(base_hp + curve.hp_advance * advance_mul +
                               (level - 10) * curve.hp_per_level +
                               hp_ap_ * curve.hp_per_level);
}

uint32_t CharacterStatGameInstance::GetBaseMpMax() {
  auto& sf = CharacterGameInstance::self_;
  auto jobs = JobSkillGameInstance::LoadSkiTree(sf.job);
  const int level = std::max(1, static_cast<int>(sf.level));
  if (jobs.size() == 1 || level < 10) {
    return static_cast<uint32_t>(5 + (level - 1) * 12 + mp_ap_ * 12);
  }
  const auto curve = LoadHpMpCurve(jobs[1]);
  const int base_mp = 5 + 8 * 12;
  const bool second_advance = jobs.size() >= 3 && level >= 30;
  const int advance_mul = 1 + (second_advance ? 1 : 0);
  return static_cast<uint32_t>(base_mp + curve.mp_advance * advance_mul +
                               (level - 10) * curve.mp_per_level +
                               mp_ap_ * curve.mp_per_level);
}

// ============================================================
// 主 / 副属性
// ============================================================
int CharacterStatGameInstance::LoadPrimaryStat() {
  auto job = CharacterGameInstance::self_.job;
  auto jobs = JobSkillGameInstance::LoadSkiTree(job);
  if (jobs.size() >= 2) {
    switch (jobs[1]) {
      case JobType::kBeginner:
      case JobType::kWarrior:
        return static_cast<int>(str_ap_ + equip_str_ + item_str_ + ski_str_);
      case JobType::kMagician:
        return static_cast<int>(int_ap_ + equip_int_ + item_int_ + ski_int_);
      case JobType::kBowman:
        return static_cast<int>(dex_ap_ + equip_dex_ + item_dex_ + ski_dex_);
      case JobType::kThief:
        return static_cast<int>(luk_ap_ + equip_luk_ + item_luk_ + ski_luk_);
      default:
        break;
    }
  }
  return static_cast<int>(str_ap_ + equip_str_ + item_str_ + ski_str_);
}

int CharacterStatGameInstance::LoadSecondaryStat() {
  auto job = CharacterGameInstance::self_.job;
  auto jobs = JobSkillGameInstance::LoadSkiTree(job);
  if (jobs.size() >= 2) {
    switch (jobs[1]) {
      case JobType::kWarrior:
        return static_cast<int>(dex_ap_ + equip_dex_ + item_dex_ + ski_dex_);
      case JobType::kMagician:
        return static_cast<int>(luk_ap_ + equip_luk_ + item_luk_ + ski_luk_);
      case JobType::kBowman:
        return static_cast<int>(str_ap_ + equip_str_ + item_str_ + ski_str_);
      case JobType::kThief:  // 指南：匕首/拳套的 Secondary = STR + DEX
        return static_cast<int>(dex_ap_ + equip_dex_ + item_dex_ + ski_dex_);
      default:
        break;
    }
  }
  return static_cast<int>(dex_ap_ + equip_dex_ + item_dex_ + ski_dex_);
}

void CharacterStatGameInstance::UpdateSfSkillStat() {
  ski_hp_ = 0;
  ski_hp_rate_ = 0;
  ski_crit_rate_ = 0;
  ski_crit_damage_ = 0;
  ski_attack_range_ = 0;
  ski_throw_range_ = 0;
  ski_hp_recovery_ = 0;
  ski_mp_rate_ = 0;
  ski_mp_recovery_ = 0;
  ski_mp_ = 0;
  ski_str_ = 0;
  ski_dex_ = 0;
  ski_int_ = 0;
  ski_luk_ = 0;
  ski_pro_ = 0;  // 熟练度

  ski_accuracy_ = 0;  // 命中
  ski_avoid_ = 0;     // 回避
  ski_exp_ = 0;

  ski_attack_def_ = 0;
  ski_magic_def_ = 0;

  auto job_id = CharacterGameInstance::self_.job;
  auto ski_tree = JobSkillGameInstance::LoadSkiTree(job_id);
  auto& skills = SkillGameInstance::Skis();
  for (auto ski_t : ski_tree) {
    auto job_skis = JobSkillGameInstance::LoadJobSkis(ski_t);
    for (auto [k, v] : job_skis) {
      auto ski_lv = JobSkillGameInstance::LoadSkiLevel(k);
      if (ski_lv == 0) {
        continue;
      }
      if (!skills.contains(k)) {
        continue;
      }
      auto& ski = skills.at(k);
      if (ski.passive) {
        ski.passive(ski_lv);
      }
    }
  }
}

// 主动技能 buff: 每次整体重算, 不做增量 (重复施放/漏调 end 都不会漂移)
void CharacterStatGameInstance::UpdateSfBuffStat() {
  buff_hp_ = 0;
  buff_mp_ = 0;
  buff_pad_ = 0;
  buff_mad_ = 0;
  buff_acc_ = 0;
  buff_avd_ = 0;
  buff_pdd_ = 0;
  buff_pdd_rate_ = 0;
  buff_mdd_ = 0;
  buff_speed_ = 0;
  buff_jump_ = 0;
  std::flat_set<std::u16string> applied;  // 同一技能只算一次
  for (auto& buff : SkillGameInstance::ski_) {
    if (!applied.insert(buff.id).second) {
      continue;
    }
    auto stat = SkillGameInstance::LoadSkiBuffStat(buff.id, buff.lv);
    if (buff.buff_stat) {
      buff.buff_stat(buff.lv, stat);
    }
    buff_hp_ += stat.hp;
    buff_mp_ += stat.mp;
    buff_pad_ += stat.pad;
    buff_mad_ += stat.mad;
    buff_acc_ += stat.acc;
    buff_avd_ += stat.eva;
    buff_pdd_ += stat.pdd;
    buff_pdd_rate_ += stat.pdd_rate;
    buff_mdd_ += stat.mdd;
    buff_speed_ += stat.speed;
    buff_jump_ += stat.jump;
  }
}

void CharacterStatGameInstance::UpdateSfEquipStat() {
  equip_hp_ = 0;
  equip_mp_ = 0;
  equip_str_ = 0;
  equip_dex_ = 0;
  equip_int_ = 0;
  equip_luk_ = 0;
  equip_pro_ = 0;  // 熟练度

  equip_accuracy_ = 0;  // 命中
  equip_avoid_ = 0;     // 回避

  equip_attack_ = 0;  // pad
  equip_magic_ = 0;

  equip_attack_def_ = 0;
  equip_magic_def_ = 0;

  equip_exp_ = 0;

  const auto& sf = CharacterGameInstance::self_;

  // 单个来源（装备本体 / 已成功卷轴）的固定加成累加
  auto add_inc =
      [&](const std::flat_map<EquipGameInstance::IncType, int>& inc) {
        for (auto [t, v] : inc) {
          switch (t) {
            case EquipGameInstance::IncType::kWeaponPad:
              equip_attack_ += static_cast<uint32_t>(v);
              break;
            case EquipGameInstance::IncType::kWeaponMad:
              equip_magic_ += static_cast<uint32_t>(v);
              break;
            case EquipGameInstance::IncType::kStr:
              equip_str_ += v;
              break;
            case EquipGameInstance::IncType::kDex:
              equip_dex_ += v;
              break;
            case EquipGameInstance::IncType::kInt:
              equip_int_ += v;
              break;
            case EquipGameInstance::IncType::kLuk:
              equip_luk_ += v;
              break;
            case EquipGameInstance::IncType::kPdd:
              equip_attack_def_ += v;
              break;
            case EquipGameInstance::IncType::kAcc:
              equip_accuracy_ += static_cast<uint32_t>(v);
              break;
            default:  // 攻速/CRT 等暂不进入面板数值
              break;
          }
        }
      };

  // 遍历所有已穿戴装备：本体 inc + 每个成功卷轴的 inc
  for (const auto& opt : {sf.weapon, sf.cap, sf.cape, sf.coat, sf.glove,
                          sf.longcoat, sf.pant, sf.shoes, sf.shield, sf.ring0,
                          sf.ring1, sf.ring2, sf.ring3, sf.accessory}) {
    if (!opt.has_value()) {
      continue;
    }
    const auto& eq = opt.value();
    add_inc(EquipGameInstance::LoadEquipInc(eq.id));
    for (const auto& scroll : eq.scroll) {
      if (!scroll.success) {
        continue;
      }
      add_inc(EquipGameInstance::LoadScrollInc(scroll.id));
    }
  }
}

// ============================================================
// update
// ============================================================
void CharacterStatGameInstance::Update() {
  UpdateSfEquipStat();
  UpdateSfSkillStat();
  UpdateSfBuffStat();

  // ---- 总属性（AP + 装备 + 道具/Buff + 技能被动）----
  str_point_ = str_ap_ + equip_str_ + item_str_ + ski_str_;
  dex_point_ = dex_ap_ + equip_dex_ + item_dex_ + ski_dex_;
  int_point_ = int_ap_ + equip_int_ + item_int_ + ski_int_;
  luk_point_ = luk_ap_ + equip_luk_ + item_luk_ + ski_luk_;

  // ---- HP / MP 上限 ----
  const int32_t old_hp_max = hp_point_max_;
  const int64_t base_hp = GetBaseHpMax();
  const int64_t hp_with_rate =
      base_hp * (100 + std::max<int64_t>(0, ski_hp_rate_)) / 100;
  hp_point_max_ = static_cast<int32_t>(std::clamp<int64_t>(
      hp_with_rate + equip_hp_ + item_hp_ + ski_hp_ + buff_hp_, 1, INT32_MAX));
  // Max HP Increase 加点后当前 HP 同步增加差额。
  if (old_hp_max > 0 && old_hp_max != INT32_MAX && hp_point_max_ > old_hp_max &&
      hp_point_ > 0) {
    hp_point_ += hp_point_max_ - old_hp_max;
  }
  const int32_t old_mp_max = mp_point_max_;
  const int64_t base_mp = GetBaseMpMax();
  const int64_t mp_with_rate =
      base_mp * (100 + std::max<int64_t>(0, ski_mp_rate_)) / 100;
  mp_point_max_ = static_cast<int32_t>(
      std::clamp<int64_t>(mp_with_rate + equip_mp_ + item_mp_ + ski_mp_ +
                              buff_mp_,
                          1, INT32_MAX));
  // Max MP Increase 加点后当前 MP 同步增加差额。
  if (old_mp_max > 0 && old_mp_max != INT32_MAX && mp_point_max_ > old_mp_max &&
      mp_point_ > 0) {
    mp_point_ += mp_point_max_ - old_mp_max;
  }
  hp_point_ = std::min<int32_t>(hp_point_, hp_point_max_);
  mp_point_ = std::min<int32_t>(mp_point_, mp_point_max_);

  // ---- 主属性 / 副属性（按职业树推导）----
  const auto primary_ap = LoadPrimaryStat();
  const auto secondary_ap = LoadSecondaryStat();

  // ---- 攻击力 ----
  // AttackPower：参与最终乘法的攻击力（装备 pad + 道具/Buff pad）
  const uint32_t atk_power = equip_attack_ + item_attack_ + buff_pad_;
  // 魔法攻击力（法师公式的 MagicTotal 组成项）
  const uint32_t mad_power = equip_magic_ + item_magic_ + buff_mad_;

  // ---- 武器动作倍率 W（MeowDB：普通动作包络，MIN 取最低、MAX 取最高）----
  auto [weapon_mult_min, weapon_mult_max] = []() -> std::pair<float, float> {
    if (!CharacterGameInstance::self_.weapon.has_value()) {
      return {1.0f, 1.0f};
    }
    switch (EquipGameInstance::LoadWeaponEnum(
        CharacterGameInstance::self_.weapon->id)) {
      case EquipGameInstance::WeaponEnum::kSword1H:  // 单手剑
        return {1.8f, 1.8f};                         // swing/stab 1.8
      case EquipGameInstance::WeaponEnum::kAxe1H:    // 单手斧
      case EquipGameInstance::WeaponEnum::kMace1H:   // 单手钝器
        return {1.2f, 2.4f};  // stab 1.2 ~ swing 2.4（mixed 1.8 在包络内）
      case EquipGameInstance::WeaponEnum::kDagger:   // 匕首
        return {1.0f, 2.0f};                         // swing 1.0 ~ stab 2.0
      case EquipGameInstance::WeaponEnum::kSword2H:  // 双手剑
        return {2.5f, 2.5f};
      case EquipGameInstance::WeaponEnum::kAxe2H:   // 双手斧
      case EquipGameInstance::WeaponEnum::kMace2H:  // 双手钝器
        return {2.0f, 3.0f};                        // stab 2.0 ~ swing 3.0
      case EquipGameInstance::WeaponEnum::kSpear:  // 长枪：swing 1.5 / stab 3.5
      case EquipGameInstance::WeaponEnum::kPolearm:  // 矛：swing 3.5 / stab 1.5
        return {1.5f, 3.5f};
      case EquipGameInstance::WeaponEnum::kBow:       // 弓
      case EquipGameInstance::WeaponEnum::kCrossbow:  // 弩
      case EquipGameInstance::WeaponEnum::kClaw:      // 拳套
        return {1.0f, 2.5f};                          // 近身 1.0 ~ 射击 2.5
      case EquipGameInstance::WeaponEnum::kWand:      // 短杖物理敲击
      case EquipGameInstance::WeaponEnum::kStaff:     // 长杖物理敲击
        return {1.8f, 1.8f};
      default:  // KNUCKLE/GUN 等指南未收录，占位
        return {1.0f, 1.0f};
    }
  }();

  // ---- 熟练度：m =（MasteryLevel / 10 + 0.1）× 0.7（物理/魔法共用）----
  const int mastery_lv = static_cast<int>(
      std::clamp<int64_t>(ski_pro_ + equip_pro_ + item_pro_, 0, 10));
  const float mastery = (static_cast<float>(mastery_lv) / 10.0f + 0.1f) * 0.7f;

  // ---- 物理武器攻击伤害（MeowDB「Physical / Weapon attacks」）----
  // K = SkillDamage / 100（面板按 K=1 显示）
  // MIN = K × TotalWATK × (0.8 + (P×m×Wmin + Q)/StatDiv + AttackPower/APDiv)
  // MAX = K × TotalWATK × (1.0 + (P×Wmax + Q)/StatDiv + AttackPower/APDiv)
  // 普通近战/射击：StatDiv = 100，APDiv = 50
  // TotalWATK = 全部装备提供的武器攻击力（equip_attack）
  // AttackPower = 道具/Buff 提供的攻击力加成（item_attack）
  constexpr float stat_div = 100.0f;
  constexpr float ap_div = 50.0f;
  const float total_watk = static_cast<float>(equip_attack_);
  const float attack_power = static_cast<float>(item_attack_);
  const float phys_max =
      total_watk * (1.0f +
                    (static_cast<float>(primary_ap) * weapon_mult_max +
                     static_cast<float>(secondary_ap)) /
                        stat_div +
                    attack_power / ap_div);
  const float phys_min =
      total_watk *
      (0.8f +
       (static_cast<float>(primary_ap) * mastery * weapon_mult_min +
        static_cast<float>(secondary_ap)) /
           stat_div +
       attack_power / ap_div);

  // ---- 法师魔法伤害（MeowDB「Magic / Spells」）----
  // K = SkillPower / 100（界面按 K=1 显示）
  // m =（SpellMastery / 10 + 0.1）× 0.7
  // MagicTotal = floor(TotalINT / 2) + EquipmentMATK + ScrollMATK + BuffMATK
  // MAX = K × MagicTotal × (1 + TotalINT / 125)
  // MIN = K × MagicTotal × (1 + TotalINT × m / 125)
  const uint32_t magic_total = static_cast<uint32_t>(int_point_ / 2) +
                               equip_magic_ + item_magic_ + buff_mad_;
  const float mage_max = static_cast<float>(magic_total) *
                         (1.0f + static_cast<float>(int_point_) / 125.0f);
  const float mage_min =
      static_cast<float>(magic_total) *
      (1.0f + static_cast<float>(int_point_) * mastery / 125.0f);

  const auto jobs =
      JobSkillGameInstance::LoadSkiTree(CharacterGameInstance::self_.job);
  const bool is_magician = jobs.size() >= 2 && jobs[1] == JobType::kMagician;

  // 面板取整按指南口径 trunc（直接截断，而非四舍五入）
  if (is_magician) {
    max_atk_ = static_cast<uint32_t>(mage_max);
    min_atk_ = static_cast<uint32_t>(mage_min);
  } else {
    max_atk_ = static_cast<uint32_t>(phys_max);
    min_atk_ = static_cast<uint32_t>(phys_min);
  }
  // 防御下限，避免 min > max 的显示异常
  if (min_atk_ > max_atk_) {
    min_atk_ = max_atk_;
  }

  // ---- 攻击力 / 魔法攻击力（界面展示值）----
  attack_ = static_cast<int32_t>(atk_power);
  magic_ = static_cast<int32_t>(mad_power);

  // ---- 命中 / 回避 ----
  accuracy_ = ski_accuracy_ + equip_accuracy_ + item_accuracy_ + buff_acc_;
  avoid_ = ski_avoid_ + equip_avoid_ + item_avoid_ + buff_avd_;

  // ---- 防御 ----
  const int32_t pdd_base =
      ski_attack_def_ + equip_attack_def_ + item_attack_def_ + buff_pdd_;
  attack_def_ = pdd_base + pdd_base * std::max(0, buff_pdd_rate_) / 100;
  magic_def_ = ski_magic_def_ + equip_magic_def_ + item_magic_def_ + buff_mdd_;

  // ---- 暴击 ----
  // MeowDB：弓箭手基础 5% 暴击；Precise Strikes 提供额外暴击率。
  // crit_rate_ 每次整体重算，不能把上一轮结果继续叠加。
  uint32_t base_crit_rate = 0;
  {
    auto jobs =
        JobSkillGameInstance::LoadSkiTree(CharacterGameInstance::self_.job);
    if (jobs.size() >= 2 &&
        (jobs[1] == JobType::kBowman || jobs[1] == JobType::kThief)) {
      base_crit_rate = 5;
    }
  }
  crit_rate_ = base_crit_rate + ski_crit_rate_;
  crit_damage_ = 120 + ski_crit_damage_;

  // 速度/跳跃上限只在属性变化时刷新 (技能 buff / 药水 / 装备都会走到这里)
  UpdateMoveLimits();
}

// ============================================================
// 移动上限
// ============================================================
// 速度 1 点约等于 1.25 px/s (与原 0001002 的换算一致)
void CharacterStatGameInstance::UpdateMoveLimits() {
  // 移速 = 技能 buff(轻功/极速等) + 道具(药水等)
  const auto speed =
      static_cast<float>(GetBuffSpeed() + std::max(GetItemSpeed(), 0));
  const auto jump = static_cast<float>(GetBuffJump());
  hspeed_max_ = 125.0f + 1.25f * speed;
  hspeed_min_ = -125.0f - 1.25f * speed;
  // 跳跃: 每点约 +1% 初速度
  vspeed_max_ = 670.0f * (1.0f + 0.01f * jump);
}

float CharacterStatGameInstance::GetHspeedMin() {
  return hspeed_min_;
}
float CharacterStatGameInstance::GetHspeedMax() {
  return hspeed_max_;
}
float CharacterStatGameInstance::GetVspeedMin() {
  return vspeed_min_;
}
float CharacterStatGameInstance::GetVspeedMax() {
  return vspeed_max_;
}

// ============================================================
// upgrade / exp
// ============================================================
bool CharacterStatGameInstance::Upgrade() {
  if (exp_point_ >= exp_point_max_) {
    exp_point_ -= exp_point_max_;
    auto& sf = CharacterGameInstance::self_;
    sf.level += 1;

    remain_ap_ += 5;
    JobSkillGameInstance::RecalculateRemainSp();

    fbs::ClientCharacterLvUpT ccl;
    ccl.map_id = SceneSystemInstance::map_id_;
    ClientRequest::SendToHost(ccl);

    ServerCharacterInstance::HandleLvUp(sf);
    Update();
    // 升级后按 MeowDB 的新 HP/MP 上限回满。
    hp_point_ = hp_point_max_;
    mp_point_ = mp_point_max_;
    return true;
  }
  return false;
}

void CharacterStatGameInstance::AddExp(uint32_t num) {
  if (num == 0)
    return;
  exp_point_ += num;
  Upgrade();
  Update();
}

// ============================================================
// HP / MP
// ============================================================
void CharacterStatGameInstance::SetHp(int32_t n) {
  Update();
  hp_point_ = std::min<int32_t>(n, hp_point_max_);
  if (hp_point_ < 0)
    hp_point_ = 0;
}

void CharacterStatGameInstance::SetMp(int32_t n) {
  Update();
  mp_point_ = std::min<int32_t>(n, mp_point_max_);
  if (mp_point_ < 0)
    mp_point_ = 0;
}

void CharacterStatGameInstance::SetCritRate(uint32_t n) {
  crit_rate_ = n;
}
void CharacterStatGameInstance::SetCritDamage(uint32_t n) {
  crit_damage_ = n;
}

int32_t CharacterStatGameInstance::GetHp() {
  return hp_point_;
}
int32_t CharacterStatGameInstance::GetMp() {
  return mp_point_;
}
int32_t CharacterStatGameInstance::GetHpMax() {
  return hp_point_max_;
}
int32_t CharacterStatGameInstance::GetMpMax() {
  return mp_point_max_;
}
uint64_t CharacterStatGameInstance::GetExp() {
  return exp_point_;
}

uint64_t CharacterStatGameInstance::GetExpMax() {
  static std::flat_map<int, uint64_t> cache;
  auto& sf = CharacterGameInstance::self_;
  if (!cache.contains(sf.level)) {
    auto exp_node = WzResource::ms_->get_root()->find(u"LevelExp.img");
    exp_node = exp_node->get_child(std::to_string(sf.level));
    if (exp_node == nullptr) {
      return UINT64_MAX;
    }
    if (exp_node->type == wz::Type::Long) {
      exp_point_max_ = static_cast<wz::Property<int64_t>*>(exp_node)->get();
    } else {
      exp_point_max_ = static_cast<wz::Property<int>*>(exp_node)->get();
    }
    cache[sf.level] = exp_point_max_;
  }
  exp_point_max_ = cache.at(sf.level);
  return exp_point_max_;
}

// ============================================================
// 剩余 AP
// ============================================================
uint32_t CharacterStatGameInstance::GetRemainAp() {
  return remain_ap_;
}

void CharacterStatGameInstance::SetRemainAp(uint32_t n) {
  remain_ap_ = n;
}

// ============================================================
// AP 加点
// ============================================================
void CharacterStatGameInstance::SetStrAp(uint32_t n) {
  str_ap_ = n;
  Update();
}
uint32_t CharacterStatGameInstance::GetStrAp() {
  return str_ap_;
}

void CharacterStatGameInstance::SetDexAp(uint32_t n) {
  dex_ap_ = n;
  Update();
}
uint32_t CharacterStatGameInstance::GetDexAp() {
  return dex_ap_;
}

void CharacterStatGameInstance::SetIntAp(uint32_t n) {
  int_ap_ = n;
  Update();
}
uint32_t CharacterStatGameInstance::GetIntAp() {
  return int_ap_;
}  // 修复：原返回 dex_ap

void CharacterStatGameInstance::SetLukAp(uint32_t n) {
  luk_ap_ = n;
  Update();
}
uint32_t CharacterStatGameInstance::GetLukAp() {
  return luk_ap_;
}

void CharacterStatGameInstance::SetHpAp(uint32_t n) {
  hp_ap_ = n;
  Update();
}
uint32_t CharacterStatGameInstance::GetHpAp() {
  return hp_ap_;
}

void CharacterStatGameInstance::SetMpAp(uint32_t n) {
  mp_ap_ = n;
  Update();
}
uint32_t CharacterStatGameInstance::GetMpAp() {
  return mp_ap_;
}

// ============================================================
// 主属性 getter
// ============================================================
uint32_t CharacterStatGameInstance::GetStrPoint() {
  return str_point_;
}
uint32_t CharacterStatGameInstance::GetDexPoint() {
  return dex_point_;
}
uint32_t CharacterStatGameInstance::GetIntPoint() {
  return int_point_;
}
uint32_t CharacterStatGameInstance::GetLukPoint() {
  return luk_point_;
}

// ============================================================
// 最终战斗属性 getter
// ============================================================
uint32_t CharacterStatGameInstance::GetMinAtk() {
  return min_atk_;
}
uint32_t CharacterStatGameInstance::GetMaxAtk() {
  return max_atk_;
}

uint32_t CharacterStatGameInstance::RollAttackDamage(uint32_t skill_percent,
                                                     bool* critical) {
  uint32_t lo = min_atk_;
  uint32_t hi = max_atk_;
  if (hi < lo) {
    std::swap(lo, hi);
  }
  if (hi == 0) {
    if (critical != nullptr) {
      *critical = false;
    }
    return 1;
  }
  std::uniform_int_distribution<uint32_t> dist(lo, hi);
  uint64_t damage = static_cast<uint64_t>(dist(RandomGameInstance::gen_)) *
                    skill_percent / 100;
  std::uniform_int_distribution<uint32_t> crit_roll(0, 99);
  const bool is_critical = crit_roll(RandomGameInstance::gen_) < crit_rate_;
  if (is_critical) {
    damage = damage * std::max<uint32_t>(100, crit_damage_) / 100;
  }
  if (critical != nullptr) {
    *critical = is_critical;
  }
  return static_cast<uint32_t>(std::max<uint64_t>(1, damage));
}

uint32_t CharacterStatGameInstance::RollMagicDamage(uint32_t skill_percent,
                                                    uint32_t mastery,
                                                    bool* critical) {
  const uint32_t magic_total = static_cast<uint32_t>(int_point_ / 2) +
                               equip_magic_ + item_magic_ + buff_mad_;
  if (magic_total == 0) {
    if (critical != nullptr) {
      *critical = false;
    }
    return 1;
  }

  // Skill.wz 的 mastery 是 1..10，越高伤害浮动越小。
  const uint32_t mastery_lv = std::clamp<uint32_t>(mastery, 1, 10);
  const float m =
      (static_cast<float>(mastery_lv) / 10.0f + 0.1f) * 0.7f;
  const float magic_max =
      static_cast<float>(magic_total) *
      (1.0f + static_cast<float>(int_point_) / 125.0f);
  const float magic_min =
      static_cast<float>(magic_total) *
      (1.0f + static_cast<float>(int_point_) * m / 125.0f);

  std::uniform_real_distribution<float> dist(magic_min, magic_max);
  uint64_t damage = static_cast<uint64_t>(
      std::max(0.0f, dist(RandomGameInstance::gen_)) * skill_percent / 100.0f);

  std::uniform_int_distribution<uint32_t> crit_roll(0, 99);
  const bool is_critical = crit_roll(RandomGameInstance::gen_) < crit_rate_;
  if (is_critical) {
    damage = damage * std::max<uint32_t>(100, crit_damage_) / 100;
  }
  if (critical != nullptr) {
    *critical = is_critical;
  }
  return static_cast<uint32_t>(std::max<uint64_t>(1, damage));
}

uint32_t CharacterStatGameInstance::CalcHitChance(uint32_t mob_eva,
                                                  uint32_t mob_level) {
  const double acc = static_cast<double>(accuracy_);
  const double eva = static_cast<double>(mob_eva);
  const double player_level = static_cast<double>(
      std::max<uint32_t>(1, CharacterGameInstance::self_.level));
  const double level_gap =
      std::max(0.0, static_cast<double>(std::max<uint32_t>(1, mob_level)) -
                        player_level);
  const double roll_center = 100.0 * acc / (10.0 * level_gap + 255.0);
  double chance = 0.0;
  if (eva <= 0.0) {
    chance = 1.0;
  } else if (acc <= 0.0) {
    chance = 0.0;
  } else {
    const double diff = roll_center - eva;
    const double spread = 0.15 + 0.2 / (1.0 + std::exp(diff / 12.0));
    if (diff >= 25.0 + level_gap * (0.5 * player_level + 15.0)) {
      chance = 1.0;
    } else if (roll_center <= 0.0) {
      chance = eva <= 0.0 ? 1.0 : 0.0;
    } else {
      const double ratio = eva / roll_center;
      if (ratio <= 1.0 - spread) {
        chance = 1.0;
      } else if (ratio >= 1.0 + spread) {
        chance = 0.0;
      } else {
        chance = (1.0 + spread - ratio) / (2.0 * spread);
      }
    }
  }
  return static_cast<uint32_t>(std::clamp<int32_t>(
      static_cast<int32_t>(std::lround(chance * 100.0)), 0, 100));
}
uint32_t CharacterStatGameInstance::GetCritRate() {
  return crit_rate_;
}
uint32_t CharacterStatGameInstance::GetCritDamage() {
  return crit_damage_;
}
uint32_t CharacterStatGameInstance::GetAcc() {
  return accuracy_;
}
uint32_t CharacterStatGameInstance::GetAvd() {
  return avoid_;
}
int32_t CharacterStatGameInstance::GetDef() {
  return attack_def_;
}
int32_t CharacterStatGameInstance::GetMdef() {
  return magic_def_;
}
int32_t CharacterStatGameInstance::GetAttack() {
  return attack_;
}
int32_t CharacterStatGameInstance::GetMagic() {
  return magic_;
}

// ============================================================
// 技能加成 setter (ski_)
// ============================================================
void CharacterStatGameInstance::SetSkiHp(int64_t n) {
  ski_hp_ = n;
  Update();
}
void CharacterStatGameInstance::ApplyPassiveHpRate(int64_t n) {
  ski_hp_rate_ = std::max<int64_t>(0, n);
}
void CharacterStatGameInstance::ApplyPassiveMpRate(int64_t n) {
  ski_mp_rate_ = std::max<int64_t>(0, n);
}
void CharacterStatGameInstance::ApplyPassiveMpRecovery(uint32_t n) {
  ski_mp_recovery_ = std::max<uint32_t>(0, n);
}
uint32_t CharacterStatGameInstance::GetMpRecoveryRate() {
  return ski_mp_recovery_;
}
void CharacterStatGameInstance::ApplyPassiveCritRate(uint32_t n) {
  ski_crit_rate_ = n;
}
void CharacterStatGameInstance::ApplyPassiveCritDamage(uint32_t n) {
  ski_crit_damage_ = n;
}
void CharacterStatGameInstance::ApplyPassiveAttackRange(uint32_t n) {
  ski_attack_range_ = n;
}
void CharacterStatGameInstance::ApplyPassiveThrowRange(uint32_t n) {
  ski_throw_range_ = n;
}
uint32_t CharacterStatGameInstance::GetThrowRangeBonus() {
  return ski_throw_range_;
}
void CharacterStatGameInstance::ApplyPassiveAcc(uint32_t n) {
  ski_accuracy_ += n;
}
void CharacterStatGameInstance::ApplyPassiveAvoid(uint32_t n) {
  ski_avoid_ += n;
}
void CharacterStatGameInstance::ApplyPassiveHpRecovery(uint32_t n) {
  ski_hp_recovery_ = std::max<uint32_t>(0, n);
}
uint32_t CharacterStatGameInstance::GetHpRecoveryRate() {
  return ski_hp_recovery_;
}
uint32_t CharacterStatGameInstance::GetAttackRangeBonus() {
  return ski_attack_range_;
}
void CharacterStatGameInstance::SetSkiMp(int64_t n) {
  ski_mp_ = n;
  Update();
}
void CharacterStatGameInstance::SetSkiStr(int64_t n) {
  ski_str_ = n;
  Update();
}
void CharacterStatGameInstance::SetSkiDex(int64_t n) {
  ski_dex_ = n;
  Update();
}
void CharacterStatGameInstance::SetSkiInt(int64_t n) {
  ski_int_ = n;
  Update();
}
void CharacterStatGameInstance::SetSkiLuk(int64_t n) {
  ski_luk_ = n;
  Update();
}
void CharacterStatGameInstance::SetSkiPro(int64_t n) {
  ski_pro_ = n;
  Update();
}
void CharacterStatGameInstance::SetSkiAcc(uint32_t n) {
  ski_accuracy_ = n;
  Update();
}
void CharacterStatGameInstance::SetSkiAvd(uint32_t n) {
  ski_avoid_ = n;
  Update();
}
void CharacterStatGameInstance::SetSkiDef(int32_t n) {
  ski_attack_def_ = n;
  Update();
}
void CharacterStatGameInstance::SetSkiMdef(int32_t n) {
  ski_magic_def_ = n;
  Update();
}  // 修复：原写 ski_attack_def
void CharacterStatGameInstance::SetSkiExp(uint32_t n) {
  ski_exp_ = n;
  Update();
}

// ============================================================
// 技能加成 getter (ski_)
// ============================================================
int64_t CharacterStatGameInstance::GetSkiHp() {
  return ski_hp_;
}
int64_t CharacterStatGameInstance::GetSkiMp() {
  return ski_mp_;
}
int64_t CharacterStatGameInstance::GetSkiStr() {
  return ski_str_;
}
int64_t CharacterStatGameInstance::GetSkiDex() {
  return ski_dex_;
}
int64_t CharacterStatGameInstance::GetSkiInt() {
  return ski_int_;
}
int64_t CharacterStatGameInstance::GetSkiLuk() {
  return ski_luk_;
}
int64_t CharacterStatGameInstance::GetSkiPro() {
  return ski_pro_;
}
uint32_t CharacterStatGameInstance::GetSkiAcc() {
  return ski_accuracy_;
}
uint32_t CharacterStatGameInstance::GetSkiAvd() {
  return ski_avoid_;
}
int32_t CharacterStatGameInstance::GetSkiDef() {
  return ski_attack_def_;
}
int32_t CharacterStatGameInstance::GetSkiMdef() {
  return ski_magic_def_;
}
uint32_t CharacterStatGameInstance::GetSkiExp() {
  return ski_exp_;
}

// ============================================================
// 装备加成 setter (equip_)
// ============================================================
void CharacterStatGameInstance::SetEquipHp(int64_t n) {
  equip_hp_ = n;
  Update();
}
void CharacterStatGameInstance::SetEquipMp(int64_t n) {
  equip_mp_ = n;
  Update();
}
void CharacterStatGameInstance::SetEquipStr(int64_t n) {
  equip_str_ = n;
  Update();
}
void CharacterStatGameInstance::SetEquipDex(int64_t n) {
  equip_dex_ = n;
  Update();
}
void CharacterStatGameInstance::SetEquipInt(int64_t n) {
  equip_int_ = n;
  Update();
}
void CharacterStatGameInstance::SetEquipLuk(int64_t n) {
  equip_luk_ = n;
  Update();
}
void CharacterStatGameInstance::SetEquipPro(int64_t n) {
  equip_pro_ = n;
  Update();
}
void CharacterStatGameInstance::SetEquipAcc(uint32_t n) {
  equip_accuracy_ = n;
  Update();
}
void CharacterStatGameInstance::SetEquipAvd(uint32_t n) {
  equip_avoid_ = n;
  Update();
}
void CharacterStatGameInstance::SetEquipDef(int32_t n) {
  equip_attack_def_ = n;
  Update();
}
void CharacterStatGameInstance::SetEquipMdef(int32_t n) {
  equip_magic_def_ = n;
  Update();
}
void CharacterStatGameInstance::SetEquipExp(uint32_t n) {
  equip_exp_ = n;
  Update();
}
void CharacterStatGameInstance::SetEquipAttack(uint32_t n) {
  equip_attack_ = n;
  Update();
}
void CharacterStatGameInstance::SetEquipMagic(uint32_t n) {
  equip_magic_ = n;
  Update();
}
// ============================================================
// 装备加成 getter (equip_)
// ============================================================
int64_t CharacterStatGameInstance::GetEquipHp() {
  return equip_hp_;
}
int64_t CharacterStatGameInstance::GetEquipMp() {
  return equip_mp_;
}
int64_t CharacterStatGameInstance::GetEquipStr() {
  return equip_str_;
}
int64_t CharacterStatGameInstance::GetEquipDex() {
  return equip_dex_;
}
int64_t CharacterStatGameInstance::GetEquipInt() {
  return equip_int_;
}
int64_t CharacterStatGameInstance::GetEquipLuk() {
  return equip_luk_;
}
int64_t CharacterStatGameInstance::GetEquipPro() {
  return equip_pro_;
}
uint32_t CharacterStatGameInstance::GetEquipAcc() {
  return equip_accuracy_;
}
uint32_t CharacterStatGameInstance::GetEquipAvd() {
  return equip_avoid_;
}
int32_t CharacterStatGameInstance::GetEquipDef() {
  return equip_attack_def_;
}
int32_t CharacterStatGameInstance::GetEquipMdef() {
  return equip_magic_def_;
}
uint32_t CharacterStatGameInstance::GetEquipExp() {
  return equip_exp_;
}

// ============================================================
// 道具加成 setter (item_)
// ============================================================
void CharacterStatGameInstance::SetItemHp(int64_t n) {
  item_hp_ = n;
  Update();
}
void CharacterStatGameInstance::SetItemMp(int64_t n) {
  item_mp_ = n;
  Update();
}
void CharacterStatGameInstance::SetItemStr(int64_t n) {
  item_str_ = n;
  Update();
}
void CharacterStatGameInstance::SetItemDex(int64_t n) {
  item_dex_ = n;
  Update();
}
void CharacterStatGameInstance::SetItemInt(int64_t n) {
  item_int_ = n;
  Update();
}
void CharacterStatGameInstance::SetItemLuk(int64_t n) {
  item_luk_ = n;
  Update();
}
void CharacterStatGameInstance::SetItemPro(int64_t n) {
  item_pro_ = n;
  Update();
}
void CharacterStatGameInstance::SetItemAcc(uint32_t n) {
  item_accuracy_ = n;
  Update();
}
void CharacterStatGameInstance::SetItemAvd(uint32_t n) {
  item_avoid_ = n;
  Update();
}
void CharacterStatGameInstance::SetItemDef(int32_t n) {
  item_attack_def_ = n;
  Update();
}
void CharacterStatGameInstance::SetItemMdef(int32_t n) {
  item_magic_def_ = n;
  Update();
}
void CharacterStatGameInstance::SetItemExp(uint32_t n) {
  item_exp_ = n;
  Update();
}

void CharacterStatGameInstance::SetItemAttack(uint32_t n) {
  item_attack_ = n;
  Update();
}
void CharacterStatGameInstance::SetItemMagic(uint32_t n) {
  item_magic_ = n;
  Update();
}

// ============================================================
// 道具加成 getter (item_)
// ============================================================
int64_t CharacterStatGameInstance::GetItemHp() {
  return item_hp_;
}
int64_t CharacterStatGameInstance::GetItemMp() {
  return item_mp_;
}
int64_t CharacterStatGameInstance::GetItemStr() {
  return item_str_;
}
int64_t CharacterStatGameInstance::GetItemDex() {
  return item_dex_;
}
int64_t CharacterStatGameInstance::GetItemInt() {
  return item_int_;
}
int64_t CharacterStatGameInstance::GetItemLuk() {
  return item_luk_;
}
int64_t CharacterStatGameInstance::GetItemPro() {
  return item_pro_;
}
uint32_t CharacterStatGameInstance::GetItemAcc() {
  return item_accuracy_;
}
uint32_t CharacterStatGameInstance::GetItemAvd() {
  return item_avoid_;
}
uint32_t CharacterStatGameInstance::GetItemAttack() {
  return item_attack_;
}
uint32_t CharacterStatGameInstance::GetItemMagic() {
  return item_magic_;
}
uint32_t CharacterStatGameInstance::GetItemExp() {
  return item_exp_;
}
int32_t CharacterStatGameInstance::GetItemDef() {
  return item_attack_def_;
}
int32_t CharacterStatGameInstance::GetItemMdef() {
  return item_magic_def_;
}
void CharacterStatGameInstance::SetItemSpeed(int32_t n) {
  item_speed_ = std::max(n, 0);
  Update();
}
int32_t CharacterStatGameInstance::GetItemSpeed() {
  return item_speed_;
}

// ============================================================
// buff 加成 getter (buff_)
// ============================================================
int64_t CharacterStatGameInstance::GetBuffHp() {
  return buff_hp_;
}
int64_t CharacterStatGameInstance::GetBuffMp() {
  return buff_mp_;
}
uint32_t CharacterStatGameInstance::GetBuffPad() {
  return buff_pad_;
}
uint32_t CharacterStatGameInstance::GetBuffMad() {
  return buff_mad_;
}
uint32_t CharacterStatGameInstance::GetBuffAcc() {
  return buff_acc_;
}
uint32_t CharacterStatGameInstance::GetBuffAvd() {
  return buff_avd_;
}
int32_t CharacterStatGameInstance::GetBuffPdd() {
  return buff_pdd_;
}
int32_t CharacterStatGameInstance::GetBuffMdd() {
  return buff_mdd_;
}
int32_t CharacterStatGameInstance::GetBuffSpeed() {
  return buff_speed_;
}
int32_t CharacterStatGameInstance::GetBuffJump() {
  return buff_jump_;
}

}  // namespace sdlms
