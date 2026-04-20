#include "npc_logic_system.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/window/window.h"
#include "wz/Property.h"
#include <string>

bool npc_logic_system::run() {
  for (auto &m : npc_game_instance::data) {
    for (auto &g_npc : m) {
      g_npc.ani_time += window::delta_time;
      auto npc_node = npc_game_instance::load_link_npc_node(g_npc.id);
      auto action_node = npc_node->get_child(g_npc.action);

      auto texture_node =
          action_node->get_child(std::to_string(g_npc.ani_index));

      int32_t delay = 100;
      auto delay_node = texture_node->get_child(u"delay");
      if (delay_node != nullptr) {
        delay = static_cast<wz::Property<int> *>(delay_node)->get();
      }
      if (g_npc.ani_time >= delay) {
        g_npc.ani_time = 0;
        g_npc.ani_index++;
        g_npc.ani_index = g_npc.ani_index % action_node->children_count();
      }
    }
  }
  return true;
}