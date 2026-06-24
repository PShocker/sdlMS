#include "login_notice_system_instance.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/login_notice_ui_system.h"

void login_notice_system_instance::enter(notice_type type, void (*fn)()) {
  login_notice_ui_system::type = type;
  login_notice_ui_system::fn = fn;

  auto render = login_notice_ui_system::render;
  system::render_systems.insert(system::render_systems.end() - 1, render);

  system::logic_systems.push_back(login_notice_ui_system::run);

  auto event = login_notice_ui_system::event;
  system::event_systems.insert(system::event_systems.begin(), event);
}