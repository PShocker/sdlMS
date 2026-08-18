#include "character_stat_game_instance.h"
#include "character_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"

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