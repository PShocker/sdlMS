#include "package_ui_system.h"
#include "src/client/system/system.h"
#include <cstdlib>

bool package_ui_system::render() { return true; }

void package_ui_system::open() {
  system::render_systems.push_back(package_ui_system::render);
  system::event_systems.push_back(package_ui_system::event);
}

void package_ui_system::close() {
  std::erase(system::render_systems, package_ui_system::render);
  std::erase(system::event_systems, package_ui_system::event);
}

bool package_ui_system::event(SDL_Event *event) { return false; }