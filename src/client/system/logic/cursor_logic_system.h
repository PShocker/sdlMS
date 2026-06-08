#pragma once

#include <string>
class cursor_logic_system {
private:
  static bool run_package_motion();

  static void run_cursor_ui();
  static void run_cursor_action(const std::u16string &action);

  static bool run_default();
  static bool run_animate();

public:
  static bool run();
};