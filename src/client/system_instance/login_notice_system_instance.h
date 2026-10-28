#pragma once

#include <cstdint>
class LoginNoticeSystemInstance {
public:
  enum NoticeType {
    charactername_error,
    charactername_used,
    character_delete,
    character_full,
    character_use_ap,
    logining,
    logining_used,
  };
  static void enter(NoticeType type, void (*fn)());
};