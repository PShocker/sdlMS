#ifndef SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_LOGIN_NOTICE_SYSTEM_INSTANCE_H_
#define SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_LOGIN_NOTICE_SYSTEM_INSTANCE_H_

#include <cstdint>
namespace sdlms {

class LoginNoticeSystemInstance {
 public:
  enum class NoticeType {
    kCharacternameError,
    kCharacternameUsed,
    kCharacterDelete,
    kCharacterFull,
    kCharacterUseAp,
    kLogining,
    kLoginingUsed,
  };
  static void Enter(NoticeType type, void (*fn)());
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_LOGIN_NOTICE_SYSTEM_INSTANCE_H_
