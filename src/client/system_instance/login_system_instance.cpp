#include "login_system_instance.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/login_ui_system.h"

void login_system_instance::enter() {
  system::event_systems = {

  };
  system::logic_systems = {};

  system::render_systems = {
      login_ui_system::render,
  };
}