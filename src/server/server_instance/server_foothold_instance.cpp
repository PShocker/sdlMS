#include "server_foothold_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"

std::flat_map<int32_t, server_foothold>
server_foothold_instance::load_server_fh(uint32_t map_id) {
  std::flat_map<int32_t, server_foothold> fhs;
  auto data = foothold_game_instance::load(map_id);
  for (auto [k, v] : data) {
    server_foothold m_fh{
        .id = v.id,
        .page = v.page,
        .zmass = v.zmass,
        .prev = v.prev,
        .next = v.next,
        .x1 = v.x1,
        .y1 = v.y1,
        .x2 = v.x2,
        .y2 = v.y2,
        .l = v.l,
        .r = v.r,
        .t = v.t,
        .b = v.b,
        .k = v.k,
        .intercept = v.intercept,
    };
    fhs[k] = m_fh;
  }
  return fhs;
}