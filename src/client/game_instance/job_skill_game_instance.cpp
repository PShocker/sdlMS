#include "job_skill_game_instance.h"

void job_skill_game_instance::load_job() { self_job = 130; }

void job_skill_game_instance::load_skill_point() {
  self_skill_point = {
      {1300000, 1},
  };
}
void job_skill_game_instance::load() {
  load_job();
  load_skill_point();
  return;
}