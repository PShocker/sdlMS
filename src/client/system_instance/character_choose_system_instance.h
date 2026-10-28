#pragma once

#include <string>
#include <vector>
class CharacterChooseSystemInstance {
public:
  static void enter_prepare(const std::string &login);

  static void enter();
  static void enter(const std::string &login);
};