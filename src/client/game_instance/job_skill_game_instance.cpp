#include "job_skill_game_instance.h"
#include "character_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include <cstdint>
#include <flat_set>
#include <string>

void JobSkillGameInstance::load(const CharacterSaveData &cs) {
  skill_point = cs.sp.ski_sp;
  remain_point = cs.remain_sp;
}

uint8_t JobSkillGameInstance::load_ski_level(const std::u16string &id) {
  auto ski_id = std::stoi(std::string(id.begin(), id.end()));
  if (skill_point.contains(ski_id)) {
    return skill_point.at(ski_id);
  } else {
    return 1;
  }
}

std::vector<JobType>
JobSkillGameInstance::load_ski_tree(const std::u16string &id) {
  std::vector<JobType> r = {JobType::BEGINNER};
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
    r.push_back(JobType::WARRIOR);
    switch (ch2) {
    case u'1': {
      r.push_back(JobType::FIGHTER);
      break;
    }
    case u'2': {
      r.push_back(JobType::PAGE);
      break;
    }
    case u'3': {
      r.push_back(JobType::SPEARMAN);
      break;
    }
    }
    break;
  }
  case u'2': {
    r.push_back(JobType::MAGICIAN);
    switch (ch2) {
    case u'1': {
      r.push_back(JobType::FP_WIZARD);
      break;
    }
    case u'2': {
      r.push_back(JobType::IL_WIZARD);
      break;
    }
    case u'3': {
      r.push_back(JobType::CLERIC);
      break;
    }
    }
    break;
  }
  case u'3': {
    r.push_back(JobType::BOWMAN);
    switch (ch2) {
    case u'1': {
      r.push_back(JobType::HUNTER);
      break;
    }
    case u'2': {
      r.push_back(JobType::CROSSBOWMAN);
      break;
    }
    }
    break;
  }
  case u'4': {
    r.push_back(JobType::THIEF);
    switch (ch2) {
    case u'1': {
      r.push_back(JobType::ASSASSIN);
      break;
    }
    case u'2': {
      r.push_back(JobType::BANDIT);
      break;
    }
    }
    break;
  }
  }

  return r;
}

std::u16string JobSkillGameInstance::load_job_id(JobType type) {
  switch (type) {
  case JobType::BEGINNER:
    return u"000";
  case JobType::WARRIOR:
    return u"100";
  case JobType::MAGICIAN:
    return u"200";
  case JobType::BOWMAN:
    return u"300";
  case JobType::THIEF:
    return u"400";
  case JobType::FIGHTER:
    return u"110";
  case JobType::PAGE:
    return u"120";
  case JobType::SPEARMAN:
    return u"130";
  case JobType::FP_WIZARD:
    return u"210";
  case JobType::IL_WIZARD:
    return u"220";
  case JobType::CLERIC:
    return u"230";
  case JobType::HUNTER:
    return u"310";
  case JobType::CROSSBOWMAN:
    return u"320";
  case JobType::ASSASSIN:
    return u"410";
  case JobType::BANDIT:
    return u"420";
  }
  return u"";
}

wz::WzMap JobSkillGameInstance::load_job_skis(JobType type) {
  wz::WzMap r;
  auto job = JobSkillGameInstance::load_job_id(type);
  // 根据active_tab获取技能组
  auto skill_node = WzResource::skill->find(job + u".img");
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

std::vector<JobType> JobSkillGameInstance::load_ski_tree(int i) {
  auto tmp = std::to_string(i);
  std::u16string id{tmp.begin(), tmp.end()};
  return load_ski_tree(id);
}

std::u16string JobSkillGameInstance::load_two_jump() {
  const static std::flat_set<std::u16string> strs = {
      u"4111005",
  };
  for (auto k : strs) {
    auto ski = load_ski_level(k);
    if (ski > 0) {
      return k;
    }
  }
  return u"";
}