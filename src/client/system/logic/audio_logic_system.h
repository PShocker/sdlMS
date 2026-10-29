#pragma once

namespace sdlms {

class AudioLogicSystem {
 private:
  static void RunBackgrnd();
  static void RunAudios();

 public:
  static bool Run();
};

}  // namespace sdlms
