#ifndef SDLMS_SRC_CLIENT_GAME_GAME_ITEM_BUFF_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_ITEM_BUFF_H_

#include <cstdint>
#include <string>
namespace sdlms {

struct GameItemBuff{
  std::u16string id;
  uint64_t destroy = 0;
  uint64_t duration;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_ITEM_BUFF_H_
