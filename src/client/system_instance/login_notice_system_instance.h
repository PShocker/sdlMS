#pragma once

#include <cstdint>
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
  static void enter(NoticeType type, void (*fn)());
};