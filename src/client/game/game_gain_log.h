#pragma once

#include <cstdint>
#include <string>
namespace sdlms {


enum class GainEnum { kItem, kExperience };

struct GameGainLog{
  std::u16string id;
  int32_t num;
  uint64_t destroy;

  GainEnum type;
};

}  // namespace sdlms
