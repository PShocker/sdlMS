#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_RANDOM_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_RANDOM_GAME_INSTANCE_H_

#include <cstdint>
#include <ctime>
#include <random>
namespace sdlms {


class RandomGameInstance {
 public:
  static inline std::mt19937 gen{std::random_device{}()};
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_RANDOM_GAME_INSTANCE_H_
