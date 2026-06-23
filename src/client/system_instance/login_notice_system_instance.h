#pragma once

#include <cstdint>
class login_notice_system_instance {
public:
  enum notice_type {
    charactername_error,
    charactername_used,
    character_delete,
    character_full,
    character_use_ap,
    logining,
  };
  static void enter(notice_type type, void (*fn)());
};