#include "server_client_instance.h"
#include "server_scene_instance.h"
#include <ranges>

void ServerClientInstance::ClientExit(uint64_t client_id) {
  if (clients_.contains(client_id)) {
    auto map_id = clients_[client_id].map_id;
    auto &map_clients = ServerSceneInstance::scenes_.at(map_id).clients;
    map_clients.erase(map_id);
    ServerClientInstance::clients_.erase(client_id);
  }
  return;
}