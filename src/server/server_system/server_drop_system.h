#pragma once

class ServerDropSystem {
private:
  static void RunFade();
  static void RunDelayDrops();
  static void RunDestroy();

public:
  static bool Run();
};