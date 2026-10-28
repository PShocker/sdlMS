#ifndef SDLMS_SRC_COMMON_SCRIPT_SCRIPT_H_
#define SDLMS_SRC_COMMON_SCRIPT_SCRIPT_H_

#include <any>
#include <flat_map>
#include <string>
namespace sdlms {

class Script {
public:
  static auto &fns() {
    static std::flat_map<std::u16string, void (*)(std::any)> instance;
    return instance;
  }
};

}  // namespace sdlms

#endif  // SDLMS_SRC_COMMON_SCRIPT_SCRIPT_H_
