#ifndef SDLMS_SRC_SERVER_SERVER_MAIN_H_
#define SDLMS_SRC_SERVER_SERVER_MAIN_H_

#include <uv.h>
#include <cstdint>
#include <string>
namespace sdlms {

class ServerMain {
public:
  static inline uint64_t local_addr_;

  static inline uv_loop_t *loop_;
  static inline bool host_ = true;
  static inline sockaddr_in host_addr = {};

  static void ServerInit(const std::string &ip, uint32_t port);
  static void ServerInit();
  static bool ServerSend(const uint8_t *data, size_t len, sockaddr_in *addr);
  static bool ServerSend(const uint8_t *data, size_t len, uint64_t client_id);

  static bool Run();

private:
  static inline uv_udp_t local_socket_;
  static inline uint32_t local_port_;

  static uint64_t CombineIpPort(uint32_t ip, uint16_t port);
  static void SplitIpPort(uint64_t combined, uint32_t &ip, uint16_t &port);

  static void AllocCb(uv_handle_t *handle, size_t suggested_size,
                       uv_buf_t *buf);
  static void OnRecv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf,
                      const struct sockaddr *addr, unsigned flags);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_MAIN_H_
