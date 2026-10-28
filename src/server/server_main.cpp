#include "server_main.h"
#include "SDL3/SDL_timer.h"
#include "request_handler.h"
#include "server_system_instance/server_system_instance.h"
#include "src/common/request/client_request.h"
#include <cstdint>
#include <ctime>
namespace sdlms {


// 分配缓冲区回调
void ServerMain::AllocCb(uv_handle_t *handle, size_t suggested_size,
                           uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}

// 将uint32 IP和uint16端口组合为uint64
uint64_t ServerMain::CombineIpPort(uint32_t ip, uint16_t port) {
  // 将ip放入高32位，port放入低32位
  return ((uint64_t)ip << 32) | port;
}

// 分解uint64为IP和端口
void ServerMain::SplitIpPort(uint64_t combined, uint32_t &ip,
                                uint16_t &port) {
  ip = (uint32_t)(combined >> 32);      // 取高32位
  port = (uint16_t)(combined & 0xFFFF); // 取低16位（注意是16位，不是32位）
}

// 接收回调：当收到数据时被调用
void ServerMain::OnRecv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf,
                          const struct sockaddr *addr, unsigned flags) {
  if (nread < 0) {
    fprintf(stderr, "Read error: %s\n", uv_err_name(nread));
    uv_close((uv_handle_t *)handle, NULL);
    free(buf->base);
    return;
  }
  if (nread == 0) {
    // 没有数据或收到空包
    free(buf->base);
    return;
  }

  auto addr_in = (const struct sockaddr_in *)addr;
  auto ip = (uint32_t)(addr_in->sin_addr.s_addr);
  auto port = ntohs(addr_in->sin_port);
  auto client_id = CombineIpPort(ip, port);

  RequestHandler::HandleRequest(client_id, buf->base, nread);

  char sender_ip[17] = {0};
  uint16_t sender_port = 0;
  if (addr) {
    uv_ip4_name((const struct sockaddr_in *)addr, sender_ip, 16);
    sender_port = ntohs(((const struct sockaddr_in *)addr)->sin_port);
  }
  printf("Received %zd bytes from %s:%d\n", nread, sender_ip, sender_port);

  free(buf->base); // 释放由 alloc_cb 分配的内存
}

void ServerMain::ServerInit() {
  loop_ = uv_default_loop();
  // 1. 创建UDP句柄
  uv_udp_init(loop_, &local_socket_);

  // 绑定到 IPv4 和 IPv6
  struct sockaddr_in local_addr;
  uv_ip4_addr("0.0.0.0", 0, &local_addr);
  auto r = uv_udp_bind(&local_socket_, (const struct sockaddr *)&local_addr, 0);
  if (r < 0) {
    std::abort();
  }
  // 4. 获取系统实际分配的端口号
  struct sockaddr_in assigned_addr;
  int32_t namelen = sizeof(assigned_addr);
  r = uv_udp_getsockname(&local_socket_, (struct sockaddr *)&assigned_addr,
                         &namelen);
  if (r) {
    fprintf(stderr, "Getsockname error: %s\n", uv_strerror(r));
    std::abort();
  }
  local_port_ = ntohs(assigned_addr.sin_port);

  if (host_addr.sin_port == 0) {
    uv_ip4_addr("127.0.0.1", local_port_, &host_addr);
  }
  if (!host_) {
    // 连接到指定的远程地址
    uv_udp_connect(&local_socket_, (const struct sockaddr *)&host_addr);
  }
  // 开始接收数据
  r = uv_udp_recv_start(&local_socket_, AllocCb, OnRecv);
  if (r < 0) {
    fprintf(stderr, "Recv start error: %s\n", uv_err_name(r));
    std::abort();
  }
  printf("Started receiving on port %d...\n", local_port_);

  ServerSystemInstance::Enter(host_);
}

void ServerMain::ServerInit(const std::string &ip, uint32_t port) {
  host_ = false;
  uv_ip4_addr(ip.c_str(), port, &host_addr);
  ServerInit();
}

bool ServerMain::ServerSend(const uint8_t *data, size_t len,
                              sockaddr_in *addr) {
  auto buffer = (uint8_t *)malloc(len);
  memcpy(buffer, data, len);
  uv_udp_send_t *send_req = (uv_udp_send_t *)malloc(sizeof(uv_udp_send_t));
  send_req->data = buffer;

  uv_buf_t buf = uv_buf_init((char *)buffer, len);

  // 使用在 main 中初始化好的目标地址 send_addr
  auto r =
      uv_udp_send(send_req, &local_socket_, &buf, 1, (const sockaddr *)addr,
                  [](uv_udp_send_t *req, int status) {
                    if (status < 0) {
                      fprintf(stderr, "Send error: %s\n", uv_strerror(status));
                    }
                    free(req->data);
                    free(req); // 释放发送请求
                  });
  if (r < 0) {
    fprintf(stderr, "uv_udp_send error: %s\n", uv_strerror(r));
    free(send_req);
    return false;
  }
  return true;
}

bool ServerMain::ServerSend(const uint8_t *data, size_t len,
                              uint64_t client_id) {
  uint32_t ip;
  uint16_t port;
  SplitIpPort(client_id, ip, port);

  sockaddr_in send_addr;
  send_addr.sin_family = AF_INET;
  send_addr.sin_port = htons(port);
  send_addr.sin_addr.s_addr = ip;
  return ServerSend(data, len, &send_addr);
}

bool ServerMain::Run() {
  uv_run(loop_, UV_RUN_NOWAIT);
  return true;
}

}  // namespace sdlms
