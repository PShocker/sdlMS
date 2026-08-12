#pragma once

#include <any>
#include <flat_map>
#include <string>
class script {
public:
  static auto &fns() {
    static std::flat_map<std::u16string, void (*)(std::any)> instance;
    return instance;
  }
};