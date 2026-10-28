#ifndef SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_DROP_SYSTEM_H_
#define SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_DROP_SYSTEM_H_
namespace sdlms {
class ServerDropSystem {
 private:
  static void RunFade();
  static void RunDelayDrops();
  static void RunDestroy();

 public:
  static bool Run();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_DROP_SYSTEM_H_
