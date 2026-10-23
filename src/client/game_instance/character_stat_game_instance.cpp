#include "character_stat_game_instance.h"
#include "character_game_instance.h"
#include "equip_game_instance.h"
#include "skill_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
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
void character_stat_game_instance::load(const character_save &cs) {
  str_ap = cs.ap.str_ap;
  dex_ap = cs.ap.dex_ap;
  int_ap = cs.ap.int_ap;
  luk_ap = cs.ap.luk_ap;

  remain_ap = cs.remain_ap;

  update();
}

uint32_t character_stat_game_instance::get_base_hp_max() {
  int base_hp_per = 0;
  int base_hp = 0;
  auto &sf = character_game_instance::self;
  auto sf_job = sf.job;
  auto jobs = job_skill_game_instance::load_ski_tree(sf_job);
  if (jobs.size() == 1) {
    // beginner
    base_hp_per = 16;
    base_hp = 50 + (sf.level - 1) * base_hp_per;
    return base_hp + base_hp_per * hp_ap;
  }
  auto job = jobs[1];
  switch (job) {
  case job_type::WARRIOR: {
    base_hp_per = 28;
    base_hp = 50 + 16 * 9 + (sf.level - 10) * base_hp_per;
    break;
  }
  case job_type::MAGICIAN: {
    base_hp_per = 16;
    base_hp = 50 + 16 * 7 + (sf.level - 8) * base_hp_per;
    break;
  }
  case job_type::BOWMAN: {
    base_hp_per = 22;
    base_hp = 50 + 16 * 9 + (sf.level - 10) * base_hp_per;
    break;
  }
  case job_type::THIEF: {
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
    case job_type::WARRIOR: {
      base_hp += 300;
      break;
    }
    case job_type::MAGICIAN: {
      base_hp += 150;
      break;
    }
    case job_type::BOWMAN: {
      base_hp += 250;
      break;
    }
    case job_type::THIEF: {
      base_hp += 250;
      break;
    }
    case job_type::FIGHTER: {
      break;
    }
    case job_type::PAGE: {
      break;
    }
    case job_type::SPEARMAN: {
      break;
    }
    case job_type::FP_WIZARD: {
      break;
    }
    case job_type::IL_WIZARD: {
      break;
    }
    case job_type::CLERIC: {
      break;
    }
    case job_type::HUNTER: {
      base_hp += 250;
      break;
    }
    case job_type::CROSSBOWMAN: {
      base_hp += 250;
      break;
    }
    case job_type::ASSASSIN: {
      base_hp += 250;
      break;
    }
    case job_type::BANDIT: {
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

uint32_t character_stat_game_instance::get_base_mp_max() {
  int base_mp_per = 0;
  int base_mp = 0;
  auto &sf = character_game_instance::self;
  auto sf_job = sf.job;
  auto jobs = job_skill_game_instance::load_ski_tree(sf_job);
  if (jobs.size() == 1) {
    // beginner
    base_mp_per = 12;
    base_mp = 50 + (sf.level - 1) * base_mp_per;
    return base_mp + base_mp_per * mp_ap;
  }
  auto job = jobs[1];
  switch (job) {
  case job_type::WARRIOR: {
    base_mp_per = 28;
    base_mp = 50 + 16 * 9 + (sf.level - 10) * base_mp_per;
    break;
  }
  case job_type::MAGICIAN: {
    base_mp_per = 16;
    base_mp = 50 + 16 * 7 + (sf.level - 8) * base_mp_per;
    break;
  }
  case job_type::BOWMAN: {
    base_mp_per = 22;
    base_mp = 50 + 16 * 9 + (sf.level - 10) * base_mp_per;
    break;
  }
  case job_type::THIEF: {
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
    case job_type::WARRIOR: {
      base_mp += 150;
      break;
    }
    case job_type::MAGICIAN: {
      base_mp += 350;
      break;
    }
    case job_type::BOWMAN: {
      base_mp += 250;
      break;
    }
    case job_type::THIEF: {
      base_mp += 250;
      break;
    }
    case job_type::FIGHTER: {
      break;
    }
    case job_type::PAGE: {
      break;
    }
    case job_type::SPEARMAN: {
      break;
    }
    case job_type::FP_WIZARD: {
      break;
    }
    case job_type::IL_WIZARD: {
      break;
    }
    case job_type::CLERIC: {
      break;
    }
    case job_type::HUNTER: {
      base_mp += 250;
      break;
    }
    case job_type::CROSSBOWMAN: {
      base_mp += 250;
      break;
    }
    case job_type::ASSASSIN: {
      base_mp += 250;
      break;
    }
    case job_type::BANDIT: {
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
int character_stat_game_instance::load_primary_stat() {
  auto job = character_game_instance::self.job;
  auto jobs = job_skill_game_instance::load_ski_tree(job);
  if (jobs.size() >= 2) {
    switch (jobs[1]) {
    case job_type::WARRIOR:
      return static_cast<int>(str_ap + eqp_str + itm_str + ski_str);
    case job_type::MAGICIAN:
      return static_cast<int>(int_ap + eqp_int + itm_int + ski_int);
    case job_type::BOWMAN:
      return static_cast<int>(dex_ap + eqp_dex + itm_dex + ski_dex);
    case job_type::THIEF:
      return static_cast<int>(luk_ap + eqp_luk + itm_luk + ski_luk);
    default:
      break;
    }
  }
  return static_cast<int>(str_ap + eqp_str + itm_str + ski_str);
}

int character_stat_game_instance::load_secondary_stat() {
  auto job = character_game_instance::self.job;
  auto jobs = job_skill_game_instance::load_ski_tree(job);
  if (jobs.size() >= 2) {
    switch (jobs[1]) {
    case job_type::WARRIOR:
      return static_cast<int>(dex_ap + eqp_dex + itm_dex + ski_dex);
    case job_type::MAGICIAN:
      return static_cast<int>(luk_ap + eqp_luk + itm_luk + ski_luk);
    case job_type::BOWMAN:
      return static_cast<int>(str_ap + eqp_str + itm_str + ski_str);
    case job_type::THIEF: // 指南：匕首/拳套的 Secondary = STR + DEX
      return static_cast<int>(str_ap + eqp_str + itm_str + ski_str +
                              dex_ap + eqp_dex + itm_dex + ski_dex);
    default:
      break;
    }
  }
  return static_cast<int>(dex_ap + eqp_dex + itm_dex + ski_dex);
}

void character_stat_game_instance::update_sf_skill_stat() {
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

  auto job_id = character_game_instance::self.job;
  auto ski_tree = job_skill_game_instance::load_ski_tree(job_id);
  auto &skis = skill_game_instance::skis();
  for (auto ski_t : ski_tree) {
    auto job_skis = job_skill_game_instance::load_job_skis(ski_t);
    for (auto [k, v] : job_skis) {
      auto ski_lv = job_skill_game_instance::load_ski_level(k);
      if (ski_lv == 0) {
        continue;
      }
      if (!skis.contains(k)) {
        continue;
      }
      auto &ski = skis.at(k);
      if (ski.passive) {
        ski.passive(ski_lv);
      }
    }
  }
}

void character_stat_game_instance::update_sf_equip_stat() {
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

  const auto &sf = character_game_instance::self;

  // 单个来源（装备本体 / 已成功卷轴）的固定加成累加
  auto add_inc = [&](const std::flat_map<equip_game_instance::inc_type, int>
                         &inc) {
    for (auto [t, v] : inc) {
      switch (t) {
      case equip_game_instance::inc_type::WEAPON_PAD:
        eqp_attack += static_cast<uint32_t>(v);
        break;
      case equip_game_instance::inc_type::WEAPON_MAD:
        eqp_magic += static_cast<uint32_t>(v);
        break;
      case equip_game_instance::inc_type::STR:
        eqp_str += v;
        break;
      case equip_game_instance::inc_type::DEX:
        eqp_dex += v;
        break;
      case equip_game_instance::inc_type::INT:
        eqp_int += v;
        break;
      case equip_game_instance::inc_type::LUK:
        eqp_luk += v;
        break;
      case equip_game_instance::inc_type::PDD:
        eqp_attack_def += v;
        break;
      case equip_game_instance::inc_type::ACC:
        eqp_accuracy += static_cast<uint32_t>(v);
        break;
      default: // 攻速/CRT 等暂不进入面板数值
        break;
      }
    }
  };

  // 遍历所有已穿戴装备：本体 inc + 每个成功卷轴的 inc
  for (const auto &opt :
       {sf.weapon, sf.cap, sf.cape, sf.coat, sf.glove, sf.longcoat, sf.pant,
        sf.shoes, sf.shield, sf.ring0, sf.ring1, sf.ring2, sf.ring3,
        sf.accessory}) {
    if (!opt.has_value()) {
      continue;
    }
    const auto &eq = opt.value();
    add_inc(equip_game_instance::load_equip_inc(eq.id));
    for (const auto &scroll : eq.scroll) {
      if (!scroll.success) {
        continue;
      }
      add_inc(equip_game_instance::load_scroll_inc(scroll.id));
    }
  }
}

// ============================================================
// update
// ============================================================
void character_stat_game_instance::update() {
  update_sf_equip_stat();
  update_sf_skill_stat();

  // ---- 总属性（AP + 装备 + 道具/Buff + 技能被动）----
  str_point = str_ap + eqp_str + itm_str + ski_str;
  dex_point = dex_ap + eqp_dex + itm_dex + ski_dex;
  int_point = int_ap + eqp_int + itm_int + ski_int;
  luk_point = luk_ap + eqp_luk + itm_luk + ski_luk;

  // ---- HP / MP 上限 ----
  hp_point_max = static_cast<int32_t>(std::clamp<int64_t>(
      static_cast<int64_t>(get_base_hp_max()) + eqp_hp + itm_hp + ski_hp, 1,
      INT32_MAX));
  mp_point_max = static_cast<int32_t>(std::clamp<int64_t>(
      static_cast<int64_t>(get_base_mp_max()) + eqp_mp + itm_mp + ski_mp, 1,
      INT32_MAX));
  hp_point = std::min<int32_t>(hp_point, hp_point_max);
  mp_point = std::min<int32_t>(mp_point, mp_point_max);

  // ---- 主属性 / 副属性（按职业树推导）----
  const auto primary_ap = load_primary_stat();
  const auto secondary_ap = load_secondary_stat();

  // ---- 攻击力 ----
  // AttackPower：参与最终乘法的攻击力（装备 pad + 道具/Buff pad）
  const uint32_t atk_power = eqp_attack + itm_attack;
  // 魔法攻击力（法师公式的 MagicTotal 组成项）
  const uint32_t mad_power = eqp_magic + itm_magic;

  // ---- 武器动作倍率 W（MeowDB：普通动作包络，MIN 取最低、MAX 取最高）----
  auto [weapon_mult_min, weapon_mult_max] = []() -> std::pair<float, float> {
    if (!character_game_instance::self.weapon.has_value()) {
      return {1.0f, 1.0f};
    }
    switch (equip_game_instance::load_weapon_enum(
        character_game_instance::self.weapon->id)) {
    case equip_game_instance::weapon_enum::SWORD_1H: // 单手剑
      return {1.8f, 1.8f};                           // swing/stab 1.8
    case equip_game_instance::weapon_enum::AXE_1H:  // 单手斧
    case equip_game_instance::weapon_enum::MACE_1H: // 单手钝器
      return {1.2f, 2.4f}; // stab 1.2 ~ swing 2.4（mixed 1.8 在包络内）
    case equip_game_instance::weapon_enum::DAGGER: // 匕首
      return {1.0f, 2.0f};                         // swing 1.0 ~ stab 2.0
    case equip_game_instance::weapon_enum::SWORD_2H: // 双手剑
      return {2.5f, 2.5f};
    case equip_game_instance::weapon_enum::AXE_2H:  // 双手斧
    case equip_game_instance::weapon_enum::MACE_2H: // 双手钝器
      return {2.0f, 3.0f}; // stab 2.0 ~ swing 3.0
    case equip_game_instance::weapon_enum::SPEAR:   // 长枪：swing 1.5 / stab 3.5
    case equip_game_instance::weapon_enum::POLEARM: // 矛：swing 3.5 / stab 1.5
      return {1.5f, 3.5f};
    case equip_game_instance::weapon_enum::BOW:      // 弓
    case equip_game_instance::weapon_enum::CROSSBOW: // 弩
    case equip_game_instance::weapon_enum::CLAW:     // 拳套
      return {1.0f, 2.5f}; // 近身 1.0 ~ 射击 2.5
    case equip_game_instance::weapon_enum::WAND:  // 短杖物理敲击
    case equip_game_instance::weapon_enum::STAFF: // 长杖物理敲击
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
      total_watk *
      (1.0f + (static_cast<float>(primary_ap) * weapon_mult_max +
               static_cast<float>(secondary_ap)) /
                  stat_div +
       attack_power / ap_div);
  const float phys_min =
      total_watk *
      (0.8f + (static_cast<float>(primary_ap) * mastery * weapon_mult_min +
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
      job_skill_game_instance::load_ski_tree(character_game_instance::self.job);
  const bool is_magician = jobs.size() >= 2 && jobs[1] == job_type::MAGICIAN;

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
bool character_stat_game_instance::upgrade() {
  if (exp_point >= exp_point_max) {
    exp_point -= exp_point_max;

    remain_ap += 5;
    auto &sf = character_game_instance::self;
    auto job = job_skill_game_instance::load_ski_tree(sf.job);

    if (job.size() == 1) {
      job_skill_game_instance::remain_point[0] += 1;
    } else {
      job_skill_game_instance::remain_point[job.size() - 1] += 3;
    }

    ClientCharacterLvUpT ccl;
    ccl.map_id = scene_system_instance::map_id;
    client_request::send_to_host(ccl);

    server_character_instance::handle_lv_up(sf);
    update();
    return true;
  }
  return false;
}

void character_stat_game_instance::add_exp(uint32_t num) {
  if (num == 0)
    return;
  exp_point += num;
  upgrade();
  update();
}

// ============================================================
// HP / MP
// ============================================================
void character_stat_game_instance::set_hp(int32_t n) {
  update();
  hp_point = std::min<int32_t>(n, hp_point_max);
  if (hp_point < 0)
    hp_point = 0;
}

void character_stat_game_instance::set_mp(int32_t n) {
  update();
  mp_point = std::min<int32_t>(n, mp_point_max);
  if (mp_point < 0)
    mp_point = 0;
}

void character_stat_game_instance::set_crit_rate(uint32_t n) { crit_rate = n; }
void character_stat_game_instance::set_crit_damage(uint32_t n) {
  crit_damage = n;
}

int32_t character_stat_game_instance::get_hp() { return hp_point; }
int32_t character_stat_game_instance::get_mp() { return mp_point; }
int32_t character_stat_game_instance::get_hp_max() { return hp_point_max; }
int32_t character_stat_game_instance::get_mp_max() { return mp_point_max; }
uint32_t character_stat_game_instance::get_exp() { return exp_point; }
uint32_t character_stat_game_instance::get_exp_max() { return exp_point_max; }

// ============================================================
// 剩余 AP
// ============================================================
uint32_t character_stat_game_instance::get_remain_ap() { return remain_ap; }

void character_stat_game_instance::set_remain_ap(uint32_t n) { remain_ap = n; }

// ============================================================
// AP 加点
// ============================================================
void character_stat_game_instance::set_str_ap(uint32_t n) {
  str_ap = n;
  update();
}
uint32_t character_stat_game_instance::get_str_ap() { return str_ap; }

void character_stat_game_instance::set_dex_ap(uint32_t n) {
  dex_ap = n;
  update();
}
uint32_t character_stat_game_instance::get_dex_ap() { return dex_ap; }

void character_stat_game_instance::set_int_ap(uint32_t n) {
  int_ap = n;
  update();
}
uint32_t character_stat_game_instance::get_int_ap() {
  return int_ap;
} // 修复：原返回 dex_ap

void character_stat_game_instance::set_luk_ap(uint32_t n) {
  luk_ap = n;
  update();
}
uint32_t character_stat_game_instance::get_luk_ap() { return luk_ap; }

void character_stat_game_instance::set_hp_ap(uint32_t n) {
  hp_ap = n;
  update();
}
uint32_t character_stat_game_instance::get_hp_ap() { return hp_ap; }

void character_stat_game_instance::set_mp_ap(uint32_t n) {
  mp_ap = n;
  update();
}
uint32_t character_stat_game_instance::get_mp_ap() { return mp_ap; }

// ============================================================
// 主属性 getter
// ============================================================
uint32_t character_stat_game_instance::get_str_point() { return str_point; }
uint32_t character_stat_game_instance::get_dex_point() { return dex_point; }
uint32_t character_stat_game_instance::get_int_point() { return int_point; }
uint32_t character_stat_game_instance::get_luk_point() { return luk_point; }

// ============================================================
// 最终战斗属性 getter
// ============================================================
uint32_t character_stat_game_instance::get_min_atk() { return min_atk; }
uint32_t character_stat_game_instance::get_max_atk() { return max_atk; }
uint32_t character_stat_game_instance::get_crit_rate() { return crit_rate; }
uint32_t character_stat_game_instance::get_crit_damage() { return crit_damage; }
uint32_t character_stat_game_instance::get_acc() { return accuracy; }
uint32_t character_stat_game_instance::get_avd() { return avoid; }
int32_t character_stat_game_instance::get_def() { return attack_def; }
int32_t character_stat_game_instance::get_mdef() { return magic_def; }
int32_t character_stat_game_instance::get_attack() { return attack; }
int32_t character_stat_game_instance::get_magic() { return magic; }

// ============================================================
// 技能加成 setter (ski_)
// ============================================================
void character_stat_game_instance::set_ski_hp(int64_t n) {
  ski_hp = n;
  update();
}
void character_stat_game_instance::set_ski_mp(int64_t n) {
  ski_mp = n;
  update();
}
void character_stat_game_instance::set_ski_str(int64_t n) {
  ski_str = n;
  update();
}
void character_stat_game_instance::set_ski_dex(int64_t n) {
  ski_dex = n;
  update();
}
void character_stat_game_instance::set_ski_int(int64_t n) {
  ski_int = n;
  update();
}
void character_stat_game_instance::set_ski_luk(int64_t n) {
  ski_luk = n;
  update();
}
void character_stat_game_instance::set_ski_pro(int64_t n) {
  ski_pro = n;
  update();
}
void character_stat_game_instance::set_ski_acc(uint32_t n) {
  ski_accuracy = n;
  update();
}
void character_stat_game_instance::set_ski_avd(uint32_t n) {
  ski_avoid = n;
  update();
}
void character_stat_game_instance::set_ski_def(int32_t n) {
  ski_attack_def = n;
  update();
}
void character_stat_game_instance::set_ski_mdef(int32_t n) {
  ski_magic_def = n;
  update();
} // 修复：原写 ski_attack_def
void character_stat_game_instance::set_ski_exp(uint32_t n) {
  ski_exp = n;
  update();
}

// ============================================================
// 技能加成 getter (ski_)
// ============================================================
int64_t character_stat_game_instance::get_ski_hp() { return ski_hp; }
int64_t character_stat_game_instance::get_ski_mp() { return ski_mp; }
int64_t character_stat_game_instance::get_ski_str() { return ski_str; }
int64_t character_stat_game_instance::get_ski_dex() { return ski_dex; }
int64_t character_stat_game_instance::get_ski_int() { return ski_int; }
int64_t character_stat_game_instance::get_ski_luk() { return ski_luk; }
int64_t character_stat_game_instance::get_ski_pro() { return ski_pro; }
uint32_t character_stat_game_instance::get_ski_acc() { return ski_accuracy; }
uint32_t character_stat_game_instance::get_ski_avd() { return ski_avoid; }
int32_t character_stat_game_instance::get_ski_def() { return ski_attack_def; }
int32_t character_stat_game_instance::get_ski_mdef() { return ski_magic_def; }
uint32_t character_stat_game_instance::get_ski_exp() { return ski_exp; }

// ============================================================
// 装备加成 setter (eqp_)
// ============================================================
void character_stat_game_instance::set_eqp_hp(int64_t n) {
  eqp_hp = n;
  update();
}
void character_stat_game_instance::set_eqp_mp(int64_t n) {
  eqp_mp = n;
  update();
}
void character_stat_game_instance::set_eqp_str(int64_t n) {
  eqp_str = n;
  update();
}
void character_stat_game_instance::set_eqp_dex(int64_t n) {
  eqp_dex = n;
  update();
}
void character_stat_game_instance::set_eqp_int(int64_t n) {
  eqp_int = n;
  update();
}
void character_stat_game_instance::set_eqp_luk(int64_t n) {
  eqp_luk = n;
  update();
}
void character_stat_game_instance::set_eqp_pro(int64_t n) {
  eqp_pro = n;
  update();
}
void character_stat_game_instance::set_eqp_acc(uint32_t n) {
  eqp_accuracy = n;
  update();
}
void character_stat_game_instance::set_eqp_avd(uint32_t n) {
  eqp_avoid = n;
  update();
}
void character_stat_game_instance::set_eqp_def(int32_t n) {
  eqp_attack_def = n;
  update();
}
void character_stat_game_instance::set_eqp_mdef(int32_t n) {
  eqp_magic_def = n;
  update();
}
void character_stat_game_instance::set_eqp_exp(uint32_t n) {
  eqp_exp = n;
  update();
}
void character_stat_game_instance::set_eqp_attack(uint32_t n) {
  eqp_attack = n;
  update();
}
void character_stat_game_instance::set_eqp_magic(uint32_t n) {
  eqp_magic = n;
  update();
}
// ============================================================
// 装备加成 getter (eqp_)
// ============================================================
int64_t character_stat_game_instance::get_eqp_hp() { return eqp_hp; }
int64_t character_stat_game_instance::get_eqp_mp() { return eqp_mp; }
int64_t character_stat_game_instance::get_eqp_str() { return eqp_str; }
int64_t character_stat_game_instance::get_eqp_dex() { return eqp_dex; }
int64_t character_stat_game_instance::get_eqp_int() { return eqp_int; }
int64_t character_stat_game_instance::get_eqp_luk() { return eqp_luk; }
int64_t character_stat_game_instance::get_eqp_pro() { return eqp_pro; }
uint32_t character_stat_game_instance::get_eqp_acc() { return eqp_accuracy; }
uint32_t character_stat_game_instance::get_eqp_avd() { return eqp_avoid; }
int32_t character_stat_game_instance::get_eqp_def() { return eqp_attack_def; }
int32_t character_stat_game_instance::get_eqp_mdef() { return eqp_magic_def; }
uint32_t character_stat_game_instance::get_eqp_exp() { return eqp_exp; }

// ============================================================
// 道具加成 setter (itm_)
// ============================================================
void character_stat_game_instance::set_itm_hp(int64_t n) {
  itm_hp = n;
  update();
}
void character_stat_game_instance::set_itm_mp(int64_t n) {
  itm_mp = n;
  update();
}
void character_stat_game_instance::set_itm_str(int64_t n) {
  itm_str = n;
  update();
}
void character_stat_game_instance::set_itm_dex(int64_t n) {
  itm_dex = n;
  update();
}
void character_stat_game_instance::set_itm_int(int64_t n) {
  itm_int = n;
  update();
}
void character_stat_game_instance::set_itm_luk(int64_t n) {
  itm_luk = n;
  update();
}
void character_stat_game_instance::set_itm_pro(int64_t n) {
  itm_pro = n;
  update();
}
void character_stat_game_instance::set_itm_acc(uint32_t n) {
  itm_accuracy = n;
  update();
}
void character_stat_game_instance::set_itm_avd(uint32_t n) {
  itm_avoid = n;
  update();
}
void character_stat_game_instance::set_itm_def(int32_t n) {
  itm_attack_def = n;
  update();
}
void character_stat_game_instance::set_itm_mdef(int32_t n) {
  itm_magic_def = n;
  update();
}
void character_stat_game_instance::set_itm_exp(uint32_t n) {
  itm_exp = n;
  update();
}

void character_stat_game_instance::set_itm_attack(uint32_t n) {
  itm_attack = n;
  update();
}
void character_stat_game_instance::set_itm_magic(uint32_t n) {
  itm_magic = n;
  update();
}

// ============================================================
// 道具加成 getter (itm_)
// ============================================================
int64_t character_stat_game_instance::get_itm_hp() { return itm_hp; }
int64_t character_stat_game_instance::get_itm_mp() { return itm_mp; }
int64_t character_stat_game_instance::get_itm_str() { return itm_str; }
int64_t character_stat_game_instance::get_itm_dex() { return itm_dex; }
int64_t character_stat_game_instance::get_itm_int() { return itm_int; }
int64_t character_stat_game_instance::get_itm_luk() { return itm_luk; }
int64_t character_stat_game_instance::get_itm_pro() { return itm_pro; }
uint32_t character_stat_game_instance::get_itm_acc() { return itm_accuracy; }
uint32_t character_stat_game_instance::get_itm_avd() { return itm_avoid; }
uint32_t character_stat_game_instance::get_itm_attack() { return itm_attack; }
uint32_t character_stat_game_instance::get_itm_magic() { return itm_magic; }
uint32_t character_stat_game_instance::get_itm_exp() { return itm_exp; }
int32_t character_stat_game_instance::get_itm_def() { return itm_attack_def; }
int32_t character_stat_game_instance::get_itm_mdef() { return itm_magic_def; }