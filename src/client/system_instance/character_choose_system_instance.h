#pragma once

#include <string>
#include <vector>
class CharacterChooseSystemInstance {
public:
  static void EnterPrepare(const std::string &login);

  static void Enter();
  static void Enter(const std::string &login);
};