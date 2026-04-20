#include "equip_ui_system.h"
#include "src/client/system/system.h"

bool equip_ui_system::render() { return true; }

void equip_ui_system::open() {
  system::render_systems.push_back(equip_ui_system::render);
  system::event_systems.push_back(equip_ui_system::event);
}

void equip_ui_system::close() {}

bool equip_ui_system::event(SDL_Event *event) { return true; }