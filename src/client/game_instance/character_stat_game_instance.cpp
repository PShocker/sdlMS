#include "character_stat_game_instance.h"

void character_stat_game_instance::load(const character_save &cs) {
  str_point = cs.ap.str_ap;
  dex_point = cs.ap.dex_ap;
  int_point = cs.ap.int_ap;
  luk_point = cs.ap.luk_ap;

  hp_point = 100;
  hp_point_max = 100;

  mp_point = 20;
  mp_point_max = 100;

  exp_point = 50;
  exp_point_max = 100;
}

void character_stat_game_instance::hp_inc(uint64_t i) {
  exp_point += i;
  //
}

void character_stat_game_instance::mp_inc(uint64_t i) {
  exp_point += i;
  //
}

void character_stat_game_instance::str_inc(uint64_t i) {
  exp_point += i;
  //
}

void character_stat_game_instance::dex_inc(uint64_t i) {
  exp_point += i;
  //
}

void character_stat_game_instance::int_inc(uint64_t i) {
  exp_point += i;
  //
}

void character_stat_game_instance::luk_inc(uint64_t i) {
  exp_point += i;
  //
}