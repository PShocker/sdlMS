#pragma once

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
