#include "character_stat_game_instance.h"
#include "character_game_instance.h"
#include "equip_game_instance.h"
#include "skill_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_character_instance.h"
#include "wz/Property.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
#include <utility>

// ============================================================
// load
// ============================================================
void CharacterStatGameInstance::Load(const CharacterSaveData &cs) {
  str_ap = cs.ap.str_ap;
  dex_ap = cs.ap.dex_ap;
  int_ap = cs.ap.int_ap;
  luk_ap = cs.ap.luk_ap;

  remain_ap = cs.remain_ap;

  Update();
}

uint32_t CharacterStatGameInstance::GetBaseHpMax() {
  int base_hp_per = 0;
  int base_hp = 0;
  auto &sf = CharacterGameInstance::self;
  auto sf_job = sf.job;
  auto jobs = JobSkillGameInstance::LoadSkiTree(sf_job);
  if (jobs.size() == 1) {
    // beginner
    base_hp_per = 16;
    base_hp = 50 + (sf.level - 1) * base_hp_per;
    return base_hp + base_hp_per * hp_ap;
  }
  auto job = jobs[1];
  switch (job) {
  case JobType::kWarrior: {
    base_hp_per = 28;
    base_hp = 50 + 16 * 9 + (sf.level - 10) * base_hp_per;
    break;
  }
  case JobType::kMagician: {
    base_hp_per = 16;
    base_hp = 50 + 16 * 7 + (sf.level - 8) * base_hp_per;
    break;
  }
  case JobType::kBowman: {
    base_hp_per = 22;
    base_hp = 50 + 16 * 9 + (sf.level - 10) * base_hp_per;
    break;
  }
  case JobType::kThief: {
    base_hp_per = 22;
    base_hp = 50 + 16 * 9 + (sf.level - 10) * base_hp_per;
    break;
  }
  default: {
    break;
  }
  }
  for (int i = 1; i < jobs.size(); i++) {
    switch (jobs[i]) {
    case JobType::kWarrior: {
      base_hp += 300;
      break;
    }
    case JobType::kMagician: {
      base_hp += 150;
      break;
    }
    case JobType::kBowman: {
      base_hp += 250;
      break;
    }
    case JobType::kThief: {
      base_hp += 250;
      break;
    }
    case JobType::kFighter: {
      break;
    }
    case JobType::kPage: {
      break;
    }
    case JobType::kSpearman: {
      break;
    }
    case JobType::kFpWizard: {
      break;
    }
    case JobType::kIlWizard: {
      break;
    }
    case JobType::kCleric: {
      break;
    }
    case JobType::kHunter: {
      base_hp += 250;
      break;
    }
    case JobType::kCrossbowman: {
      base_hp += 250;
      break;
    }
    case JobType::kAssassin: {
      base_hp += 250;
      break;
    }
    case JobType::kBandit: {
      base_hp += 250;
      break;
    }
    default: {
      break;
    }
    }
  }
  return base_hp + base_hp_per * hp_ap;
}

