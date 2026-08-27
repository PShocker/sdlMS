#include "job_skill_game_instance.h"
#include "character_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include <cstdint>
#include <string>

void job_skill_game_instance::load_passive_ski() {
  auto &self = character_game_instance::self;
  auto job_id = self.job;
  auto jobs = load_ski_tree(job_id);
  auto &skis = skill_game_instance::skis();
  for (auto jo : jobs) {
    auto job_ski = load_job_skis(jo);
    for (auto [k, v] : job_ski) {
      auto &ski = skis.at(k);
      if (ski.passive) {
        auto ski_lv = load_ski_level(k);
        ski.passive(ski_lv);
      }
    }
  }
}

void job_skill_game_instance::load(const character_save &cs) {
  skill_point = cs.sp.ski_sp;
}

uint8_t job_skill_game_instance::load_ski_level(const std::u16string &id) {
  auto ski_id = std::stoi(std::string(id.begin(), id.end()));
  if (skill_point.contains(ski_id)) {
    return skill_point.at(ski_id);
  } else {
    return 1;
  }
}

std::vector<job_type>
job_skill_game_instance::load_ski_tree(const std::u16string &id) {
  std::vector<job_type> r = {job_type::BEGINNER};
  if (id == u"0") {
    return r;
  }
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

std::u16string job_skill_game_instance::load_job_id(job_type type) {
  switch (type) {
  case job_type::BEGINNER:
    return u"000";
  case job_type::WARRIOR:
    return u"100";
  case job_type::MAGICIAN:
    return u"200";
  case job_type::BOWMAN:
    return u"300";
  case job_type::THIEF:
    return u"400";
  case job_type::FIGHTER:
    return u"110";
  case job_type::PAGE:
    return u"120";
  case job_type::SPEARMAN:
    return u"130";
  case job_type::FP_WIZARD:
    return u"210";
  case job_type::IL_WIZARD:
    return u"220";
  case job_type::CLERIC:
    return u"230";
  case job_type::HUNTER:
    return u"310";
  case job_type::CROSSBOWMAN:
    return u"320";
  case job_type::ASSASSIN:
    return u"410";
  case job_type::BANDIT:
    return u"420";
  }
  return u"";
}

wz::WzMap job_skill_game_instance::load_job_skis(job_type type) {
  wz::WzMap r;
  auto job = job_skill_game_instance::load_job_id(type);
  // 根据active_tab获取技能组
  auto skill_node = wz_resource::skill->find(job + u".img");
  skill_node = skill_node->get_child(u"skill");
  r = skill_node->children;
  // 删除所有长度大于5的key
  for (auto it = r.begin(); it != r.end();) {
    auto n = it->second[0];
    if (!n->get_child(u"icon")) {
      it = r.erase(it);
    } else {
      ++it; // 不删除时手动递增
    }
  }
  return r;
}

std::vector<job_type> job_skill_game_instance::load_ski_tree(int i) {
  auto tmp = std::to_string(i);
  std::u16string id{tmp.begin(), tmp.end()};
  return load_ski_tree(id);
}