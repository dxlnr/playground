#include "shmm.h"

/*
 * Check if `SHMM_MAGIC` is set (internal use only)
 **/
static inline bool shmm_is_valid(shmm_t *m)
{
  return m && m->magic == SHMM_MAGIC;
}

shmm_error_t shmm_new_buf(
  shmm_t *m,
  const char * path,
  size_t buf_size_bytes,
  uint32_t nonce
) {
  /*
   * Buffer must be smaller than max system memory capacity
   **/
  if (buf_size_bytes >= MEM_MAX_CAP) return SHMM_ERR_MEM_EXCEEDED;

  std::string fn = path;
  m->endpoint       = "/" + fn;
  m->sem_done_name  = "/" + fn + ".done";

  m->fd = shm_open(m->endpoint.c_str(), O_CREAT | O_EXCL | O_RDWR, 0640);
  if (m->fd < 0)
    return SHMM_ERR_IO;
  const size_t head_bytes = sizeof(shmm_header_t);
  m->payload_size         = buf_size_bytes;
  m->mmap_size            = head_bytes + PAGE_ALIGN(buf_size_bytes);

  int rc = ftruncate(m->fd, m->mmap_size);
  if (rc < 0) {
    close(m->fd);
    return SHMM_ERR_IO;
  }
  char * mem = (char*)mmap(NULL, m->mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, m->fd, 0);
  if (mem == MAP_FAILED)
    return SHMM_ERR_IO;

  m->mmap_ptr = mem;
  m->h        = reinterpret_cast<shmm_header_t*>(m->mmap_ptr);
  m->payload  = mem + head_bytes;
  m->h->ref_cnt.store(1, std::memory_order_release);
  m->h->payload_size.store(static_cast<uint64_t>(buf_size_bytes), std::memory_order_release);
  m->h->write_ok.store(0, std::memory_order_release);
  m->magic    = SHMM_MAGIC;
  /* 0 initialized */
  if (!m->payload || m->payload_size == 0)
    return SHMM_ERR_NO_INIT;
  memset(m->payload, 0, m->payload_size);
  /* semaphore */
  m->s_done = sem_open(m->sem_done_name.c_str(), O_CREAT|O_EXCL, 0600, 0);
  if (m->s_done == SEM_FAILED && errno == EEXIST)
  {
    sem_unlink(m->sem_done_name.c_str());
    m->s_done = sem_open(m->sem_done_name.c_str(), O_CREAT | O_EXCL, 0600, 0);
    if (m->s_done == SEM_FAILED) { return SHMM_ERR_SEM; }
  }
  /* buffer creation complete */
  m->h->magic = SHMM_MAGIC;
  m->h->nonce = nonce;

  return SHMM_ERR_OK;
}

shmm_error_t shmm_set_buf_shape(
  shmm_t *m,
  uint16_t dtype_size,
  uint16_t dtype_code,
  uint16_t n_dims,
  const uint16_t* dims
) {
  if (!shmm_is_valid(m))
    return SHMM_ERR_NO_INIT;
  if (n_dims > MAX_N_DIMS)
    return SHMM_ERR_SHAPE_INVALID;
  if (n_dims > 0 && !dims)
    return SHMM_ERR_SHAPE_INVALID;
  /* dims must not contain zeros */
  for (uint16_t i = 0; i < n_dims; ++i)
    if (dims[i] == 0)
      return SHMM_ERR_SHAPE_INVALID;

  m->h->dtype_size = dtype_size;
  m->h->dtype_code = dtype_code;
  m->h->n_dims     = n_dims;

  for (uint16_t i = 0; i < n_dims; ++i)
    m->h->dims[i] = dims[i];
  /* Fill unused with 0 for safety */
  for (uint16_t i = n_dims; i < MAX_N_DIMS; ++i)
    m->h->dims[i] = 0;

  return SHMM_ERR_OK;
}

