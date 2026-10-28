#include "login_notice_system_instance.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/login_notice_ui_system.h"

void LoginNoticeSystemInstance::Enter(NoticeType type, void (*fn)()) {
  LoginNoticeUiSystem::type = type;
  LoginNoticeUiSystem::fn = fn;

  auto Render = LoginNoticeUiSystem::Render;
  System::render_systems.insert(System::render_systems.end() - 1, Render);

  System::logic_systems.push_back(LoginNoticeUiSystem::Run);

  auto Event = LoginNoticeUiSystem::Event;
  System::event_systems.insert(System::event_systems.begin(), Event);
}