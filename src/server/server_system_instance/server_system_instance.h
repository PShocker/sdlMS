#pragma once

class server_system_instance {
public:
  static void enter(bool host);
  static void create_client_heartbeat();
};