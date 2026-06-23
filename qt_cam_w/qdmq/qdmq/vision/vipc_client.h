#pragma once

#include <set>
#include <string>

#include "ipc.h"
#include "vipc.h"
#include "vbuf.h"


class VisionIpcClient {
private:
  std::string name;
  Context    *msg_ctx;
  SubSocket  *sock;
  Poller     *poller;

public:
  bool       connected   = false;
  int        num_buffers = 0;
  VisionType type;
  VisionBuf buffers[VISIONIPC_MAX_FDS];

  VisionIpcClient(std::string name, VisionType type, bool conflate);
  ~VisionIpcClient();

  VisionBuf *recv(VisionIpcBufExtra * extra=nullptr, const int timeout_ms=100);
  bool connect(bool blocking=true);
  bool is_connected() { return connected; }

  static std::set<VisionType> getAvailableStreams(const std::string &name, bool blocking = true);
};
