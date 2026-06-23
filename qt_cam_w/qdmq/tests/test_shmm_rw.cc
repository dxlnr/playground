#include <chrono>
#include <unistd.h>
#include <sys/wait.h>
#include <thread>

#include "test_helper.h"

#include "qdmq/shmm/shmm_rw.h"

#define SHMM_TOPIC "test_shmm_buf"
#define W          32
#define H          32
#define F         401
#define N_FRAMES   10

class shmm_writer_test {
public:
  static void *get_shmm_payload_ptr(const shmm_writer_t &sw) {
    return sw._m.payload;
  }
};

inline size_t idx2(size_t r, size_t c, size_t C) { return (r * C + c); }

inline size_t idx4(size_t a, size_t A,
                   size_t b, size_t B,
                   size_t c, size_t C,
                   size_t d, size_t D)
{
  return (((a * B + b) * C + c) * D + d);
}

static bool _helper_shmm_payload_is_correct(const shmm_t* m, uint32_t value)
{
  uint32_t *dbuf = (uint32_t *)m->payload;
  for (size_t a = 0; a < 2; ++a) {
  for (size_t f = 0; f < F; ++f) {
  for (size_t h = 0; h < H; ++h) {
  for (size_t w = 0; w < W; ++w) {
    if (dbuf[idx4(a, 2, f, F, h, H, w, W)] != value) return false;
  }}}}
  return true;
}

int test_shmm_writer()
{
  shmm_error_t err;

  uint16_t im[H*W];
  for (size_t i = 0; i < H * W; ++i)
    im[i] = 256;

  // `uint32_t`
  shmm_writer_t sw;
  uint16_t dtype_code = 6;
  uint16_t dims[5]    = {2,F,1,H,W};
  err = sw.init(SHMM_TOPIC, sizeof(uint32_t), dtype_code, 5, dims);
  /* make sure reader can in fact create & open the buffer
   **/
  std::this_thread::sleep_for(std::chrono::milliseconds(425));

  TEST_ASSERT(err == SHMM_ERR_OK);
  if (err != SHMM_ERR_OK) { sw.close(); return 1; }
  // WRITE
  size_t idx;
  for (size_t f_step=0; f_step < F; ++f_step) {
    for (size_t n=0; n < N_FRAMES; ++n) {
      idx = idx2(n % 2, f_step, F);
      err = sw.add_u16_im_to_u32_buf(idx, &im[0], sizeof(im));
      if (err != SHMM_ERR_OK) return 1;
    }
  }
  sw.write_done();
  sw.close();
  return 0;
}

int test_shmm_reader()
{
  shmm_error_t err;
  shmm_t m;

  err = shmm_poll_is_created(SHMM_TOPIC, -1);
  TEST_ASSERT(err == SHMM_ERR_OK);
  if (err != SHMM_ERR_OK) return 1;
  err = shmm_open_buf(&m, SHMM_TOPIC);
  TEST_ASSERT(err == SHMM_ERR_OK);
  if (err != SHMM_ERR_OK) return 1;

  shmm_block_until_done(&m);

  TEST_ASSERT(_helper_shmm_payload_is_correct(&m, (N_FRAMES/2) * 256) == true);

  shmm_close_buf(&m);
  return 0;
}

static int wait_child_with_timeout(pid_t pid, int timeout_ms)
{
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
  for (;;)
  {
    int status = 0;
    pid_t r = waitpid(pid, &status, WNOHANG);
    if (r == pid) return status;
    if (r == -1) return -1;
    if (std::chrono::steady_clock::now() > deadline)
      return -2;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

void test_shmm_rw_add_to_buf(void)
{
  // READER
  pid_t reader_pid = fork();
  TEST_ASSERT(reader_pid >= 0);
  if (reader_pid == 0)
  {
    int rc = test_shmm_reader();
    _exit(rc); // IMPORTANT: _exit in child
  }
  // WRITER
  pid_t writer_pid = fork();
  TEST_ASSERT(writer_pid >= 0);

  if (writer_pid == 0)
  {
    int rc = test_shmm_writer();
    _exit(rc);
  }

  int writer_status = wait_child_with_timeout(writer_pid, /*timeout_ms=*/5000);
  int reader_status = wait_child_with_timeout(reader_pid, /*timeout_ms=*/5000);

  auto kill_if_alive = [](pid_t pid) {
    if (pid > 0) {
      kill(pid, SIGKILL);
      // reap
      int st = 0;
      waitpid(pid, &st, 0);
    }
  };

  if (writer_status == -2 || reader_status == -2)
    kill_if_alive(writer_pid); kill_if_alive(reader_pid);
}

void test_shmm_rw_get_data_ptr(void)
{
  shmm_error_t err;
  /* `float` */
  shmm_writer_t sw;
  /* SHMM_DTYPE_FLOAT32 */
  uint16_t dtype_code = 11;
  uint16_t dims[4]    = {6,60,H,W};
  err = sw.init(SHMM_TOPIC, sizeof(float), dtype_code, 4, dims);

  float *d_ptr;
  err = sw.get_data_ptr_f32(&d_ptr);
  TEST_ASSERT(err == SHMM_ERR_OK);
  void* t_ptr = shmm_writer_test::get_shmm_payload_ptr(sw);
  TEST_ASSERT(d_ptr == t_ptr);
  sw.close();
}
