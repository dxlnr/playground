#include <iostream>
#include <chrono>
#include <cassert>
#include <thread>
#include <unistd.h>

#include "vipc.h"
#include "vipc_client.h"
#include "vipc_server.h"

#ifdef _WIN32
static HANDLE connect_to_vipc_server(const std::string &name, bool blocking) 
{
  const std::string ipc_path = get_ipc_path(name);
  HANDLE h = ipc_connect(ipc_path.c_str());

  while (h == INVALID_HANDLE_VALUE && blocking) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    h = ipc_connect(ipc_path.c_str());
  }
  return h;
}
#else
static int connect_to_vipc_server(const std::string &name, bool blocking) 
{
  const std::string ipc_path = get_ipc_path(name);
  int socket_fd = ipc_connect(ipc_path.c_str());
  while (socket_fd < 0 && blocking) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    socket_fd = ipc_connect(ipc_path.c_str());
  }
  return socket_fd;
}
#endif

VisionIpcClient::VisionIpcClient(std::string name, VisionType type, bool conflate) : name(name), type(type) 
{
  msg_ctx = Context::create();
  sock = SubSocket::create(msg_ctx, get_endpoint_name(name, type), "127.0.0.1", conflate, false);

  poller = Poller::create();
  poller->registerSocket(sock);
}

// Connect is not thread safe. Do not use the buffers while calling connect
bool VisionIpcClient::connect(bool blocking) 
{
  connected = false;
  // Cleanup old buffers on reconnect
  for (size_t i = 0; i < num_buffers; i++) 
  {
    if (buffers[i].free() != 0) { std::cerr << "Failed to free buffer " << i; }
  }
  num_buffers = 0;

#ifdef _WIN32
  HANDLE h = connect_to_vipc_server(name, blocking);
  if (h == INVALID_HANDLE_VALUE) {
    return false;
  }
  // int r = ipc_sendrecv_with_fds(true, h, &type, sizeof(type), nullptr, 0, nullptr);
  // assert(r == sizeof(type));

  // HANDLE hs[VISIONIPC_MAX_FDS];
  // VisionBuf bufs[VISIONIPC_MAX_FDS];
  // r = ipc_sendrecv_with_fds(false, h, &bufs, sizeof(bufs), hs, VISIONIPC_MAX_FDS, &num_buffers);

  // assert(num_buffers >= 0);
  // assert(r == sizeof(VisionBuf) * num_buffers);

  // for (size_t i = 0; i < num_buffers; i++){
  //   buffers[i] = bufs[i];
  //   // TODO -> eliminate the fd
  //   buffers[i].fd = hs[i];
  //   buffers[i].import();
  //   buffers[i].init_mono(buffers[i].width, buffers[i].height);
  // }
  // TODO: CloseHandle(h);

  connected = true;
  return true;
#else
  int socket_fd = connect_to_vipc_server(name, blocking);
  if (socket_fd < 0) {
    return false;
  }
  // Send stream type to server to request FDs
  int r = ipc_sendrecv_with_fds(true, socket_fd, &type, sizeof(type), nullptr, 0, nullptr);
  assert(r == sizeof(type));

  int fds[VISIONIPC_MAX_FDS];
  VisionBuf bufs[VISIONIPC_MAX_FDS];
  r = ipc_sendrecv_with_fds(false, socket_fd, &bufs, sizeof(bufs), fds, VISIONIPC_MAX_FDS, &num_buffers);

  assert(num_buffers >= 0);
  assert(r == sizeof(VisionBuf) * num_buffers);

  for (size_t i = 0; i < num_buffers; i++){
    buffers[i] = bufs[i];
    buffers[i].fd = fds[i];
    buffers[i].import();
    buffers[i].init_mono(buffers[i].width, buffers[i].height);
  }
  close(socket_fd);
  connected = true;
  return true;
#endif
}

VisionBuf * VisionIpcClient::recv(VisionIpcBufExtra * extra, const int timeout_ms)
{
  auto p = poller->poll(timeout_ms);

  if (!p.size()) {
    return nullptr;
  }
  Message * r = sock->receive(true);
  if (r == nullptr) {
    return nullptr;
  }

  assert(r->getSize() == sizeof(VisionIpcPacket));
  VisionIpcPacket *packet = (VisionIpcPacket*)r->getData();

  assert(packet->idx < num_buffers);
  VisionBuf * buf = &buffers[packet->idx];

  if (buf->server_id != packet->server_id) {
    connected = false;
    delete r;
    return nullptr;
  }

  if (extra) {
    *extra = packet->extra;
  }
  delete r;
  return buf;
}

std::set<VisionType> VisionIpcClient::getAvailableStreams(const std::string &name, bool blocking) 
{
#ifdef _WIN32
  HANDLE h = connect_to_vipc_server(name, blocking);
  if (h == INVALID_HANDLE_VALUE) {
    return {};
  }
  // int request = VISION_STREAM_MAX;
  // int r = ipc_sendrecv_with_fds(true, h, &request, sizeof(request), nullptr, 0, nullptr);
  // assert(r == sizeof(request));

  // VisionType available_streams[VISION_STREAM_MAX] = {};
  // r = ipc_sendrecv_with_fds(false, h, &available_streams, sizeof(available_streams), nullptr, 0, nullptr);
  // assert((r >= 0) && (r % sizeof(VisionType) == 0));

  // TODO: CloseHandle(h);
  return {};
#else
  int socket_fd = connect_to_vipc_server(name, blocking);
  if (socket_fd < 0) {
    return {};
  }
  // Send VISION_STREAM_MAX to server to request available streams
  int request = VISION_STREAM_MAX;
  int r = ipc_sendrecv_with_fds(true, socket_fd, &request, sizeof(request), nullptr, 0, nullptr);
  assert(r == sizeof(request));

  VisionType available_streams[VISION_STREAM_MAX] = {};
  r = ipc_sendrecv_with_fds(false, socket_fd, &available_streams, sizeof(available_streams), nullptr, 0, nullptr);
  assert((r >= 0) && (r % sizeof(VisionType) == 0));
  close(socket_fd);
  return std::set<VisionType>(available_streams, available_streams + r / sizeof(VisionType));
#endif
}

VisionIpcClient::~VisionIpcClient() 
{
  for (size_t i = 0; i < num_buffers; i++)
  {
    if (buffers[i].free() != 0) { std::cerr << "Failed to free buffer " << i; }
  }
  delete sock;
  delete poller;
  delete msg_ctx;
}
