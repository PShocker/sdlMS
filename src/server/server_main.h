#pragma once

#include "flatbuffers/flatbuffer_builder.h"
#include "uv.h"
#include <cstdint>
#include <string>
class server_main {
public:
  static inline bool host = true;
  static inline sockaddr_in host_addr = {};

  static void server_init_heartbeat();
  static void server_init(const std::string &ip, uint32_t port);
  static void server_init();
  static bool server_send(const uint8_t *data, size_t len, sockaddr_in *addr);
  static bool server_send(const uint8_t *data, size_t len, uint64_t client_id);

  static void server_run();

private:
  static inline uv_loop_t *loop;
  static inline uv_udp_t local_socket;
  static inline uint32_t local_port;

  static void heartbeat_cb(uv_timer_t *handle);

  static uint64_t combine_ip_port(uint32_t ip, uint16_t port);
  static void split_ip_port(uint64_t combined, uint32_t &ip, uint16_t &port);

  static void alloc_cb(uv_handle_t *handle, size_t suggested_size,
                       uv_buf_t *buf);
  static void on_recv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf,
                      const struct sockaddr *addr, unsigned flags);
};