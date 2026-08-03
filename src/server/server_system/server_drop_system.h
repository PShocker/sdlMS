#pragma once

class server_drop_system {
private:
  static void run_fade();
  static void run_destroy();

public:
  static bool run();
};