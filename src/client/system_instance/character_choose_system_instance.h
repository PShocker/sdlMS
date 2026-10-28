#ifndef SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_CHARACTER_CHOOSE_SYSTEM_INSTANCE_H_
#define SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_CHARACTER_CHOOSE_SYSTEM_INSTANCE_H_

#include <string>
#include <vector>
namespace sdlms {

class CharacterChooseSystemInstance {
public:
  static void EnterPrepare(const std::string &login);

  static void Enter();
  static void Enter(const std::string &login);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_CHARACTER_CHOOSE_SYSTEM_INSTANCE_H_
