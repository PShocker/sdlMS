#pragma once

namespace sdlms {

class CursorRenderSystem {
 private:
  static void RenderCursor();
  static void RenderHand();

 public:
  static bool Render();
};

}  // namespace sdlms