shmm_error_t shmm_open_buf(shmm_t *m, const char * path)
{
  std::string fn = path;
  m->endpoint       = "/" + fn;
  m->sem_done_name  = "/" + fn + ".done";

  m->fd = shm_open(m->endpoint.c_str(), O_RDWR, 0600);
  if (m->fd < 0)
    return SHMM_ERR_NO_INIT;
  const size_t head_bytes = sizeof(shmm_header_t);
  void * h_map = mmap(nullptr, head_bytes, PROT_READ|PROT_WRITE, MAP_SHARED, m->fd, 0);
  if (h_map == MAP_FAILED)
    return SHMM_ERR_IO;
  m->h             = reinterpret_cast<shmm_header_t*>(h_map);
  uint64_t p_bytes = m->h->payload_size.load(std::memory_order_acquire);
  munmap(h_map, head_bytes);
  if (!p_bytes) return SHMM_ERR_IO;

  m->payload_size = p_bytes;
  m->mmap_size    = head_bytes + PAGE_ALIGN(m->payload_size);
  m->mmap_ptr     = (char*)mmap(nullptr, m->mmap_size, PROT_READ|PROT_WRITE, MAP_SHARED, m->fd, 0);
  if (m->mmap_ptr == MAP_FAILED)
    return SHMM_ERR_IO;

  m->h       = reinterpret_cast<shmm_header_t*>(m->mmap_ptr);
  m->payload = reinterpret_cast<char*>(m->mmap_ptr) + head_bytes;
  m->h->ref_cnt.fetch_add(1, std::memory_order_acq_rel);
  m->magic   = SHMM_MAGIC;
  /* semaphore */
  m->s_done  = sem_open(m->sem_done_name.c_str(), 0);
  if (m->s_done == SEM_FAILED) return SHMM_ERR_SEM;

  return SHMM_ERR_OK;
}

shmm_error_t shmm_poll_is_created(const char * path, int timeout_ms, uint32_t nonce)
{
  if (shmm_is_created(path, nonce)) return SHMM_ERR_OK;

  const int chunk_ms   = 100;  // poll granularity
  int64_t remaining_ms = (timeout_ms < 0) ? -1 : (int64_t)timeout_ms;

  while (!shmm_is_created(path, nonce)) {
    int ms;
    if (remaining_ms < 0) {
      ms = chunk_ms;
    } else {
      if (remaining_ms <= 0)
        return SHMM_ERR_TIMEDOUT;
      ms = (remaining_ms > chunk_ms) ? chunk_ms : (int)remaining_ms;
    }

    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;  // ms to ns

    if (nanosleep(&ts, NULL) == -1) {
      // signal interrupt
      if (errno == EINTR)
        return SHMM_ERR_INTERRUPTED;
      return SHMM_ERR_IO;
    }
    if (remaining_ms > 0)
      remaining_ms -= ms;
  }
  return SHMM_ERR_OK;
}

bool shmm_is_created(const char * path, uint32_t nonce)
{
  std::string fn    = path;
  std::string fpath = "/" + fn;
  int fd = shm_open(fpath.c_str(), O_RDWR, 0);
  /* all related errors are catched and returned as not created */
  if (fd < 0) return false;

  struct stat st;
  if (fstat(fd, &st) < 0) { close(fd); return false; }
  /* ensure header is initialized correctly */
  if (st.st_size < (off_t)sizeof(shmm_header_t)) { close(fd); return false; }

  void *h_ptr = mmap(nullptr, sizeof(shmm_header_t), PROT_READ, MAP_SHARED, fd, 0);
  if (h_ptr == MAP_FAILED) { close(fd); return false; }

  bool ok = false;
  auto *h        = static_cast<shmm_header_t *>(h_ptr);
  uint32_t magic = h->magic;
  ok             = (magic == SHMM_MAGIC);
  /* make sure the buffer is the correct one */
  uint32_t w_nonce = h->nonce;
  ok               = (w_nonce == nonce);
  /* clean up */
  munmap(h_ptr, sizeof(shmm_header_t));
  close(fd);

  if (ok) return true;
  return false;
}
/*
 * only used internally
 * when last buffer is dereferenced
**/
static inline void shmm_invalidate_buf(shmm_t *m)
{
  m->payload      = NULL;
  m->payload_size = 0;
  m->mmap_size    = 0;
  m->magic        = 0;
  m->h            = NULL;
}