uint32_t CharacterStatGameInstance::GetBaseMpMax() {
  int base_mp_per = 0;
  int base_mp = 0;
  auto &sf = CharacterGameInstance::self;
  auto sf_job = sf.job;
  auto jobs = JobSkillGameInstance::LoadSkiTree(sf_job);
  if (jobs.size() == 1) {
    // beginner
    base_mp_per = 12;
    base_mp = 50 + (sf.level - 1) * base_mp_per;
    return base_mp + base_mp_per * mp_ap;
  }
  auto job = jobs[1];
  switch (job) {
  case JobType::kWarrior: {
    base_mp_per = 28;
    base_mp = 50 + 16 * 9 + (sf.level - 10) * base_mp_per;
    break;
  }
  case JobType::kMagician: {
    base_mp_per = 16;
    base_mp = 50 + 16 * 7 + (sf.level - 8) * base_mp_per;
    break;
  }
  case JobType::kBowman: {
    base_mp_per = 22;
    base_mp = 50 + 16 * 9 + (sf.level - 10) * base_mp_per;
    break;
  }
  case JobType::kThief: {
    base_mp_per = 22;
    base_mp = 50 + 16 * 9 + (sf.level - 10) * base_mp_per;
    break;
  }
  default: {
    break;
  }
  }
  for (int i = 1; i < jobs.size(); i++) {
    switch (jobs[i]) {
    case JobType::kWarrior: {
      base_mp += 150;
      break;
    }
    case JobType::kMagician: {
      base_mp += 350;
      break;
    }
    case JobType::kBowman: {
      base_mp += 250;
      break;
    }
    case JobType::kThief: {
      base_mp += 250;
      break;
    }
    case JobType::kFighter: {
      break;
    }
    case JobType::kPage: {
      break;
    }
    case JobType::kSpearman: {
      break;
    }
    case JobType::kFpWizard: {
      break;
    }
    case JobType::kIlWizard: {
      break;
    }
    case JobType::kCleric: {
      break;
    }
    case JobType::kHunter: {
      base_mp += 250;
      break;
    }
    case JobType::kCrossbowman: {
      base_mp += 250;
      break;
    }
    case JobType::kAssassin: {
      base_mp += 250;
      break;
    }
    case JobType::kBandit: {
      base_mp += 250;
      break;
    }
    default: {
      break;
    }
    }
  }
  return base_mp + base_mp_per * mp_ap;
}

// ============================================================
// 主 / 副属性
// ============================================================
int CharacterStatGameInstance::LoadPrimaryStat() {
  auto job = CharacterGameInstance::self.job;
  auto jobs = JobSkillGameInstance::LoadSkiTree(job);
  if (jobs.size() >= 2) {
    switch (jobs[1]) {
    case JobType::kBeginner:
    case JobType::kWarrior:
      return static_cast<int>(str_ap + eqp_str + itm_str + ski_str);
    case JobType::kMagician:
      return static_cast<int>(int_ap + eqp_int + itm_int + ski_int);
    case JobType::kBowman:
      return static_cast<int>(dex_ap + eqp_dex + itm_dex + ski_dex);
    case JobType::kThief:
      return static_cast<int>(luk_ap + eqp_luk + itm_luk + ski_luk);
    default:
      break;
    }
  }
  return static_cast<int>(str_ap + eqp_str + itm_str + ski_str);
}

int CharacterStatGameInstance::LoadSecondaryStat() {
  auto job = CharacterGameInstance::self.job;
  auto jobs = JobSkillGameInstance::LoadSkiTree(job);
  if (jobs.size() >= 2) {
    switch (jobs[1]) {
    case JobType::kWarrior:
      return static_cast<int>(dex_ap + eqp_dex + itm_dex + ski_dex);
    case JobType::kMagician:
      return static_cast<int>(luk_ap + eqp_luk + itm_luk + ski_luk);
    case JobType::kBowman:
      return static_cast<int>(str_ap + eqp_str + itm_str + ski_str);
    case JobType::kThief: // 指南：匕首/拳套的 Secondary = STR + DEX
      return static_cast<int>(dex_ap + eqp_dex + itm_dex + ski_dex);
    default:
      break;
    }
  }
  return static_cast<int>(dex_ap + eqp_dex + itm_dex + ski_dex);
}

void CharacterStatGameInstance::UpdateSfSkillStat() {
  ski_hp = 0;
  ski_mp = 0;
  ski_str = 0;
  ski_dex = 0;
  ski_int = 0;
  ski_luk = 0;
  ski_pro = 0; // 熟练度

  ski_accuracy = 0; // 命中
  ski_avoid = 0;    // 回避
  ski_exp = 0;

  ski_attack_def = 0;
  ski_magic_def = 0;

  auto job_id = CharacterGameInstance::self.job;
  auto ski_tree = JobSkillGameInstance::LoadSkiTree(job_id);
  auto &Skis = SkillGameInstance::Skis();
  for (auto ski_t : ski_tree) {
    auto job_skis = JobSkillGameInstance::LoadJobSkis(ski_t);
    for (auto [k, v] : job_skis) {
      auto ski_lv = JobSkillGameInstance::LoadSkiLevel(k);
      if (ski_lv == 0) {
        continue;
      }
      if (!Skis.contains(k)) {
        continue;
      }
      auto &ski = Skis.at(k);
      if (ski.passive) {
        ski.passive(ski_lv);
      }
    }
  }
}

