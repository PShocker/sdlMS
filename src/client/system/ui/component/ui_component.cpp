#include "src/client/system/ui/component/ui_component.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/window/window.h"

namespace sdlms {

void RenderComponents(UIComponents& components) {
  for (auto& component : components) {
    std::visit([](auto& widget) { widget.Render(); }, component);
  }
}

bool DispatchButtonClick(UIComponents& components, bool play_sound) {
  for (auto& component : components) {
    auto* button = std::get_if<Button>(&component);
    if (button == nullptr || button->callback == nullptr || !button->enable ||
        !button->focused) {
      continue;
    }
    if (!button->Hover()) {
      continue;
    }
    if (play_sound) {
      AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
    }
    button->callback();
    return true;
  }
  return false;
}

int ButtonHoverIndex(UIComponents& components) {
  for (int i = 0; i < static_cast<int>(components.size()); ++i) {
    auto* button = std::get_if<Button>(&components[i]);
    if (button != nullptr && button->Hover()) {
      return i;
    }
  }
  return -1;
}

}  // namespace sdlms
