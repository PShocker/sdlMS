#include "src/common/script/script.h"
#include "src/client/game/game_item.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <wz/Property.h>
namespace sdlms {

// roger apple

static void state() {
  switch (NpcDlgUiSystem::index_) {
  case 1: {
    NpcDlgUiSystem::Close();
    GameConsumeItem con;
    con.num = 1;
    con.id = u"02010000";
    std::polymorphic<GameItem> itm{con};
    auto i = PackageGameInstance::AddNewItem(itm);
    if (!i.has_value()) {
      NoticeUiSystem::OpenNoSpace(itm->type);
    } else {
      // accept quest
      QuestGameInstance::AcceptQuest(u"1002.img");
    }
    break;
  }
  case 0: {
    auto text_node =
        WzResource::ms_->get_root()->find(u"String.img/Script/npc_3/t0");
    auto text = static_cast<wz::Property<std::u16string> *>(text_node)->get();
    NpcDlgUiSystem::text_ = text;
    break;
  }
  }
  NpcDlgUiSystem::time_ = Window::dt_now_;
}

static void Q1002s(std::any data) {
  NpcDlgUiSystem::type_ = NpcDlgUiSystem::NpcDlgEnum::kQuestAvaliable;
  NpcDlgUiSystem::max_index_ = 0;
  NpcDlgUiSystem::npc_id_ = u"0000003";
  state();
  return;
};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"q1002s"] = Q1002s;
  return true;
}();

}  // namespace sdlms
