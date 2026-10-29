#pragma once

namespace sdlms {

class GainLogUiSystem {
 private:
  static void RenderText();
  static void Destroy();

 public:
  static bool Render();
};

}  // namespace sdlms
