#pragma once

#include <string>
#include <vector>
class character_choose_system_instance {
public:
  static void enter_prepare(const std::string &login);

  static void enter();
  static void enter(const std::string &login);
};