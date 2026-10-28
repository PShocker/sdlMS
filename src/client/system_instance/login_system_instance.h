#ifndef SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_LOGIN_SYSTEM_INSTANCE_H_
#define SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_LOGIN_SYSTEM_INSTANCE_H_
namespace sdlms {
class LoginSystemInstance {
 public:
  static void EnterPrepare();
  static void Enter();
  static bool RenderGame();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_LOGIN_SYSTEM_INSTANCE_H_
