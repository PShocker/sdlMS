#pragma once

class cursor_render_system {
private:
  static void render_cursor();
  static void render_hand();

public:
  static bool render();
};