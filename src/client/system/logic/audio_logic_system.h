#ifndef SDLMS_SRC_CLIENT_SYSTEM_LOGIC_AUDIO_LOGIC_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_LOGIC_AUDIO_LOGIC_SYSTEM_H_
namespace sdlms {
class AudioLogicSystem {
 private:
  static void RunBackgrnd();
  static void RunAudios();

 public:
  static bool Run();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_LOGIC_AUDIO_LOGIC_SYSTEM_H_
