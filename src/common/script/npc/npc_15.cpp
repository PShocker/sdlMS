#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/script/script.h"

#include <cstdint>
#include <functional>
#include <utility>

namespace sdlms {

namespace {

constexpr uint32_t kRequiredLevel = 8;
constexpr uint64_t kMesoCost = 150;
constexpr uint32_t kTargetMap = 10000000;

constexpr char16_t kLevelNotEnough[] = u"等级不够";
constexpr char16_t kLevelEnough[] = u"恭喜你,可以离开这里了";
constexpr char16_t kAskLeave[] = u"是否愿意花费150meso离开这里";
constexpr char16_t kGoodLuck[] = u"祝你好运";
constexpr char16_t kNotEnoughMeso[] = u"金币不够";

void CloseDialog() { NpcDlgUiSystem::Close(); }

void ShowResult(const char16_t* text, std::function<void()> on_ok) {
  NpcDlgUiSystem::text_ = text;
  NpcDlgUiSystem::buttons_ = NpcDlgUiSystem::NpcDlgButton::kOk;
  NpcDlgUiSystem::on_ok_ = std::move(on_ok);
  NpcDlgUiSystem::time_ = Window::dt_now_;
}

void LeaveHere() {
  // 结果页不可回退，这里再确认一次，避免动画期间状态变化导致扣款后无法传送。
  if (PackageGameInstance::meso_ < kMesoCost) {
    ShowResult(kNotEnoughMeso, CloseDialog);
    return;
  }
  PackageGameInstance::meso_ -= kMesoCost;
  NpcDlgUiSystem::Close();
  SceneSystemInstance::EnterPrepare(kTargetMap, u"sp", 0);
}

void OnYes() {
  if (PackageGameInstance::meso_ < kMesoCost) {
    ShowResult(kNotEnoughMeso, CloseDialog);
    return;
  }
  ShowResult(kGoodLuck, LeaveHere);
}

void OnNext() {
  if (NpcDlgUiSystem::index_ != 1) {
    return;
  }
  NpcDlgUiSystem::text_ = kAskLeave;
  NpcDlgUiSystem::buttons_ = NpcDlgUiSystem::NpcDlgButton::kYesNo;
  NpcDlgUiSystem::time_ = Window::dt_now_;
}

void Npc15(std::any) {
  NpcDlgUiSystem::Close();

  NpcDlgUiSystem::npc_id_ = u"0000015";
  NpcDlgUiSystem::type_ = NpcDlgUiSystem::NpcDlgEnum::kScript;
  NpcDlgUiSystem::index_ = 0;
  NpcDlgUiSystem::max_index_ = 0;
  NpcDlgUiSystem::select_ = 0;
  NpcDlgUiSystem::buttons_ = NpcDlgUiSystem::NpcDlgButton::kDefault;
  NpcDlgUiSystem::cb_ = nullptr;
  NpcDlgUiSystem::on_yes_ = OnYes;
  NpcDlgUiSystem::on_no_ = CloseDialog;
  NpcDlgUiSystem::on_ok_ = nullptr;

  if (CharacterGameInstance::self_.level < kRequiredLevel) {
    NpcDlgUiSystem::text_ = kLevelNotEnough;
  } else {
    NpcDlgUiSystem::text_ = kLevelEnough;
    NpcDlgUiSystem::max_index_ = 1;
    NpcDlgUiSystem::cb_ = OnNext;
  }

  NpcDlgUiSystem::time_ = Window::dt_now_;
  NpcDlgUiSystem::Open();
}

}  // namespace

[[maybe_unused]] static const bool r = [] {
  auto& fns = Script::fns();
  fns[u"npc_15"] = Npc15;
  return true;
}();

}  // namespace sdlms
