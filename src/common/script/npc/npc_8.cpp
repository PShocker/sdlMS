#include "../script.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

static void State() {
  switch (NpcDlgUiSystem::index) {
  case 0: {
    auto text_node =
        WzResource::ms->get_root()->find(u"String.img/Script/npc_8/t0");
    auto text = static_cast<wz::Property<std::u16string> *>(text_node)->get();
    NpcDlgUiSystem::text = text;
    break;
  }
  case 1: {
    auto text_node =
        WzResource::ms->get_root()->find(u"String.img/Script/npc_8/t1");
    auto text = static_cast<wz::Property<std::u16string> *>(text_node)->get();
    NpcDlgUiSystem::text = text;
    break;
  }
  }
  NpcDlgUiSystem::time = Window::dt_now;
}

static void Npc8(std::any data) {
  NpcDlgUiSystem::Close();
  NpcDlgUiSystem::Open();
  NpcDlgUiSystem::type = NpcDlgUiSystem::NpcDlgEnum::kTalk;
  NpcDlgUiSystem::index = 0;
  NpcDlgUiSystem::max_index = 1;
  NpcDlgUiSystem::cb = State;
  NpcDlgUiSystem::npc_id = u"0000008";
  State();
  return;
};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"npc_8"] = Npc8;
  return true;
}();