void CharacterStatGameInstance::UpdateSfEquipStat() {
  eqp_hp = 0;
  eqp_mp = 0;
  eqp_str = 0;
  eqp_dex = 0;
  eqp_int = 0;
  eqp_luk = 0;
  eqp_pro = 0; // 熟练度

  eqp_accuracy = 0; // 命中
  eqp_avoid = 0;    // 回避

  eqp_attack = 0; // pad
  eqp_magic = 0;

  eqp_attack_def = 0;
  eqp_magic_def = 0;

  eqp_exp = 0;

  const auto &sf = CharacterGameInstance::self;

  // 单个来源（装备本体 / 已成功卷轴）的固定加成累加
  auto add_inc =
      [&](const std::flat_map<EquipGameInstance::IncType, int> &inc) {
        for (auto [t, v] : inc) {
          switch (t) {
          case EquipGameInstance::IncType::kWeaponPad:
            eqp_attack += static_cast<uint32_t>(v);
            break;
          case EquipGameInstance::IncType::kWeaponMad:
            eqp_magic += static_cast<uint32_t>(v);
            break;
          case EquipGameInstance::IncType::kStr:
            eqp_str += v;
            break;
          case EquipGameInstance::IncType::kDex:
            eqp_dex += v;
            break;
          case EquipGameInstance::IncType::kInt:
            eqp_int += v;
            break;
          case EquipGameInstance::IncType::kLuk:
            eqp_luk += v;
            break;
          case EquipGameInstance::IncType::kPdd:
            eqp_attack_def += v;
            break;
          case EquipGameInstance::IncType::kAcc:
            eqp_accuracy += static_cast<uint32_t>(v);
            break;
          default: // 攻速/CRT 等暂不进入面板数值
            break;
          }
        }
      };

  // 遍历所有已穿戴装备：本体 inc + 每个成功卷轴的 inc
  for (const auto &opt : {sf.weapon, sf.cap, sf.cape, sf.coat, sf.glove,
                          sf.longcoat, sf.pant, sf.shoes, sf.shield, sf.ring0,
                          sf.ring1, sf.ring2, sf.ring3, sf.accessory}) {
    if (!opt.has_value()) {
      continue;
    }
    const auto &eq = opt.value();
    add_inc(EquipGameInstance::LoadEquipInc(eq.id));
    for (const auto &scroll : eq.scroll) {
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

  // ---- 总属性（AP + 装备 + 道具/Buff + 技能被动）----
  str_point = str_ap + eqp_str + itm_str + ski_str;
  dex_point = dex_ap + eqp_dex + itm_dex + ski_dex;
  int_point = int_ap + eqp_int + itm_int + ski_int;
  luk_point = luk_ap + eqp_luk + itm_luk + ski_luk;

  // ---- HP / MP 上限 ----
  hp_point_max = static_cast<int32_t>(std::clamp<int64_t>(
      static_cast<int64_t>(GetBaseHpMax()) + eqp_hp + itm_hp + ski_hp, 1,
      INT32_MAX));
  mp_point_max = static_cast<int32_t>(std::clamp<int64_t>(
      static_cast<int64_t>(GetBaseMpMax()) + eqp_mp + itm_mp + ski_mp, 1,
      INT32_MAX));
  hp_point = std::min<int32_t>(hp_point, hp_point_max);
  mp_point = std::min<int32_t>(mp_point, mp_point_max);

  // ---- 主属性 / 副属性（按职业树推导）----
  const auto primary_ap = LoadPrimaryStat();
  const auto secondary_ap = LoadSecondaryStat();

  // ---- 攻击力 ----
  // AttackPower：参与最终乘法的攻击力（装备 pad + 道具/Buff pad）
  const uint32_t atk_power = eqp_attack + itm_attack;
  // 魔法攻击力（法师公式的 MagicTotal 组成项）
  const uint32_t mad_power = eqp_magic + itm_magic;

  // ---- 武器动作倍率 W（MeowDB：普通动作包络，MIN 取最低、MAX 取最高）----
  auto [weapon_mult_min, weapon_mult_max] = []() -> std::pair<float, float> {
    if (!CharacterGameInstance::self.weapon.has_value()) {
      return {1.0f, 1.0f};
    }
    switch (EquipGameInstance::LoadWeaponEnum(
        CharacterGameInstance::self.weapon->id)) {
    case EquipGameInstance::WeaponEnum::kSword1H: // 单手剑
      return {1.8f, 1.8f};                           // swing/stab 1.8
    case EquipGameInstance::WeaponEnum::kAxe1H:   // 单手斧
    case EquipGameInstance::WeaponEnum::kMace1H:  // 单手钝器
      return {1.2f, 2.4f}; // stab 1.2 ~ swing 2.4（mixed 1.8 在包络内）
    case EquipGameInstance::WeaponEnum::kDagger:   // 匕首
      return {1.0f, 2.0f};                           // swing 1.0 ~ stab 2.0
    case EquipGameInstance::WeaponEnum::kSword2H: // 双手剑
      return {2.5f, 2.5f};
    case EquipGameInstance::WeaponEnum::kAxe2H:  // 双手斧
    case EquipGameInstance::WeaponEnum::kMace2H: // 双手钝器
      return {2.0f, 3.0f};                          // stab 2.0 ~ swing 3.0
    case EquipGameInstance::WeaponEnum::kSpear: // 长枪：swing 1.5 / stab 3.5
    case EquipGameInstance::WeaponEnum::kPolearm: // 矛：swing 3.5 / stab 1.5
      return {1.5f, 3.5f};
    case EquipGameInstance::WeaponEnum::kBow:      // 弓
    case EquipGameInstance::WeaponEnum::kCrossbow: // 弩
    case EquipGameInstance::WeaponEnum::kClaw:     // 拳套
      return {1.0f, 2.5f};                           // 近身 1.0 ~ 射击 2.5
    case EquipGameInstance::WeaponEnum::kWand:     // 短杖物理敲击
    case EquipGameInstance::WeaponEnum::kStaff:    // 长杖物理敲击
      return {1.8f, 1.8f};
    default: // KNUCKLE/GUN 等指南未收录，占位
      return {1.0f, 1.0f};
    }
  }();

  // ---- 熟练度：m =（MasteryLevel / 10 + 0.1）× 0.7（物理/魔法共用）----
  const int mastery_lv =
      static_cast<int>(std::clamp<int64_t>(ski_pro + eqp_pro + itm_pro, 0, 10));
  const float mastery = (static_cast<float>(mastery_lv) / 10.0f + 0.1f) * 0.7f;

  // ---- 物理武器攻击伤害（MeowDB「Physical / Weapon attacks」）----
  // K = SkillDamage / 100（面板按 K=1 显示）
  // MIN = K × TotalWATK × (0.8 + (P×m×Wmin + Q)/StatDiv + AttackPower/APDiv)
  // MAX = K × TotalWATK × (1.0 + (P×Wmax + Q)/StatDiv + AttackPower/APDiv)
  // 普通近战/射击：StatDiv = 100，APDiv = 50
  // TotalWATK = 全部装备提供的武器攻击力（eqp_attack）
  // AttackPower = 道具/Buff 提供的攻击力加成（itm_attack）
  constexpr float stat_div = 100.0f;
  constexpr float ap_div = 50.0f;
  const float total_watk = static_cast<float>(eqp_attack);
  const float attack_power = static_cast<float>(itm_attack);
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
  const uint32_t magic_total =
      static_cast<uint32_t>(int_point / 2) + eqp_magic + itm_magic;
  const float mage_max = static_cast<float>(magic_total) *
                         (1.0f + static_cast<float>(int_point) / 125.0f);
  const float mage_min =
      static_cast<float>(magic_total) *
      (1.0f + static_cast<float>(int_point) * mastery / 125.0f);

  const auto jobs =
      JobSkillGameInstance::LoadSkiTree(CharacterGameInstance::self.job);
  const bool is_magician = jobs.size() >= 2 && jobs[1] == JobType::kMagician;

  // 面板取整按指南口径 trunc（直接截断，而非四舍五入）
  if (is_magician) {
    max_atk = static_cast<uint32_t>(mage_max);
    min_atk = static_cast<uint32_t>(mage_min);
  } else {
    max_atk = static_cast<uint32_t>(phys_max);
    min_atk = static_cast<uint32_t>(phys_min);
  }
  // 防御下限，避免 min > max 的显示异常
  if (min_atk > max_atk) {
    min_atk = max_atk;
  }

  // ---- 攻击力 / 魔法攻击力（界面展示值）----
  attack = static_cast<int32_t>(atk_power);
  magic = static_cast<int32_t>(mad_power);

  // ---- 命中 / 回避 ----
  accuracy = ski_accuracy + eqp_accuracy + itm_accuracy;
  avoid = ski_avoid + eqp_avoid + itm_avoid;

  // ---- 防御 ----
  attack_def = ski_attack_def + eqp_attack_def + itm_attack_def;
  magic_def = ski_magic_def + eqp_magic_def + itm_magic_def;

  // ---- 暴击 ----
  // 基础必杀伤害 150%，暴击率默认 0；技能/道具通过 set_crit_rate /
  // set_crit_damage 累加。这里不再每次重置（旧实现按"副属性/2"重置无公式
  // 依据，且会把 buff 加成冲掉）。
  if (crit_damage == 0) {
    crit_damage = 150;
  }
}

// ============================================================
// upgrade / exp
// ============================================================
bool CharacterStatGameInstance::Upgrade() {
  if (exp_point >= exp_point_max) {
    exp_point -= exp_point_max;
    auto &sf = CharacterGameInstance::self;
    sf.level += 1;

    remain_ap += 5;
    auto job = JobSkillGameInstance::LoadSkiTree(sf.job);

    if (job.size() == 1) {
      JobSkillGameInstance::remain_point[0] += 1;
    } else {
      JobSkillGameInstance::remain_point[job.size() - 1] += 3;
    }

    ClientCharacterLvUpT ccl;
    ccl.map_id = SceneSystemInstance::map_id;
    ClientRequest::send_to_host(ccl);

    ServerCharacterInstance::HandleLvUp(sf);
    Update();
    return true;
  }
  return false;
}

void CharacterStatGameInstance::AddExp(uint32_t num) {
  if (num == 0)
    return;
  exp_point += num;
  Upgrade();
  Update();
}

// ============================================================
// HP / MP
// ============================================================
void CharacterStatGameInstance::SetHp(int32_t n) {
  Update();
  hp_point = std::min<int32_t>(n, hp_point_max);
  if (hp_point < 0)
    hp_point = 0;
}

void CharacterStatGameInstance::SetMp(int32_t n) {
  Update();
  mp_point = std::min<int32_t>(n, mp_point_max);
  if (mp_point < 0)
    mp_point = 0;
}

void CharacterStatGameInstance::SetCritRate(uint32_t n) { crit_rate = n; }
void CharacterStatGameInstance::SetCritDamage(uint32_t n) {
  crit_damage = n;
}

int32_t CharacterStatGameInstance::GetHp() { return hp_point; }
int32_t CharacterStatGameInstance::GetMp() { return mp_point; }
int32_t CharacterStatGameInstance::GetHpMax() { return hp_point_max; }
int32_t CharacterStatGameInstance::GetMpMax() { return mp_point_max; }
uint64_t CharacterStatGameInstance::GetExp() { return exp_point; }

uint64_t CharacterStatGameInstance::GetExpMax() {
  static std::flat_map<int, int> cache;
  auto &sf = CharacterGameInstance::self;
  if (!cache.contains(sf.level)) {
    auto exp_node = WzResource::ms->get_root()->find(u"LevelExp.img");
    exp_node = exp_node->get_child(std::to_string(sf.level));
    if (exp_node == nullptr) {
      return UINT64_MAX;
    }
    exp_point_max = static_cast<wz::Property<int> *>(exp_node)->get();
    cache[sf.level] = exp_point_max;
  }
  exp_point_max = cache.at(sf.level);
  return exp_point_max;
}

// ============================================================
// 剩余 AP
// ============================================================
uint32_t CharacterStatGameInstance::GetRemainAp() { return remain_ap; }

void CharacterStatGameInstance::SetRemainAp(uint32_t n) { remain_ap = n; }

// ============================================================
// AP 加点
// ============================================================
void CharacterStatGameInstance::SetStrAp(uint32_t n) {
  str_ap = n;
  Update();
}
uint32_t CharacterStatGameInstance::GetStrAp() { return str_ap; }

void CharacterStatGameInstance::SetDexAp(uint32_t n) {
  dex_ap = n;
  Update();
}
uint32_t CharacterStatGameInstance::GetDexAp() { return dex_ap; }

void CharacterStatGameInstance::SetIntAp(uint32_t n) {
  int_ap = n;
  Update();
}
uint32_t CharacterStatGameInstance::GetIntAp() {
  return int_ap;
} // 修复：原返回 dex_ap

void CharacterStatGameInstance::SetLukAp(uint32_t n) {
  luk_ap = n;
  Update();
}
uint32_t CharacterStatGameInstance::GetLukAp() { return luk_ap; }

void CharacterStatGameInstance::SetHpAp(uint32_t n) {
  hp_ap = n;
  Update();
}
uint32_t CharacterStatGameInstance::GetHpAp() { return hp_ap; }

void CharacterStatGameInstance::SetMpAp(uint32_t n) {
  mp_ap = n;
  Update();
}
uint32_t CharacterStatGameInstance::GetMpAp() { return mp_ap; }

// ============================================================
// 主属性 getter
// ============================================================
uint32_t CharacterStatGameInstance::GetStrPoint() { return str_point; }
uint32_t CharacterStatGameInstance::GetDexPoint() { return dex_point; }
uint32_t CharacterStatGameInstance::GetIntPoint() { return int_point; }
uint32_t CharacterStatGameInstance::GetLukPoint() { return luk_point; }

// ============================================================
// 最终战斗属性 getter
// ============================================================
uint32_t CharacterStatGameInstance::GetMinAtk() { return min_atk; }
uint32_t CharacterStatGameInstance::GetMaxAtk() { return max_atk; }
uint32_t CharacterStatGameInstance::GetCritRate() { return crit_rate; }
uint32_t CharacterStatGameInstance::GetCritDamage() { return crit_damage; }
uint32_t CharacterStatGameInstance::GetAcc() { return accuracy; }
uint32_t CharacterStatGameInstance::GetAvd() { return avoid; }
int32_t CharacterStatGameInstance::GetDef() { return attack_def; }
int32_t CharacterStatGameInstance::GetMdef() { return magic_def; }
int32_t CharacterStatGameInstance::GetAttack() { return attack; }
int32_t CharacterStatGameInstance::GetMagic() { return magic; }

// ============================================================
// 技能加成 setter (ski_)
// ============================================================
void CharacterStatGameInstance::SetSkiHp(int64_t n) {
  ski_hp = n;
  Update();
}
void CharacterStatGameInstance::SetSkiMp(int64_t n) {
  ski_mp = n;
  Update();
}
void CharacterStatGameInstance::SetSkiStr(int64_t n) {
  ski_str = n;
  Update();
}
void CharacterStatGameInstance::SetSkiDex(int64_t n) {
  ski_dex = n;
  Update();
}
void CharacterStatGameInstance::SetSkiInt(int64_t n) {
  ski_int = n;
  Update();
}
void CharacterStatGameInstance::SetSkiLuk(int64_t n) {
  ski_luk = n;
  Update();
}
void CharacterStatGameInstance::SetSkiPro(int64_t n) {
  ski_pro = n;
  Update();
}
void CharacterStatGameInstance::SetSkiAcc(uint32_t n) {
  ski_accuracy = n;
  Update();
}
void CharacterStatGameInstance::SetSkiAvd(uint32_t n) {
  ski_avoid = n;
  Update();
}
void CharacterStatGameInstance::SetSkiDef(int32_t n) {
  ski_attack_def = n;
  Update();
}
void CharacterStatGameInstance::SetSkiMdef(int32_t n) {
  ski_magic_def = n;
  Update();
} // 修复：原写 ski_attack_def
void CharacterStatGameInstance::SetSkiExp(uint32_t n) {
  ski_exp = n;
  Update();
}

// ============================================================
// 技能加成 getter (ski_)
// ============================================================
int64_t CharacterStatGameInstance::GetSkiHp() { return ski_hp; }
int64_t CharacterStatGameInstance::GetSkiMp() { return ski_mp; }
int64_t CharacterStatGameInstance::GetSkiStr() { return ski_str; }
int64_t CharacterStatGameInstance::GetSkiDex() { return ski_dex; }
int64_t CharacterStatGameInstance::GetSkiInt() { return ski_int; }
int64_t CharacterStatGameInstance::GetSkiLuk() { return ski_luk; }
int64_t CharacterStatGameInstance::GetSkiPro() { return ski_pro; }
uint32_t CharacterStatGameInstance::GetSkiAcc() { return ski_accuracy; }
uint32_t CharacterStatGameInstance::GetSkiAvd() { return ski_avoid; }
int32_t CharacterStatGameInstance::GetSkiDef() { return ski_attack_def; }
int32_t CharacterStatGameInstance::GetSkiMdef() { return ski_magic_def; }
uint32_t CharacterStatGameInstance::GetSkiExp() { return ski_exp; }

// ============================================================
// 装备加成 setter (eqp_)
// ============================================================
void CharacterStatGameInstance::SetEqpHp(int64_t n) {
  eqp_hp = n;
  Update();
}
void CharacterStatGameInstance::SetEqpMp(int64_t n) {
  eqp_mp = n;
  Update();
}
void CharacterStatGameInstance::SetEqpStr(int64_t n) {
  eqp_str = n;
  Update();
}
void CharacterStatGameInstance::SetEqpDex(int64_t n) {
  eqp_dex = n;
  Update();
}
void CharacterStatGameInstance::SetEqpInt(int64_t n) {
  eqp_int = n;
  Update();
}
void CharacterStatGameInstance::SetEqpLuk(int64_t n) {
  eqp_luk = n;
  Update();
}
void CharacterStatGameInstance::SetEqpPro(int64_t n) {
  eqp_pro = n;
  Update();
}
void CharacterStatGameInstance::SetEqpAcc(uint32_t n) {
  eqp_accuracy = n;
  Update();
}
void CharacterStatGameInstance::SetEqpAvd(uint32_t n) {
  eqp_avoid = n;
  Update();
}
void CharacterStatGameInstance::SetEqpDef(int32_t n) {
  eqp_attack_def = n;
  Update();
}
void CharacterStatGameInstance::SetEqpMdef(int32_t n) {
  eqp_magic_def = n;
  Update();
}
void CharacterStatGameInstance::SetEqpExp(uint32_t n) {
  eqp_exp = n;
  Update();
}
void CharacterStatGameInstance::SetEqpAttack(uint32_t n) {
  eqp_attack = n;
  Update();
}
void CharacterStatGameInstance::SetEqpMagic(uint32_t n) {
  eqp_magic = n;
  Update();
}
// ============================================================
// 装备加成 getter (eqp_)
// ============================================================
int64_t CharacterStatGameInstance::GetEqpHp() { return eqp_hp; }
int64_t CharacterStatGameInstance::GetEqpMp() { return eqp_mp; }
int64_t CharacterStatGameInstance::GetEqpStr() { return eqp_str; }
int64_t CharacterStatGameInstance::GetEqpDex() { return eqp_dex; }
int64_t CharacterStatGameInstance::GetEqpInt() { return eqp_int; }
int64_t CharacterStatGameInstance::GetEqpLuk() { return eqp_luk; }
int64_t CharacterStatGameInstance::GetEqpPro() { return eqp_pro; }
uint32_t CharacterStatGameInstance::GetEqpAcc() { return eqp_accuracy; }
uint32_t CharacterStatGameInstance::GetEqpAvd() { return eqp_avoid; }
int32_t CharacterStatGameInstance::GetEqpDef() { return eqp_attack_def; }
int32_t CharacterStatGameInstance::GetEqpMdef() { return eqp_magic_def; }
uint32_t CharacterStatGameInstance::GetEqpExp() { return eqp_exp; }

// ============================================================
// 道具加成 setter (itm_)
// ============================================================
void CharacterStatGameInstance::SetItmHp(int64_t n) {
  itm_hp = n;
  Update();
}
void CharacterStatGameInstance::SetItmMp(int64_t n) {
  itm_mp = n;
  Update();
}
void CharacterStatGameInstance::SetItmStr(int64_t n) {
  itm_str = n;
  Update();
}
void CharacterStatGameInstance::SetItmDex(int64_t n) {
  itm_dex = n;
  Update();
}
void CharacterStatGameInstance::SetItmInt(int64_t n) {
  itm_int = n;
  Update();
}
void CharacterStatGameInstance::SetItmLuk(int64_t n) {
  itm_luk = n;
  Update();
}
void CharacterStatGameInstance::SetItmPro(int64_t n) {
  itm_pro = n;
  Update();
}
void CharacterStatGameInstance::SetItmAcc(uint32_t n) {
  itm_accuracy = n;
  Update();
}
void CharacterStatGameInstance::SetItmAvd(uint32_t n) {
  itm_avoid = n;
  Update();
}
void CharacterStatGameInstance::SetItmDef(int32_t n) {
  itm_attack_def = n;
  Update();
}
void CharacterStatGameInstance::SetItmMdef(int32_t n) {
  itm_magic_def = n;
  Update();
}
void CharacterStatGameInstance::SetItmExp(uint32_t n) {
  itm_exp = n;
  Update();
}

void CharacterStatGameInstance::SetItmAttack(uint32_t n) {
  itm_attack = n;
  Update();
}
void CharacterStatGameInstance::SetItmMagic(uint32_t n) {
  itm_magic = n;
  Update();
}

// ============================================================
// 道具加成 getter (itm_)
// ============================================================
int64_t CharacterStatGameInstance::GetItmHp() { return itm_hp; }
int64_t CharacterStatGameInstance::GetItmMp() { return itm_mp; }
int64_t CharacterStatGameInstance::GetItmStr() { return itm_str; }
int64_t CharacterStatGameInstance::GetItmDex() { return itm_dex; }
int64_t CharacterStatGameInstance::GetItmInt() { return itm_int; }
int64_t CharacterStatGameInstance::GetItmLuk() { return itm_luk; }
int64_t CharacterStatGameInstance::GetItmPro() { return itm_pro; }
uint32_t CharacterStatGameInstance::GetItmAcc() { return itm_accuracy; }
uint32_t CharacterStatGameInstance::GetItmAvd() { return itm_avoid; }
uint32_t CharacterStatGameInstance::GetItmAttack() { return itm_attack; }
uint32_t CharacterStatGameInstance::GetItmMagic() { return itm_magic; }
uint32_t CharacterStatGameInstance::GetItmExp() { return itm_exp; }
int32_t CharacterStatGameInstance::GetItmDef() { return itm_attack_def; }
int32_t CharacterStatGameInstance::GetItmMdef() { return itm_magic_def; }