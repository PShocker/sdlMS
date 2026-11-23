#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/system/ui/quest_alarm_ui_system.h"
#include "src/client/system/ui/skill_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/script/script.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iterator>
#include <string>
#include <utility>

namespace sdlms {

// 二转（2nd job advancement）任务脚本。
//
// QuestData 里四个职业的二转链是 20000~20003 / 20100~20103 / 20200~20203 /
// 20300~20303，其中每个职业链：
//   200X2 "Test of Qualification"：Check/0/startscript=q200X2s（接任务），
//        Check/1/endscript=q200X2e（交任务成功），failscript=q200X2f（失败）。
//   200X3 "Proof of
//   Qualification"：Check/1/endscript=q200X3e，在这里真正改职业。
// 这些脚本名此前都没有实现，NpcDlgUiSystem 用 Script::fns().at() 调用会抛异常，
// 因此二转链在交任务时必崩；本文件把四套脚本补齐。

namespace {

struct SecondJob {
  JobType type;
  const char16_t* name;
};

struct JobChain {
  const char16_t* job_id;     // 一转职业 id，用来匹配当前角色
  const char16_t* title;      // 一转职业名（对话里用）
  const SecondJob* branches;  // 可选的二转分支
  size_t branch_count;
};

constexpr SecondJob kWarriorJobs[] = {
    {JobType::kFighter, u"剑客"},
    {JobType::kPage, u"准骑士"},
    {JobType::kSpearman, u"枪战士"},
};

constexpr SecondJob kMagicianJobs[] = {
    {JobType::kFpWizard, u"火毒法师"},
    {JobType::kIlWizard, u"冰雷法师"},
    {JobType::kCleric, u"牧师"},
};

constexpr SecondJob kBowmanJobs[] = {
    {JobType::kHunter, u"猎人"},
    {JobType::kCrossbowman, u"弩弓手"},
};

constexpr SecondJob kThiefJobs[] = {
    {JobType::kAssassin, u"刺客"},
    {JobType::kBandit, u"侠客"},
};

constexpr JobChain kChains[] = {
    {u"100", u"战士", kWarriorJobs, std::size(kWarriorJobs)},
    {u"200", u"法师", kMagicianJobs, std::size(kMagicianJobs)},
    {u"300", u"弓箭手", kBowmanJobs, std::size(kBowmanJobs)},
    {u"400", u"飞侠", kThiefJobs, std::size(kThiefJobs)},
};

const JobChain* ChainOf(const std::u16string& job_id) {
  for (const auto& chain : kChains) {
    if (job_id == chain.job_id) {
      return &chain;
    }
  }
  return nullptr;
}

void CloseDialog() {
  NpcDlgUiSystem::Close();
}

// 用脚本对话的形式显示一段文字。btn 决定底部按钮（kOk 时只能点 OK 继续）。
void ShowText(
    const std::u16string& text,
    NpcDlgUiSystem::NpcDlgButton btn = NpcDlgUiSystem::NpcDlgButton::kOk,
    std::function<void()> on_ok = CloseDialog) {
  NpcDlgUiSystem::Close();
  NpcDlgUiSystem::type_ = NpcDlgUiSystem::NpcDlgEnum::kScript;
  NpcDlgUiSystem::index_ = 0;
  NpcDlgUiSystem::max_index_ = 0;
  NpcDlgUiSystem::select_ = 0;
  NpcDlgUiSystem::max_select_ = 0;
  NpcDlgUiSystem::buttons_ = btn;
  NpcDlgUiSystem::on_yes_ = nullptr;
  NpcDlgUiSystem::on_no_ = nullptr;
  NpcDlgUiSystem::on_ok_ = std::move(on_ok);
  NpcDlgUiSystem::cb_ = nullptr;
  NpcDlgUiSystem::time_ = Window::dt_now_;
  NpcDlgUiSystem::text_ = text;
  NpcDlgUiSystem::Open();
}

// 真正改职业：与一转脚本（job_advance.cpp）保持一致的处理顺序。
void ApplySecondJob(const SecondJob& job) {
  auto& self = CharacterGameInstance::self_;
  self.job = JobSkillGameInstance::LoadJobId(job.type);

  JobSkillGameInstance::RecalculateRemainSp();
  CharacterStatGameInstance::Update();
  SkillUiSystem::SelectSecondJobTab();
  // 下一帧把新职业同步给服务器和同地图玩家。
  CharacterLogicSystem::cct_.map_id = SceneSystemInstance::map_id_;
  AudioGameInstance::PlayNow(u"Game.img/LevelUp");
}

void ConfirmSecondJob(const JobChain& chain) {
  const int count = static_cast<int>(chain.branch_count);
  const int select = static_cast<int>(NpcDlgUiSystem::select_);
  const int index = std::clamp(select, 0, count - 1);
  const SecondJob job = chain.branches[index];

  // 先结算 200X3 的奖励并标记完成，再改职业。
  const bool settled = NpcDlgUiSystem::FinishQuest();
  ApplySecondJob(job);

  std::u16string text = u"你成为" + std::u16string(job.name) + u"了！";
  if (!settled) {
    text = u"背包空间不足，奖励没能全部领取。\n请整理背包后再来找我。";
  }
  ShowText(text);
}

void StartChoice(const JobChain& chain) {
  std::u16string text = u"你已通过全部考验。\n想走哪一条" +
                        std::u16string(chain.title) + u"的道路？";
  for (size_t i = 0; i < chain.branch_count; ++i) {
    text += u"\n#L";
    text += static_cast<char16_t>(u'0' + i);
    text += u"#";
    text += chain.branches[i].name;
    text += u"#l";
  }

  NpcDlgUiSystem::Close();
  NpcDlgUiSystem::type_ = NpcDlgUiSystem::NpcDlgEnum::kSelect;
  NpcDlgUiSystem::index_ = 0;
  NpcDlgUiSystem::max_index_ = 0;
  NpcDlgUiSystem::select_ = 0;
  NpcDlgUiSystem::max_select_ = static_cast<int8_t>(chain.branch_count - 1);
  NpcDlgUiSystem::buttons_ = NpcDlgUiSystem::NpcDlgButton::kDefault;
  NpcDlgUiSystem::on_yes_ = nullptr;
  NpcDlgUiSystem::on_no_ = nullptr;
  NpcDlgUiSystem::on_ok_ = nullptr;
  NpcDlgUiSystem::text_ = text;
  NpcDlgUiSystem::time_ = Window::dt_now_;
  NpcDlgUiSystem::cb_ = [chain] { ConfirmSecondJob(chain); };
  NpcDlgUiSystem::Open();
}

// 试炼道具（Check/1/item）的文字说明：不同职业的试炼道具不同。
std::u16string TestItemIcon(const std::u16string& quest_id) {
  if (quest_id == u"20002.img") {
    return u"#i4031017#";
  }
  if (quest_id == u"20102.img") {
    return u"#i4031019#";
  }
  if (quest_id == u"20202.img") {
    return u"#i4031021#";
  }
  return u"#i4031023#";
}

// 试炼进度，例如 "12/30"。
std::u16string TestProgress(const GameQuest& quest) {
  for (const auto& [id, req] : quest.check_item) {
    const uint32_t have = quest.item.contains(id) ? quest.item.at(id).count : 0;
    const auto text = std::to_string(have) + "/" + std::to_string(req.count);
    return {text.begin(), text.end()};
  }
  return u"";
}

// Check/0/startscript：接 "Test of Qualification" 时讲解测试内容，并把玩家
// 送进试炼场（试炼场只有一个出生传送门，进出都靠脚本）。
// 脚本接管了普通接任务流程，所以由这里调用 AcceptQuest。
void StartTest(std::any) {
  const auto id = NpcDlgUiSystem::quest_id_;
  if (id.empty()) {
    return;
  }
  if (!QuestGameInstance::progress_quests_.contains(id) &&
      !QuestGameInstance::complete_quests_.contains(id)) {
    QuestGameInstance::AcceptQuest(id);
    QuestAlarmUiSystem::AcceptQuest(id);
  }
  ShowText(
      u"想通过考验，就先证明你的实力。\n进到试炼场去打倒里头的怪物，"
      u"把 30 颗 " +
      TestItemIcon(id) + u" 带回来给我。\n准备妥当就开始吧。");
  if (const auto area = QuestGameInstance::LoadJobTestArea(id);
      area.has_value()) {
    SceneSystemInstance::EnterPrepare(area->arena_map, u"sp", 0);
  }
}

// 试炼场里的教官（Npc.wz 里 link 到城里教官的自定义 NPC）：可以中途离开，
// 也可以看当前进度。凑齐道具时 QuestGameInstance 会自动把玩家送回去。
void ArenaExit(std::any) {
  const auto map_id = SceneSystemInstance::map_id_;
  for (const char16_t* id :
       {u"20002.img", u"20102.img", u"20202.img", u"20302.img"}) {
    const auto area = QuestGameInstance::LoadJobTestArea(id);
    if (!area.has_value() || area->arena_map != map_id) {
      continue;
    }
    if (auto it = QuestGameInstance::progress_quests_.find(id);
        it != QuestGameInstance::progress_quests_.end()) {
      const auto progress = TestProgress(it->second);
      const bool done = it->second.item_bool && it->second.mob_bool;
      std::u16string text;
      if (done) {
        text = u"你已经通过了考验（" + progress + u"）。\n回去向教官报告吧。";
      } else {
        text = u"考验还在进行中（" + progress +
               u"）。\n现在离开的话，需要放弃任务"
               u"才能重新进入试炼场。要离开吗？";
      }
      NpcDlgUiSystem::Close();
      NpcDlgUiSystem::type_ = NpcDlgUiSystem::NpcDlgEnum::kScript;
      NpcDlgUiSystem::index_ = 0;
      NpcDlgUiSystem::max_index_ = 0;
      NpcDlgUiSystem::buttons_ = NpcDlgUiSystem::NpcDlgButton::kYesNo;
      NpcDlgUiSystem::on_yes_ = [map = area->instructor_map] {
        SceneSystemInstance::EnterPrepare(map, u"sp", 0);
        NpcDlgUiSystem::Close();
      };
      NpcDlgUiSystem::on_no_ = CloseDialog;
      NpcDlgUiSystem::on_ok_ = nullptr;
      NpcDlgUiSystem::cb_ = nullptr;
      NpcDlgUiSystem::time_ = Window::dt_now_;
      NpcDlgUiSystem::text_ = text;
      NpcDlgUiSystem::Open();
      return;
    }
    // 没有进行中的试炼：直接送回教官面前。
    SceneSystemInstance::EnterPrepare(area->instructor_map, u"sp", 0);
    return;
  }
  ShowText(u"这里没有你需要做的事。");
}

// Check/1/endscript：交 "Test of Qualification"。
void PassTest(std::any) {
  if (!NpcDlgUiSystem::FinishQuest()) {
    return;  // 背包不足，FinishQuest 已经提示并关闭对话
  }
  AudioGameInstance::PlayNow(u"Game.img/QuestClear");
  ShowText(u"很好，你通过了考验。\n回去把结果告诉教官吧。");
}

// Check/1/failscript：测试失败（当前流程还不会触发，先补齐以保证可调用）。
void FailTest(std::any) {
  ShowText(u"这次没能通过考验。\n调整一下状态，再回来挑战一次吧。");
}

// Check/1/endscript：交 "Proof of Qualification"，在这里选择二转职业。
void SecondJobAdvance(std::any) {
  auto& self = CharacterGameInstance::self_;
  if (self.job.empty() || self.job == u"0" || self.job == u"000") {
    // 还没一转（数据上不该出现），直接结算任务收尾。
    NpcDlgUiSystem::FinishQuest();
    ShowText(u"先去完成一转再来找我吧。");
    return;
  }
  const JobChain* chain = ChainOf(self.job);
  if (chain == nullptr) {
    // 已经是二转职业了：只结算任务，不再改职业。
    NpcDlgUiSystem::FinishQuest();
    ShowText(u"你已经二转过一次了，无法再次转职。");
    return;
  }
  StartChoice(*chain);
}

}  // namespace

[[maybe_unused]] static const bool r = [] {
  auto& fns = Script::fns();
  // 四条职业链共用同一套逻辑，注册在各自的数据脚本名下。
  fns[u"q20002s"] = StartTest;
  fns[u"q20002e"] = PassTest;
  fns[u"q20002f"] = FailTest;
  fns[u"q20003e"] = SecondJobAdvance;

  fns[u"q20102s"] = StartTest;
  fns[u"q20102e"] = PassTest;
  fns[u"q20102f"] = FailTest;
  fns[u"q20103e"] = SecondJobAdvance;

  fns[u"q20202s"] = StartTest;
  fns[u"q20202e"] = PassTest;
  fns[u"q20202f"] = FailTest;
  fns[u"q20203e"] = SecondJobAdvance;

  fns[u"q20302s"] = StartTest;
  fns[u"q20302e"] = PassTest;
  fns[u"q20302f"] = FailTest;
  fns[u"q20303e"] = SecondJobAdvance;

  // 试炼场里的教官（四个试炼场各一个，Npc.wz 里 link 到城里教官）
  fns[u"npc_800003"] = ArenaExit;
  fns[u"npc_800004"] = ArenaExit;
  fns[u"npc_800005"] = ArenaExit;
  fns[u"npc_800006"] = ArenaExit;
  return true;
}();

}  // namespace sdlms
