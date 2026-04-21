#pragma once

#include <cstdint>
class server_heartbeat_system {
public:
  static bool client_heartbeat();
  static bool server_heartbeat();         // server only
  
  static void receive_server_heartbeat(); 
};