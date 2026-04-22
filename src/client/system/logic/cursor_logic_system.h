#pragma once

class cursor_logic_system {
private:
  static bool run_mousedown();
  static bool run_default();

public:
  static bool run();
};