shmm_error_t shmm_close_buf(shmm_t *m)
{
  if (!shmm_is_valid(m))
    return SHMM_ERR_OK;
  bool last = false;
  if (m->h)
  {
    uint32_t prev = m->h->ref_cnt.fetch_sub(1, std::memory_order_acq_rel);
    last = (prev == 1);
  }
  if (m->mmap_ptr) { munmap(m->mmap_ptr, m->mmap_size); m->mmap_ptr = nullptr; }
  if (m->fd >= 0)  { close(m->fd); m->fd = -1; }
  if (m->s_done != SEM_FAILED) { sem_close(m->s_done); m->s_done = SEM_FAILED; }
  if (last)
  {
    shmm_invalidate_buf(m);
    shm_unlink(m->endpoint.c_str());
    sem_unlink(m->sem_done_name.c_str());
  }
  return SHMM_ERR_OK;
}

shmm_error_t shmm_zero_init_payload(shmm_t *m)
{
  if (!m)
    return SHMM_ERR_NO_INIT;
  if (!shmm_is_valid(m))
    return SHMM_ERR_NO_INIT;
  if (!m->payload || m->payload_size == 0)
    return SHMM_ERR_NO_INIT;
  memset(m->payload, 0, m->payload_size);

  return SHMM_ERR_OK;
}

template<typename I, typename O>
shmm_error_t shmm_add_buf(shmm_t *m, shmm_data_t *d, size_t ix)
{
  if (!m || !d)
    return SHMM_ERR_NO_INIT;
  if (!shmm_is_valid(m))
    return SHMM_ERR_NO_INIT;
  if (!m->payload || m->payload_size == 0)
    return SHMM_ERR_NO_INIT;
  if (!d->data || d->n_bytes == 0)
    return SHMM_ERR_NO_INIT;
  if (d->n_bytes_dtype != sizeof(I))
    return SHMM_ERR_BAD_DTYPE;

  size_t n_elems        = d->n_bytes / sizeof(I);
  // bytes per destination block
  size_t dst_block_size = n_elems * sizeof(O);
  size_t offset_bytes   = ix * dst_block_size;

  if (offset_bytes + dst_block_size > m->payload_size)
  {
#ifdef DEBUG
    fprintf(stderr,
            "shmm_add_buf OOB: ix=%zu n_bytes=%zu offset=%zu payload_size=%zu\n",
            ix, d->n_bytes, offset_bytes, m->payload_size);
#endif
    return SHMM_ERR_OOB;
  }
  O *dst = (O*)((char*)m->payload + offset_bytes);
  I *src = (I*)d->data;

  for (size_t i = 0; i < n_elems; ++i)
    dst[i] += (O)src[i];

  return SHMM_ERR_OK;
}

template shmm_error_t shmm_add_buf<uint16_t, uint32_t>(shmm_t *m, shmm_data_t *d, size_t ix);
template shmm_error_t shmm_add_buf<uint16_t, uint16_t>(shmm_t *m, shmm_data_t *d, size_t ix);

template<typename O>
shmm_error_t shmm_get_dst_ptr(shmm_t *m, O **dst, size_t ix, size_t n_elems)
{
  if (!m)
    return SHMM_ERR_NO_INIT;
  if (!shmm_is_valid(m))
    return SHMM_ERR_NO_INIT;
  if (!m->payload || m->payload_size == 0)
    return SHMM_ERR_NO_INIT;

  size_t dst_block_size = n_elems * sizeof(O);
  size_t offset_bytes   = ix * dst_block_size;

  if (offset_bytes + dst_block_size > m->payload_size)
  {
#ifdef DEBUG
    fprintf(stderr,
            "shmm_get_dst_ptr OOB: ix=%zu offset=%zu payload_size=%zu\n",
            ix, offset_bytes, m->payload_size);
#endif
    return SHMM_ERR_OOB;
  }
  *dst = (O*)((char*)m->payload + offset_bytes);
  return SHMM_ERR_OK;
}
template shmm_error_t shmm_get_dst_ptr<float>(shmm_t *m, float **dst, size_t ix, size_t n_elems);

