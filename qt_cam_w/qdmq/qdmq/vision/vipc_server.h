#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <map>
#include <cassert>
#include <random>
#include <limits>

#ifdef _WIN32
#else
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "ipc.h"
#include "vipc.h"
#include "vbuf.h"

std::string get_endpoint_name(std::string name, VisionType type);
std::string get_ipc_path(const std::string &name);

class VisionIpcServer {
 private:
  uint64_t server_id;

  bool connected = false;
  std::atomic<bool> should_exit = false;
  std::string name;
  std::thread listener_thread;

  std::map<VisionType, std::atomic<size_t>> cur_idx;
  std::map<VisionType, std::vector<VisionBuf*>> buffers;

  Context * msg_ctx;
  std::map<VisionType, PubSocket*> sockets;

  void listener(void);

 public:
  VisionIpcServer(std::string name);
  ~VisionIpcServer();

  VisionBuf * get_buffer(VisionType type);
  void destroy_buffer();

  void create_buffers(VisionType type, size_t num_buffers, size_t width, size_t height);
  void create_buffers_with_sizes(VisionType type, 
                                 size_t num_buffers, 
                                 size_t width, 
                                 size_t height, 
                                 size_t size); 
  void send(VisionBuf * buf, VisionIpcBufExtra * extra, bool sync=true);
  void start_listener();

  bool is_connected() { return connected; }
};
