#include "character_stat_game_instance.h"
#include "character_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
#include <algorithm>
#include <cstdint>

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
}

int character_stat_game_instance::load_primary_stat() {
  auto job = character_game_instance::self.job;
  auto jobs = job_skill_game_instance::load_ski_tree(job);
  if (jobs.size() >= 2) {
    switch (jobs[1]) {
    case job_type::WARRIOR: {
      return str_ap + eqp_str + itm_str + ski_str;
      break;
    }
    case job_type::MAGICIAN: {
      return int_ap + eqp_int + itm_int + ski_int;
      break;
    }
    case job_type::BOWMAN: {
      return dex_ap + eqp_dex + itm_dex + ski_dex;
      break;
    }
    case job_type::THIEF: {
      return luk_ap + eqp_luk + itm_luk + ski_luk;
      break;
    }
    default: {
      break;
    }
    }
  } else {
    return str_ap + eqp_str + itm_str + ski_str;
  }
  return 0;
}

int character_stat_game_instance::load_secondary_stat() {
  auto job = character_game_instance::self.job;
  auto jobs = job_skill_game_instance::load_ski_tree(job);
  if (jobs.size() >= 2) {
    switch (jobs[1]) {
    case job_type::WARRIOR: {
      return dex_ap + eqp_dex + itm_dex + ski_dex;
      break;
    }
    case job_type::MAGICIAN: {
      return luk_ap + eqp_luk + itm_luk + ski_luk;
      break;
    }
    case job_type::BOWMAN: {
      return str_ap + eqp_str + itm_str + ski_str;
      break;
    }
    case job_type::THIEF: {
      return dex_ap + eqp_dex + itm_dex + ski_dex;
      break;
    }
    default: {
      break;
    }
    }
  } else {
    return dex_ap + eqp_dex + itm_dex + ski_dex;
  }
  return 0;
}

void character_stat_game_instance::update() {
  auto primary_ap = load_primary_stat();
  auto secondary_ap = load_secondary_stat();
  min_atk = str_point;
  max_atk = str_point + 1;

  hp_point_max = hp_ap + ski_hp + eqp_hp + itm_hp;
  mp_point_max = mp_ap + ski_mp + eqp_mp + itm_mp;

  avoid = ski_avoid + eqp_avoid;
  accuracy = ski_accuracy + itm_accuracy;
}

bool character_stat_game_instance::upgrade() {
  if (exp_point >= exp_point_max) {
    exp_point -= exp_point_max;
    // lv up effect
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
  if (num == 0) {
    return;
  }
  exp_point += num;
  upgrade();
}

void character_stat_game_instance::add_hp(int n) {
  hp_point += n;
  hp_point = std::min(hp_point, hp_point_max);
  update();
};

int32_t character_stat_game_instance::get_hp() { return hp_point; }

int32_t character_stat_game_instance::get_mp() { return mp_point; }

int32_t character_stat_game_instance::get_hp_max() { return hp_point_max; }

int32_t character_stat_game_instance::get_mp_max() { return mp_point_max; }

uint32_t character_stat_game_instance::get_exp() { return exp_point; }

uint32_t character_stat_game_instance::get_exp_max() { return exp_point_max; }

void character_stat_game_instance::add_mp(int n) {
  mp_point += n;
  mp_point = std::min(mp_point, mp_point_max);
  update();
};

void character_stat_game_instance::add_str_ap(int n) {
  str_ap += n;
  update();
};

uint32_t character_stat_game_instance::get_str_ap() { return str_ap; }

void character_stat_game_instance::add_dex_ap(int n) {
  dex_ap += n;
  update();
};

uint32_t character_stat_game_instance::get_dex_ap() { return dex_ap; }

void character_stat_game_instance::add_int_ap(int n) {
  int_ap += n;
  update();
};

uint32_t character_stat_game_instance::get_int_ap() { return dex_ap; }

void character_stat_game_instance::add_luk_ap(int n) {
  luk_ap += n;
  update();
};

uint32_t character_stat_game_instance::get_luk_ap() { return luk_ap; }

void character_stat_game_instance::add_hp_ap(int n) {
  hp_ap += n;
  update();
};

uint32_t character_stat_game_instance::get_hp_ap() { return hp_ap; }

void character_stat_game_instance::add_mp_ap(int n) {
  mp_ap += n;
  update();
};

uint32_t character_stat_game_instance::get_mp_ap() { return mp_ap; }

void character_stat_game_instance::add_ski_hp(int n) {
  ski_hp += n;
  update();
};

void character_stat_game_instance::add_ski_mp(int n) {
  ski_mp += n;
  update();
};

void character_stat_game_instance::add_ski_str(int n) {
  ski_str += n;
  update();
};

void character_stat_game_instance::add_ski_dex(int n) {
  ski_dex += n;
  update();
};

void character_stat_game_instance::add_ski_int(int n) {
  ski_int += n;
  update();
};

void character_stat_game_instance::add_ski_luk(int n) {
  ski_luk += n;
  update();
};

void character_stat_game_instance::add_ski_pro(int n) {
  ski_pro += n;
  update();
};

void character_stat_game_instance::add_ski_acc(int n) {
  ski_accuracy += n;
  update();
};

void character_stat_game_instance::add_ski_avd(int n) {
  ski_avoid += n;
  update();
};

void character_stat_game_instance::add_ski_def(int n) {
  ski_attack_def += n;
  update();
};

void character_stat_game_instance::add_ski_mdef(int n) {
  ski_attack_def += n;
  update();
};
void character_stat_game_instance::add_ski_exp(int n) {
  ski_exp += n;
  update();
};

void character_stat_game_instance::add_eqp_hp(int n) {
  eqp_hp += n;
  update();
};

void character_stat_game_instance::add_eqp_mp(int n) {};
void character_stat_game_instance::add_eqp_str(int n) {};
void character_stat_game_instance::add_eqp_dex(int n) {};
void character_stat_game_instance::add_eqp_int(int n) {};
void character_stat_game_instance::add_eqp_luk(int n) {};
void character_stat_game_instance::add_eqp_pro(int n) {};
void character_stat_game_instance::add_eqp_acc(int n) {};
void character_stat_game_instance::add_eqp_avd(int n) {};
void character_stat_game_instance::add_eqp_def(int n) {};
void character_stat_game_instance::add_eqp_mdef(int n) {};
void character_stat_game_instance::add_eqp_exp(int n) {};

void character_stat_game_instance::add_itm_hp(int n) {};
void character_stat_game_instance::add_itm_mp(int n) {};
void character_stat_game_instance::add_itm_str(int n) {};
void character_stat_game_instance::add_itm_dex(int n) {};
void character_stat_game_instance::add_itm_int(int n) {};
void character_stat_game_instance::add_itm_luk(int n) {};
void character_stat_game_instance::add_itm_pro(int n) {};
void character_stat_game_instance::add_itm_acc(int n) {};
void character_stat_game_instance::add_itm_avd(int n) {};
void character_stat_game_instance::add_itm_def(int n) {};
void character_stat_game_instance::add_itm_mdef(int n) {};
void character_stat_game_instance::add_itm_exp(int n) {};