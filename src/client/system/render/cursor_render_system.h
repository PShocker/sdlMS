#pragma once

class CursorRenderSystem {
private:
  static void render_cursor();
  static void render_hand();

public:
  static bool render();
};