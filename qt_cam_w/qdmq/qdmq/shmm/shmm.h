/* SHMM_H
 *
 * 1. `mmap` the shared buffer
 * 2. Store a pointer to the payload (void* payload)
 * 3. Use pointer arithmetic or structured view to write directly into shared buffer
 * 4. When done: done = 1, sem_post (using sempaphores to time the reading)
 * 5. Reader waits, then reads directly from the same memory
 **/
#ifndef SHMM_H
#define SHMM_H

#include <string>
#include <atomic>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <poll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

#define SHMM_MAGIC 0x4e7b64b0

#define PAGE_SIZE   4096u  // 4kB
#define PAGE_ALIGN(n) ((((size_t)(n)) + (PAGE_SIZE - 0)) & ~((size_t)(PAGE_SIZE - 1)))

#define MAX_N_DIMS  5

static const uint64_t MEM_MAX_CAP = 96ULL * 1024 * 1024 * 1024;

typedef enum {
  SHMM_ERR_OK            =     0,
  /* Shared memory or header not initialized */
  SHMM_ERR_NO_INIT       = -6001,
  /* Maximum memory limits exceeded when allocating */
  SHMM_ERR_MEM_EXCEEDED  = -6002,
  /* I/O error during read/write/mmap/sem open */
  SHMM_ERR_IO            = -6003,
  /* Out-of-bounds access to payload */
  SHMM_ERR_OOB           = -6004,
  /* Unsupported or mismatched data type */
  SHMM_ERR_BAD_DTYPE     = -6005,
  /* Semaphore operation failed */
  SHMM_ERR_SEM           = -6006,
  /* Writer has not signalled completion yet
   * but reader is trying to acquire
   **/
  SHMM_ERR_NOT_DONE      = -6007,
  SHMM_ERR_SHAPE_INVALID = -6008,
  SHMM_ERR_TIMEDOUT      = -6009,
  /* Action was interrupted by external signal */
  SHMM_ERR_INTERRUPTED   = -6010,
  SHMM_ERR_UNLINK        = -6011
} shmm_error_t;

/* dtype encoding for reader of shared memory
 **/
typedef enum {
  SHMM_DTYPE_UINT8   =  2,
  SHMM_DTYPE_INT8    =  1,
  SHMM_DTYPE_UINT16  =  4,
  SHMM_DTYPE_INT16   =  3,
  SHMM_DTYPE_UINT32  =  6,
  SHMM_DTYPE_INT32   =  5,
  SHMM_DTYPE_UINT64  =  8,
  SHMM_DTYPE_INT64   =  7,
  SHMM_DTYPE_FLOAT32 = 11,
  SHMM_DTYPE_FLOAT64 = 12
} shmm_dtype_t;

/* sizeof: 44 bytes
 **/
typedef struct shmm_header {
  /* guarantee writer fully initialized shmm region */
  uint32_t              magic;
  /* guarantee the correct shared memory location */
  uint32_t              nonce;
  std::atomic<uint64_t> payload_size;
  std::atomic<uint16_t> ref_cnt;
  std::atomic<uint8_t>  done;
  /* Indicate success when `done` */
  std::atomic<uint8_t>  write_ok;
  uint16_t dtype_size;
  /* shmm_dtype_t - numpy-compatible type code */
  uint16_t dtype_code;
  uint16_t n_dims;
  uint16_t dims[MAX_N_DIMS];
} shmm_header_t;

typedef struct shmm {
  /* MEMORY LAYOUT
   *
   * mmap_ptr>            payload>
   *          | header (metadata) | payload (buffer) |
   **/
  int    fd            = -1;
  void*  mmap_ptr      = nullptr;
  size_t mmap_size     = 0;

  shmm_header_t* h     = nullptr;
  void*   payload      = nullptr;
  size_t  payload_size = 0;
  /* semaphores */
  sem_t* s_done = SEM_FAILED;
  /* file names */
  std::string endpoint;
  std::string sem_done_name;
  /* guardrails to local memory */
  uint32_t magic;
} shmm_t;

typedef struct shmm_data {
  size_t n_bytes;
  size_t n_bytes_dtype;
  void * data;
} shmm_data_t;

/* Create new shared buffer on the system
 *
 * @param path           filename on the system under `/dev/shm`
 * @param buf_size_bytes defines the size of the payload in bytes
 * @param nonce          uniquely identifies the shared memory location
 * */
shmm_error_t shmm_new_buf             (shmm_t *m, const char * path, size_t buf_size_bytes, uint32_t nonce=0);
/* Open an already created (`shmm_new_buf`) buffer  */
shmm_error_t shmm_open_buf            (shmm_t *m, const char * path);
shmm_error_t shmm_set_buf_shape       (shmm_t *m,
                                       uint16_t dtype_size,
                                       uint16_t dtype_code,
                                       uint16_t n_dims,
                                       const uint16_t* dims);
shmm_error_t shmm_poll_is_created     (const char * path, int timeout_ms, uint32_t nonce=0);
bool         shmm_is_created          (const char * path, uint32_t nonce=0);
shmm_error_t shmm_close_buf           (shmm_t *m);
/* Use when leftover files blocking new attempt
 *
 * only works if `m` is valid
 **/
shmm_error_t shmm_force_cleanup       (const char *endpoint);
/* `memset` payload to zero */
shmm_error_t shmm_zero_init_payload   (shmm_t *m);
/* Add data slice to buffer
 *
 * Dependent on the specific data type that should get conveyed
 * as typeless raw memory is used for the shared memory region
 *
 * typename I  Input type
 * typename O Output type
 **/
template<typename I, typename O>
shmm_error_t shmm_add_buf             (shmm_t *m, shmm_data_t *d, size_t ix);
template<typename O>
shmm_error_t shmm_get_dst_ptr         (shmm_t *m, O **dst, size_t ix, size_t n_elems);
shmm_error_t shmm_write_done          (shmm_t *m);
shmm_error_t shmm_set_write_result    (shmm_t *m, uint8_t write_ok);
shmm_error_t shmm_block_until_done    (shmm_t *m, int timeout_ms=-1);

shmm_error_t shmm_read_write_ok_value (shmm_t *m, uint8_t& ok_value);
shmm_data_t  to_shmm_data             (void *ptr, size_t n_bytes, size_t n_bytes_dtype);

#endif /* SHMM_H */
