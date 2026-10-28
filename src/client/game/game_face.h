#pragma once

#include <cstdint>
#include <string>
namespace sdlms {

struct GameFace{
  std::u16string id;
  std::u16string action = u"default";
  uint32_t time = 0;
  uint8_t index = 0;

  uint64_t destroy = 0;
};

}  // namespace sdlms
