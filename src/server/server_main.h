#pragma once

#include "client_instance.h"
#include "flatbuffers/flatbuffer_builder.h"
#include "uv.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>
class server_main {
public:
  static inline flatbuffers::FlatBufferBuilder builder;
  static inline bool host = true;
  static inline sockaddr_in host_addr = {};
  static inline uint64_t host_heartbeat;

  const static uint8_t heartbeat_interval = 5;

  static inline std::flat_map<uint64_t, client_instance> clients;
  static inline std::flat_map<uint32_t, std::flat_set<uint64_t>> scene_clients;
  static inline std::flat_map<uint32_t, uint64_t> scene_host;

  static void server_init_heartbeat();
  static void server_init(const std::string &ip, uint32_t port);
  static void server_init();
  static bool server_send(const uint8_t *data, size_t len,
                          sockaddr_in send_addr);

private:
  static inline uv_loop_t *loop = nullptr;
  static inline uv_udp_t local_socket = {};
  static inline uint32_t local_port = 0;

  static void heartbeat_cb(uv_timer_t *handle);

  static uint64_t combine_ip_port(uint32_t ip, uint16_t port);

  static void alloc_cb(uv_handle_t *handle, size_t suggested_size,
                       uv_buf_t *buf);
  static void on_recv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf,
                      const struct sockaddr *addr, unsigned flags);
};