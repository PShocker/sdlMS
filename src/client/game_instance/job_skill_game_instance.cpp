#include "job_skill_game_instance.h"
#include "character_game_instance.h"
#include <cstdint>

void job_skill_game_instance::load(const character_save &cs) {
  skill_point = cs.sp.ski_sp;
}

uint8_t job_skill_game_instance::load_skill_level(const std::u16string &id) {
  auto ski_id = std::stoi(std::string(id.begin(), id.end()));
  if (skill_point.contains(ski_id)) {
    return skill_point.at(ski_id);
  } else {
    return 1;
  }
}

std::vector<job_type>
job_skill_game_instance::load_skill_tree(const std::u16string &id) {
  std::vector<job_type> r = {job_type::BEGINNER};
  // 千位是职业群，冒险家默认是0，省略，百位是职业群
  auto ch = id[id.size() - 3];  // 倒数第3位
  auto ch2 = id[id.size() - 2]; // 倒数第2位
  if (ch == u'0') {
    return r;
  }
  switch (ch) {
  case u'1': {
    r.push_back(job_type::WARRIOR);
    switch (ch2) {
    case u'1': {
      r.push_back(job_type::FIGHTER);
      break;
    }
    case u'2': {
      r.push_back(job_type::PAGE);
      break;
    }
    case u'3': {
      r.push_back(job_type::SPEARMAN);
      break;
    }
    }
    break;
  }
  case u'2': {
    r.push_back(job_type::MAGICIAN);
    switch (ch2) {
    case u'1': {
      r.push_back(job_type::FP_WIZARD);
      break;
    }
    case u'2': {
      r.push_back(job_type::IL_WIZARD);
      break;
    }
    case u'3': {
      r.push_back(job_type::CLERIC);
      break;
    }
    }
    break;
  }
  case u'3': {
    r.push_back(job_type::BOWMAN);
    switch (ch2) {
    case u'1': {
      r.push_back(job_type::HUNTER);
      break;
    }
    case u'2': {
      r.push_back(job_type::CROSSBOWMAN);
      break;
    }
    }
    break;
  }
  case u'4': {
    r.push_back(job_type::THIEF);
    switch (ch2) {
    case u'1': {
      r.push_back(job_type::ASSASSIN);
      break;
    }
    case u'2': {
      r.push_back(job_type::BANDIT);
      break;
    }
    }
    break;
  }
  }

  return r;
}