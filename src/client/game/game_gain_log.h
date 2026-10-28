#ifndef SDLMS_SRC_CLIENT_GAME_GAME_GAIN_LOG_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_GAIN_LOG_H_

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

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_GAIN_LOG_H_
