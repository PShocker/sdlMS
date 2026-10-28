#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_COMBO_BOX_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_COMBO_BOX_UI_SYSTEM_H_

#include "src/client/window/window.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

struct ComboBox {
  bool open;
  uint32_t w;
  std::u16string selected;
  std::vector<std::u16string> selects;
  uint8_t val;
};

class ComboBoxUiSystem {
 public:
  static void Render(int x, int y, ComboBox cm, bool top, bool disable);
  static bool Click(int x, int y, ComboBox cm, bool top, bool disable);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_COMBO_BOX_UI_SYSTEM_H_
