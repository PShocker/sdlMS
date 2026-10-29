#include "src/client/system_instance/login_notice_system_instance.h"

#include "src/client/system/system.h"
#include "src/client/system/ui/login_notice_ui_system.h"

namespace sdlms {

void LoginNoticeSystemInstance::Enter(NoticeType type, void (*fn_)()) {
  LoginNoticeUiSystem::type_ = type;
  LoginNoticeUiSystem::fn_ = fn_;

  auto render = LoginNoticeUiSystem::Render;
  System::render_systems_.insert(System::render_systems_.end() - 1, render);

  System::logic_systems_.push_back(LoginNoticeUiSystem::Run);

  auto event = LoginNoticeUiSystem::Event;
  System::event_systems_.insert(System::event_systems_.begin(), event);
}

}  // namespace sdlms
