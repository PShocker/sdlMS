#include "../script.h"
#include "src/client/game/game_item.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
// roger apple

static void state() {
  switch (NpcDlgUiSystem::index) {
  case 1: {
    NpcDlgUiSystem::close();
    GameConsumeItem con;
    con.num = 1;
    con.id = u"02010000";
    std::polymorphic<GameItem> itm{con};
    auto i = PackageGameInstance::add_new_item(itm);
    if (!i.has_value()) {
      NoticeUiSystem::open_no_space(itm->type);
    } else {
      // accept quest
      QuestGameInstance::accept_quest(u"1002.img");
    }
    break;
  }
  case 0: {
    auto text_node =
        WzResource::ms->get_root()->find(u"String.img/Script/npc_3/t0");
    auto text = static_cast<wz::Property<std::u16string> *>(text_node)->get();
    NpcDlgUiSystem::text = text;
    break;
  }
  }
  NpcDlgUiSystem::time = Window::dt_now;
}

static void q1002s(std::any data) {
  NpcDlgUiSystem::type = NpcDlgUiSystem::NpcDlgEnum::quest_avaliable;
  NpcDlgUiSystem::max_index = 0;
  NpcDlgUiSystem::npc_id = u"0000003";
  state();
  return;
};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"q1002s"] = q1002s;
  return true;
}();