#include "job_skill_game_instance.h"
#include "character_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include <cstdint>
#include <flat_set>
#include <string>

void JobSkillGameInstance::Load(const CharacterSaveData &cs) {
  skill_point = cs.sp.ski_sp;
  remain_point = cs.remain_sp;
}

uint8_t JobSkillGameInstance::LoadSkiLevel(const std::u16string &id) {
  auto ski_id = std::stoi(std::string(id.begin(), id.end()));
  if (skill_point.contains(ski_id)) {
    return skill_point.at(ski_id);
  } else {
    return 1;
  }
}

std::vector<JobType>
JobSkillGameInstance::LoadSkiTree(const std::u16string &id) {
  std::vector<JobType> r = {JobType::kBeginner};
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
    r.push_back(JobType::kWarrior);
    switch (ch2) {
    case u'1': {
      r.push_back(JobType::kFighter);
      break;
    }
    case u'2': {
      r.push_back(JobType::kPage);
      break;
    }
    case u'3': {
      r.push_back(JobType::kSpearman);
      break;
    }
    }
    break;
  }
  case u'2': {
    r.push_back(JobType::kMagician);
    switch (ch2) {
    case u'1': {
      r.push_back(JobType::kFpWizard);
      break;
    }
    case u'2': {
      r.push_back(JobType::kIlWizard);
      break;
    }
    case u'3': {
      r.push_back(JobType::kCleric);
      break;
    }
    }
    break;
  }
  case u'3': {
    r.push_back(JobType::kBowman);
    switch (ch2) {
    case u'1': {
      r.push_back(JobType::kHunter);
      break;
    }
    case u'2': {
      r.push_back(JobType::kCrossbowman);
      break;
    }
    }
    break;
  }
  case u'4': {
    r.push_back(JobType::kThief);
    switch (ch2) {
    case u'1': {
      r.push_back(JobType::kAssassin);
      break;
    }
    case u'2': {
      r.push_back(JobType::kBandit);
      break;
    }
    }
    break;
  }
  }

  return r;
}

std::u16string JobSkillGameInstance::LoadJobId(JobType type) {
  switch (type) {
  case JobType::kBeginner:
    return u"000";
  case JobType::kWarrior:
    return u"100";
  case JobType::kMagician:
    return u"200";
  case JobType::kBowman:
    return u"300";
  case JobType::kThief:
    return u"400";
  case JobType::kFighter:
    return u"110";
  case JobType::kPage:
    return u"120";
  case JobType::kSpearman:
    return u"130";
  case JobType::kFpWizard:
    return u"210";
  case JobType::kIlWizard:
    return u"220";
  case JobType::kCleric:
    return u"230";
  case JobType::kHunter:
    return u"310";
  case JobType::kCrossbowman:
    return u"320";
  case JobType::kAssassin:
    return u"410";
  case JobType::kBandit:
    return u"420";
  }
  return u"";
}

wz::WzMap JobSkillGameInstance::LoadJobSkis(JobType type) {
  wz::WzMap r;
  auto job = JobSkillGameInstance::LoadJobId(type);
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

std::vector<JobType> JobSkillGameInstance::LoadSkiTree(int i) {
  auto tmp = std::to_string(i);
  std::u16string id{tmp.begin(), tmp.end()};
  return LoadSkiTree(id);
}

std::u16string JobSkillGameInstance::LoadTwoJump() {
  const static std::flat_set<std::u16string> strs = {
      u"4111005",
  };
  for (auto k : strs) {
    auto ski = LoadSkiLevel(k);
    if (ski > 0) {
      return k;
    }
  }
  return u"";
}