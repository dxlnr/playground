#pragma once

#include <cstdint>
#include <cstddef>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#endif

constexpr int VISIONIPC_MAX_FDS = 128;

#ifdef _WIN32
HANDLE ipc_connect          (const char* pipe_name);
HANDLE ipc_bind             (const char* pipe_name);
int    ipc_sendrecv_with_fds(bool send, 
                             HANDLE h, 
                             void *buf, 
                             size_t buf_size, 
                             HANDLE *hs,
                             int num_h,
                             int *out_num_hs);

#else
int ipc_connect          (const char* socket_path);
int ipc_bind             (const char* socket_path);
int ipc_sendrecv_with_fds(bool send,
                          int fd,
                          void *buf,
                          size_t buf_size,
                          int *fds,
                          int num_fds,
                          int *out_num_fds);
#endif

struct VisionIpcBufExtra
{
  uint32_t frame_id;
  uint64_t timestamp_sof;
  uint64_t timestamp_eof;
  bool     valid;
};

struct VisionIpcPacket
{
  uint64_t server_id;
  size_t idx;
  struct VisionIpcBufExtra extra;
};