shmm_error_t shmm_write_done(shmm_t *m)
{
  if (!shmm_is_valid(m))
    return SHMM_ERR_NO_INIT;

  std::atomic_thread_fence(std::memory_order_release);
  m->h->done.store(1, std::memory_order_release);

  uint32_t n = m->h->ref_cnt.load(std::memory_order_acquire);
  /* inform all child procs
   *
   * (n-1) as creator is also ref counted
   **/
  for (uint32_t i = 0; i < n - 1; ++i)
    if (sem_post(m->s_done) != 0)
      return SHMM_ERR_SEM;
  return SHMM_ERR_OK;
}

shmm_error_t shmm_set_write_result(shmm_t *m, uint8_t write_ok)
{
  if (!shmm_is_valid(m))
    return SHMM_ERR_NO_INIT;
  std::atomic_thread_fence(std::memory_order_release);
  m->h->write_ok.store(write_ok, std::memory_order_release);
  return SHMM_ERR_OK;
}

shmm_error_t shmm_block_until_done(shmm_t *m, int timeout_ms)
{
  if (!shmm_is_valid(m))
    return SHMM_ERR_NO_INIT;

  if (timeout_ms == -1) {
    while(1) {
      /* if writer already set done, return immediately */
      if (m->h->done.load(std::memory_order_acquire))
        return SHMM_ERR_OK;
      /* BLOCK */
      if (sem_wait(m->s_done) == -1) {
        if (errno == EINTR)
          return SHMM_ERR_INTERRUPTED;  // or continue;
        return SHMM_ERR_SEM;
      }
      /* Woken up: check the flag again (handles spurious wake or early post) */
      if (m->h->done.load(std::memory_order_acquire))
        return SHMM_ERR_OK;
    }
  } else {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    ts.tv_sec  = timeout_ms / 1000;
    ts.tv_nsec = (timeout_ms % 1000) * 1000 * 1000;
    while(1) {
      // if writer already set done, return immediately
      if (m->h->done.load(std::memory_order_acquire))
        return SHMM_ERR_OK;
      // BLOCK
      if (sem_timedwait(m->s_done, &ts) == 0) {
        // Woken up: check the flag again (handles spurious wake or early post)
        if (m->h->done.load(std::memory_order_acquire))
          return SHMM_ERR_OK;
        continue;
      }
      if (errno == ETIMEDOUT)
        return SHMM_ERR_TIMEDOUT;
      if (errno == EINTR)
        return SHMM_ERR_SEM;
    }
  }
}

shmm_error_t shmm_read_write_ok_value(shmm_t *m, uint8_t& ok_value)
{
  if (!shmm_is_valid(m))
    return SHMM_ERR_NO_INIT;
  ok_value = m->h->done.load(std::memory_order_acquire);

  return SHMM_ERR_OK;
}

shmm_data_t to_shmm_data(void *ptr, size_t n_bytes, size_t n_bytes_dtype)
{
  shmm_data_t d;
  d.n_bytes       = n_bytes;
  d.n_bytes_dtype = n_bytes_dtype;
  d.data          = (char *)ptr;
  return d;
}

shmm_error_t shmm_force_cleanup(const char *endpoint)
{
  std::string fn        = endpoint;
  std::string shm_name  = "/" + fn;
  std::string sem_name  = "/sem.";
  sem_name += fn;
  sem_name += ".done";
  // unlink shm
  if (shm_unlink(shm_name.c_str()) == -1)
    if (errno != ENOENT)
      return SHMM_ERR_UNLINK;
  // unlink semaphore
  if (sem_unlink(sem_name.c_str()) == -1)
    if (errno != ENOENT)
      return SHMM_ERR_UNLINK;
  return SHMM_ERR_OK;
}
