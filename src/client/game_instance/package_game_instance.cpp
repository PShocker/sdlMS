#include "package_game_instance.h"
#include "src/client/game/game_item.h"

void package_game_instance::load() {
  equips[0] = {
      .id = u"01382004",
  };
  equips[1] = {
      .id = u"01372004",
  };
  equips[2] = {
      .id = u"01312002",
  };
  equips[3] = {
      .id = u"01402003",
  };
  cosumes[0] = {
      .id = u"04080000",
      .num = 1,
  };
  cosumes[1] = {
      .id = u"02000000",
      .num = 1,
  };
  cosumes[2] = {
      .id = u"02000001",
      .num = 1,
  };
}