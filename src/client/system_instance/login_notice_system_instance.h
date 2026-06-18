#pragma once

#include <cstdint>
class login_notice_system_instance {
public:
  enum notice_type {
    username_error,
  };
  static void enter(notice_type type, void (*fn)());
};