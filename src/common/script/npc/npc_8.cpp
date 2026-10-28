#include "src/common/script/script.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <wz/Property.h>
namespace sdlms {


static void state() {
  switch (NpcDlgUiSystem::index_) {
  case 0: {
    auto text_node =
        WzResource::ms_->get_root()->find(u"String.img/Script/npc_8/t0");
    auto text = static_cast<wz::Property<std::u16string> *>(text_node)->get();
    NpcDlgUiSystem::text_ = text;
    break;
  }
  case 1: {
    auto text_node =
        WzResource::ms_->get_root()->find(u"String.img/Script/npc_8/t1");
    auto text = static_cast<wz::Property<std::u16string> *>(text_node)->get();
    NpcDlgUiSystem::text_ = text;
    break;
  }
  }
  NpcDlgUiSystem::time_ = Window::dt_now_;
}

static void Npc8(std::any data) {
  NpcDlgUiSystem::Close();
  NpcDlgUiSystem::Open();
  NpcDlgUiSystem::type_ = NpcDlgUiSystem::NpcDlgEnum::kTalk;
  NpcDlgUiSystem::index_ = 0;
  NpcDlgUiSystem::max_index_ = 1;
  NpcDlgUiSystem::cb_ = state;
  NpcDlgUiSystem::npc_id_ = u"0000008";
  state();
  return;
};

[[maybe_unused]] static const bool r = [] {
  auto &fns = Script::fns();
  fns[u"npc_8"] = Npc8;
  return true;
}();

}  // namespace sdlms
