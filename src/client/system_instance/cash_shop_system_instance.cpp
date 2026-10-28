#include "src/client/system_instance/cash_shop_system_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/cash_shop_ui_system.h"
#include "src/common/wz/wz_resource.h"
namespace sdlms {


void CashShopSystemInstance::Enter() {
  auto node = WzResource::ui_->find(u"CashShopPreview.img");
  FootholdGameInstance::data_ = FootholdGameInstance::Load(node);

  System::logic_systems_ = {CharacterLogicSystem::Run};

  System::render_systems_ = {
      CashShopUiSystem::Render,
  };
  System::event_systems_ = {
      CashShopUiSystem::Event,
  };
}

}  // namespace sdlms
