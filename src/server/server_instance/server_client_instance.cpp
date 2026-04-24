#include "server_client_instance.h"
#include "server_scene_instance.h"
#include <ranges>

void server_client_instance::client_exit(uint64_t client_id) {
  if (clients.contains(client_id)) {
    auto map_id = clients[client_id].map_id;
    auto &map_clients = server_scene_instance::scenes.at(map_id).clients;
    map_clients.erase(map_id);
    for (auto &c : map_clients | std::views::values) {
        
    }
    server_client_instance::clients.erase(client_id);
  }
  return;
}