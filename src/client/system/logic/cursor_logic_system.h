#pragma once

class cursor_logic_system {
private:
  static bool run_mousedown();
  static bool run_default();
  static void run_cursor_ui();

public:
  static bool run();
};