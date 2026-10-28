#include "login_notice_system_instance.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/login_notice_ui_system.h"

void LoginNoticeSystemInstance::enter(NoticeType type, void (*fn)()) {
  LoginNoticeUiSystem::type = type;
  LoginNoticeUiSystem::fn = fn;

  auto render = LoginNoticeUiSystem::render;
  System::render_systems.insert(System::render_systems.end() - 1, render);

  System::logic_systems.push_back(LoginNoticeUiSystem::run);

  auto event = LoginNoticeUiSystem::event;
  System::event_systems.insert(System::event_systems.begin(), event);
}