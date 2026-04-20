#include "quest_ui_system.h"
#include "src/client/system/system.h"

bool quest_ui_system::render() { return true; }

void quest_ui_system::open() {
  system::render_systems.push_back(quest_ui_system::render);
  system::event_systems.push_back(quest_ui_system::event);
}

void quest_ui_system::close() {}

bool quest_ui_system::event(SDL_Event *event) { return true; }