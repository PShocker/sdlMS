#include "ladderrope_game_instance.h"
#include "src/client/game/game_ladderrope.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"

void ladderrope_game_instance::load(uint32_t map_id) {
  data.clear();
  auto map_node = wz_resource::load_map_node(map_id);
  auto map_ladder_node = map_node->get_child(u"ladderRope");
  for (auto [key, val] : *map_ladder_node->get_children()) {
    auto ladder_node = val[0];
    game_ladderrope g_ladderrope;
    g_ladderrope.l =
        static_cast<wz::Property<int> *>(ladder_node->get_child(u"l"))->get();
    g_ladderrope.uf =
        static_cast<wz::Property<int> *>(ladder_node->get_child(u"uf"))->get();
    g_ladderrope.page =
        static_cast<wz::Property<int> *>(ladder_node->get_child(u"page"))
            ->get();

    g_ladderrope.x =
        static_cast<wz::Property<int> *>(ladder_node->get_child(u"x"))->get();
    g_ladderrope.y1 =
        static_cast<wz::Property<int> *>(ladder_node->get_child(u"y1"))->get();
    g_ladderrope.y2 =
        static_cast<wz::Property<int> *>(ladder_node->get_child(u"y2"))->get();

    g_ladderrope.t = std::min(g_ladderrope.y1, g_ladderrope.y2);
    g_ladderrope.b = std::max(g_ladderrope.y1, g_ladderrope.y2);

    data.push_back(g_ladderrope);
  }
}