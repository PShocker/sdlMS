#include "character_stat_game_instance.h"
#include "character_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
#include <algorithm>
#include <cstdint>

// ============================================================
// load
// ============================================================
void character_stat_game_instance::load(const character_save &cs) {
  str_point = cs.ap.str_ap;
  dex_point = cs.ap.dex_ap;
  int_point = cs.ap.int_ap;
  luk_point = cs.ap.luk_ap;

  hp_point = 20;
  hp_point_max = 100;

  mp_point = 20;
  mp_point_max = 100;

  exp_point = 50;
  exp_point_max = 100;

  update();
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
    case job_type::THIEF:
      return static_cast<int>(dex_ap + eqp_dex + itm_dex + ski_dex);
    default:
      break;
    }
  }
  return static_cast<int>(dex_ap + eqp_dex + itm_dex + ski_dex);
}

// ============================================================
// update
// ============================================================
void character_stat_game_instance::update() {
  auto primary_ap = load_primary_stat();
  auto secondary_ap = load_secondary_stat();

  // 攻击力
  min_atk = static_cast<uint32_t>(primary_ap);
  max_atk = static_cast<uint32_t>(primary_ap + secondary_ap / 4);

  // 物理 / 魔法攻击
  attack = static_cast<int32_t>(min_atk + eqp_attack + itm_attack);
  magic = static_cast<int32_t>(int_point + eqp_magic + itm_magic);

  // 命中 / 回避
  accuracy = ski_accuracy + eqp_accuracy + itm_accuracy;
  avoid = ski_avoid + eqp_avoid + itm_avoid;

  // 防御
  attack_def = ski_attack_def + eqp_attack_def + itm_attack_def;
  magic_def = ski_magic_def + eqp_magic_def + itm_magic_def;

  // 暴击
  crit_rate = static_cast<uint32_t>(secondary_ap / 2);
  crit_damage = 150;
}

// ============================================================
// upgrade / exp
// ============================================================
bool character_stat_game_instance::upgrade() {
  if (exp_point >= exp_point_max) {
    exp_point -= exp_point_max;
    auto &sf = character_game_instance::self;
    ClientCharacterLvUpT ccl;
    ccl.map_id = scene_system_instance::map_id;
    client_request::send_to_host(ccl);
    server_character_instance::handle_lv_up(sf);
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