#pragma once

#include <cstdint>
class server_heartbeat_system {
public:
  static bool run_client_heartbeat();
  static bool run_server_heartbeat();         // server only
  
  static void receive_client_heartbeat(uint64_t client_id); 
  static void receive_server_heartbeat(); 
};