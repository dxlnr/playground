#include "vbuf.h"

#define ALIGN(x, align) (((x) + (align)-1) & ~((align)-1))

std::atomic<int> offset = 0;

void *malloc_with_fd(size_t len, shm_t *fd)
{
  if (len == 0) { return NULL; }

  char full_path[0x100];
#ifdef _WIN32
  DWORD pid = (DWORD)GetCurrentProcessId();
  _snprintf_s(full_path, sizeof(full_path), _TRUNCATE, "Local\\visionbuf_%lu_%d", (unsigned long)pid, offset++);

  DWORD l32 =  (DWORD)(len         & 0xFFFFFFFFu);
  DWORD h32 =  (DWORD)((len >> 32) & 0xFFFFFFFFu);
  *fd = CreateFileMappingA(
    INVALID_HANDLE_VALUE,
    NULL,
    PAGE_READWRITE,
    h32,
    l32,
    full_path
  );
  if (*fd == NULL) { return NULL; }

  void *addr = MapViewOfFile(
    *fd,
    FILE_MAP_WRITE | FILE_MAP_READ,
    0,
    0,
    len
  );
  if (addr == NULL) {
    CloseHandle(*fd);
    return NULL;
  }
#elif __APPLE__
  snprintf(full_path, sizeof(full_path)-1, "/tmp/visionbuf_%d_%d", getpid(), offset++);
#else
  snprintf(full_path, sizeof(full_path)-1, "/dev/shm/qdm/visionbuf_%d_%d", getpid(), offset++);
#endif

#ifdef _WIN32
#else
  *fd = open(full_path, O_RDWR | O_CREAT, 0664);
  assert(*fd >= 0);

  unlink(full_path);

  ftruncate(*fd, len);
  void *addr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0);
  assert(addr != MAP_FAILED);
#endif
  return addr;
}

void visionbuf_compute_aligned_width_and_height(int width, int height, int *aligned_w, int *aligned_h) 
{
  *aligned_w = width;
  *aligned_h = height;
}

void VisionBuf::init_mono(size_t init_width, size_t init_height) 
{
  this->width  = init_width;
  this->height = init_height;
  this->data   = (uint8_t *)this->addr;
}

uint64_t VisionBuf::get_frame_id()            { return *frame_id; }
void     VisionBuf::set_frame_id(uint64_t id) { *frame_id = id; }

void VisionBuf::allocate(size_t length) 
{
  this->len = length;
  this->mmap_len = this->len + sizeof(uint64_t);
  this->addr = malloc_with_fd(this->mmap_len, &this->fd);
  this->frame_id = (uint64_t*)((uint8_t*)this->addr + this->len);
}

int VisionBuf::free()
{
  int err = 0;
#ifdef _WIN32
  if (this->addr) {
    UnmapViewOfFile(this->addr);
    this->addr = nullptr;
  }
  if (this->fd) {
    CloseHandle(this->fd);
    this->fd = nullptr;
  }
#else
  err = munmap(this->addr, this->mmap_len);
  if (err != 0) 
    return err;
  err = close(this->fd);
#endif
  return err;
}

void VisionBuf::import()
{
#ifdef _WIN32
  assert(this->fd != nullptr);

  this->addr = MapViewOfFile(
    this->fd,
    FILE_MAP_READ | FILE_MAP_WRITE,
    0,
    0,
    this->mmap_len
  );
  assert(this->addr != nullptr);

  this->frame_id = reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(this->addr) + this->len);
#else
  assert(this->fd >= 0);
  this->addr = mmap(NULL, this->mmap_len, PROT_READ | PROT_WRITE, MAP_SHARED, this->fd, 0);
  assert(this->addr != MAP_FAILED);

  this->frame_id = (uint64_t*)((uint8_t*)this->addr + this->len);
#endif
}
