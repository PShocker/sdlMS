#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/system/ui/skill_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/script/script.h"

#include <cstdint>
#include <string>

namespace sdlms {

namespace {

enum class StatKind {
  kStr,
  kDex,
  kInt,
};

struct FirstJob {
  const char16_t* job_id;
  const char16_t* name;
  const char16_t* npc_id;
  uint32_t level;
  StatKind stat;
  uint32_t stat_requirement;
};

// 095 get_job_change_level：法师 8 级，其余职业 10 级。
constexpr FirstJob kWarrior{u"100", u"战士", u"0000511", 10, StatKind::kStr, 35};
constexpr FirstJob kMagician{u"200", u"法师", u"0000313", 8, StatKind::kInt, 20};
constexpr FirstJob kBowman{u"300", u"弓箭手", u"0000221", 10, StatKind::kDex, 25};
constexpr FirstJob kThief{u"400", u"飞侠", u"0000411", 10, StatKind::kDex, 25};

const FirstJob* current_job = nullptr;

std::u16string ToU16(uint32_t value) {
  auto text = std::to_string(value);
  return {text.begin(), text.end()};
}

bool IsBeginner() {
  const auto& job = CharacterGameInstance::self_.job;
  return job.empty() || job == u"0" || job == u"000";
}

uint32_t LoadStat(StatKind kind) {
  switch (kind) {
    case StatKind::kStr:
      return CharacterStatGameInstance::GetStrAp();
    case StatKind::kDex:
      return CharacterStatGameInstance::GetDexAp();
    case StatKind::kInt:
      return CharacterStatGameInstance::GetIntAp();
  }
  return 0;
}

const char16_t* StatName(StatKind kind) {
  switch (kind) {
    case StatKind::kStr:
      return u"力量";
    case StatKind::kDex:
      return u"敏捷";
    case StatKind::kInt:
      return u"智力";
  }
  return u"";
}

void CloseDialog() { NpcDlgUiSystem::Close(); }

void AdvanceJob() {
  if (current_job == nullptr) {
    return;
  }
  auto& self = CharacterGameInstance::self_;
  self.job = current_job->job_id;

  JobSkillGameInstance::RecalculateRemainSp();
  CharacterStatGameInstance::Update();
  SkillUiSystem::SelectFirstJobTab();
  // 下一帧把新职业同步给服务器和同地图玩家。
  CharacterLogicSystem::cct_.map_id = SceneSystemInstance::map_id_;
  AudioGameInstance::PlayNow(u"Game.img/LevelUp");

  NpcDlgUiSystem::text_ =
      u"恭喜你成为" + std::u16string(current_job->name) + u"！";
  NpcDlgUiSystem::buttons_ = NpcDlgUiSystem::NpcDlgButton::kOk;
  NpcDlgUiSystem::on_ok_ = CloseDialog;
  NpcDlgUiSystem::time_ = Window::dt_now_;
}

void Start(const FirstJob& job) {
  current_job = &job;
  const auto& self = CharacterGameInstance::self_;
  const std::u16string name = job.name;

  NpcDlgUiSystem::Close();
  NpcDlgUiSystem::npc_id_ = job.npc_id;
  NpcDlgUiSystem::type_ = NpcDlgUiSystem::NpcDlgEnum::kScript;
  NpcDlgUiSystem::index_ = 0;
  NpcDlgUiSystem::max_index_ = 0;
  NpcDlgUiSystem::select_ = 0;
  NpcDlgUiSystem::buttons_ = NpcDlgUiSystem::NpcDlgButton::kDefault;
  NpcDlgUiSystem::cb_ = nullptr;
  NpcDlgUiSystem::on_yes_ = AdvanceJob;
  NpcDlgUiSystem::on_no_ = CloseDialog;
  NpcDlgUiSystem::on_ok_ = nullptr;
  NpcDlgUiSystem::time_ = Window::dt_now_;

  if (!IsBeginner()) {
    if (self.job == job.job_id) {
      NpcDlgUiSystem::text_ =
          u"你已经是" + name + u"了，无法再次进行一转。";
    } else {
      NpcDlgUiSystem::text_ =
          u"你已经选择了其他职业，无法转职为" + name + u"。";
    }
  } else if (self.level < job.level) {
    NpcDlgUiSystem::text_ = u"等级不够。需要达到" + ToU16(job.level) +
                            u"级才能转职为" + name + u"。";
  } else if (LoadStat(job.stat) < job.stat_requirement) {
    NpcDlgUiSystem::text_ = u"属性不够。需要" +
                            ToU16(job.stat_requirement) + u"点" +
                            std::u16string(StatName(job.stat)) +
                            u"才能转职为" + name + u"。";
  } else {
    NpcDlgUiSystem::text_ =
        u"你已达到转职条件。是否愿意成为" + name + u"？";
    NpcDlgUiSystem::buttons_ = NpcDlgUiSystem::NpcDlgButton::kYesNo;
  }

  NpcDlgUiSystem::Open();
}

void Warrior511(std::any) { Start(kWarrior); }
void Magician313(std::any) { Start(kMagician); }
void Bowman221(std::any) { Start(kBowman); }
void Thief411(std::any) { Start(kThief); }

}  // namespace

[[maybe_unused]] static const bool r = [] {
  auto& fns = Script::fns();
  fns[u"warrior_511"] = Warrior511;
  fns[u"magician_313"] = Magician313;
  fns[u"bowman_221"] = Bowman221;
  fns[u"thief_411"] = Thief411;
  return true;
}();

}  // namespace sdlms
