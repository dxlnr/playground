#pragma once

#include <cstdint>
#include <cstddef>

#include <atomic>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
typedef HANDLE shm_t;
#else
#include <sys/mman.h>
typedef int shm_t;
#endif

#define VISIONBUF_SYNC_FROM_DEVICE 0
#define VISIONBUF_SYNC_TO_DEVICE 1

enum VisionType {
  VISION_SENSOR_MONO8,
  VISION_SENSOR_MONO16,
  //
  VISION_STREAM_MAX,
};

class VisionBuf {
public:
  shm_t      fd       = 0;
  size_t     len      = 0;
  size_t     mmap_len = 0;
  void      *addr     = nullptr;
  uint64_t  *frame_id;
  size_t     width    = 0;
  size_t     height   = 0;
  uint8_t   *data     = nullptr;
  // IPC
  uint64_t   server_id = 0;
  size_t     idx = 0;
  VisionType type;

  void     allocate(size_t len);
  void     import();
  void     init_mono(size_t width, size_t height);
  int      free();
  void     set_frame_id(uint64_t id);
  uint64_t get_frame_id();
};

void visionbuf_compute_aligned_width_and_height(int width, int height, int *aligned_w, int *aligned_h);
void *malloc_with_fd(size_t len, shm_t *fd);
