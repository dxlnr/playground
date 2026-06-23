#include <chrono>
#include <thread>

#include "test_helper.h"

#include "qdmq/shmm/shmm.h"

#define W 32
#define H 32

const char *FN = "test_shmm";

static size_t idx2(size_t r, size_t c, size_t C) { return (r * C + c); }

static size_t idx3(size_t a, size_t A,
                   size_t b, size_t B,
                   size_t c, size_t C)
{
  return (a * B + b) * C + c;
}

static size_t idx4(size_t a, size_t A,
                   size_t b, size_t B,
                   size_t c, size_t C,
                   size_t d, size_t D)
{
  return (((a * B + b) * C + c) * D + d);
}

static bool _helper_shmm_payload_is_zero(const shmm_t* m)
{
  const unsigned char* p = (const unsigned char*)m->payload;
  for (size_t i = 0; i < m->payload_size; ++i)
    if (p[i] != 0)
      return false;
  return true;
}

static bool _helper_all_equal_uint16(
  const uint16_t *arr,
  const uint16_t *correct,
  size_t n)
{
  for (size_t i = 1; i < n; ++i)
    if (arr[i] != correct[i])
      return false;
  return true;
}

void test_shmm_new_buf_ok(void)
{
#ifdef _WIN32
  // TODO ?
#elif __APPLE__
  // TODO ?
#else
  remove("/dev/shm/test_shmm");
  remove("/dev/shm/sem.test_shmm.done");
#endif
  shmm_t m;

  size_t n_bytes = 4096;
  shmm_error_t err = shmm_new_buf(&m, FN, n_bytes);
  TEST_ASSERT(err == SHMM_ERR_OK);
  // mmap sanity
  TEST_ASSERT(m.mmap_ptr != NULL);
  TEST_ASSERT(m.mmap_size >= sizeof(shmm_header_t) + n_bytes);
  // header sanity
  TEST_ASSERT(m.h != NULL);
  TEST_ASSERT(m.h->payload_size == n_bytes);
  // payload sanity
  TEST_ASSERT(m.payload != NULL);
  TEST_ASSERT(m.payload_size == n_bytes);
  // header must be before payload and contiguous
  TEST_ASSERT((char*)m.h < m.payload);
  TEST_ASSERT((char*)m.h + sizeof(shmm_header_t) == m.payload);
  // ref count initially 0 or 1 depending on design
  TEST_ASSERT(m.h->ref_cnt.load() >= 0);
  // `write_ok` is 0
  TEST_ASSERT(m.h->write_ok.load() == 0);
  // semaphore initialized
  int t_val;
  sem_getvalue(m.s_done, &t_val);
  TEST_ASSERT(t_val == 0);
  // zero initialized
  TEST_ASSERT(_helper_shmm_payload_is_zero(&m) == true);
  // MAGIC
  TEST_ASSERT(m.magic == SHMM_MAGIC);
  // CLEAN & RM
  shmm_close_buf(&m);
}

void test_shmm_new_buf_once_only(void)
{
#ifdef _WIN32
  // TODO ?
#elif __APPLE__
  // TODO ?
#else
  remove("/dev/shm/test_shmm");
  remove("/dev/shm/sem.test_shmm.done");
#endif
  shmm_error_t err;
  shmm_t m;

  size_t n_bytes = 4096;
  err = shmm_new_buf(&m, FN, n_bytes);
  TEST_ASSERT(err == SHMM_ERR_OK);
  err = shmm_new_buf(&m, FN, n_bytes);
  TEST_ASSERT(err == SHMM_ERR_IO);
  // CLEAN & RM
  shmm_close_buf(&m);
}

void test_shmm_set_buf_shape(void)
{
#ifdef _WIN32
  // TODO ?
#elif __APPLE__
  // TODO ?
#else
  remove("/dev/shm/test_shmm");
  remove("/dev/shm/sem.test_shmm.done");
#endif
  shmm_error_t err;
  shmm_t m;

  uint16_t _0_dtype_size = sizeof(uint32_t);
  /* SHMM_DTYPE_UINT32  =  6*/
  uint16_t _0_dtype_code = 6;
  uint16_t _0_n_dims     = 5;
  uint16_t _0_dims[5]    = {2, 100, 1, 32, 32};

  uint16_t _1_dtype_size = sizeof(uint32_t);
  uint16_t _1_dtype_code = 6;
  uint16_t _1_n_dims     = 6;
  uint16_t _1_dims[6]    = {1, 2, 100, 1, 32, 32};

  err = shmm_set_buf_shape(&m, _0_dtype_size, _0_dtype_code, _0_n_dims, _0_dims);
  TEST_ASSERT(err == SHMM_ERR_NO_INIT);

  size_t n_bytes = 4096;
  err = shmm_new_buf(&m, FN, n_bytes);
  
  err = shmm_set_buf_shape(&m, _0_dtype_size, _0_dtype_code, _0_n_dims, _0_dims);
  TEST_ASSERT(err == SHMM_ERR_OK);
  TEST_ASSERT(m.h->n_dims == _0_n_dims);
  TEST_ASSERT(m.h->dtype_size == _0_dtype_size);
  TEST_ASSERT(_helper_all_equal_uint16(m.h->dims, _0_dims, 5) == true);

  err = shmm_set_buf_shape(&m, _1_dtype_size, _1_dtype_code, _1_n_dims, _1_dims);
  TEST_ASSERT(err == SHMM_ERR_SHAPE_INVALID);
  // CLEAN & RM
  shmm_close_buf(&m);
}

void test_shmm_open_buf_no_buf_created(void)
{
#ifdef _WIN32
  // TODO ?
#elif __APPLE__
  // TODO ?
#else
  remove("/dev/shm/test_shmm");
  remove("/dev/shm/sem.test_shmm.done");
#endif
  shmm_t m;

  shmm_error_t err = shmm_open_buf(&m, FN);
  TEST_ASSERT(err == SHMM_ERR_NO_INIT);
  // CLEAN & RM
  shmm_close_buf(&m);
}

void test_shmm_open_buf(void)
{
#ifdef _WIN32
  // TODO ?
#elif __APPLE__
  // TODO ?
#else
  remove("/dev/shm/test_shmm");
  remove("/dev/shm/sem.test_shmm.done");
#endif
  shmm_error_t err;
  shmm_t m;

  size_t n_bytes = 4096;
  // might fail silently
  err = shmm_new_buf(&m, FN, n_bytes);

  err = shmm_open_buf(&m, FN);
  TEST_ASSERT(err == SHMM_ERR_OK);
  // test is payload (size) was converted correctly
  TEST_ASSERT(m.payload_size == n_bytes);
  // CLEAN & RM
  shmm_close_buf(&m);
}

void test_shmm_add_buf_u16_to_u32(void)
{
#ifdef _WIN32
  // TODO ?
#elif __APPLE__
  // TODO ?
#else
  remove("/dev/shm/test_shmm");
  remove("/dev/shm/sem.test_shmm.done");
#endif
  shmm_error_t err;
  shmm_t m;

  // 8192B         nim H   W   dtype;
  size_t n_bytes = 2 * H * W * sizeof(uint32_t);
  // might fail silently
  err = shmm_new_buf(&m, FN, n_bytes);
  // fill an image
  uint16_t im0[H*W];
  uint16_t im1[H*W];
  for (size_t r=0; r < H; ++r)
  {
    for (size_t c=0; c < W; ++c)
    {
      im0[r * W + c] = 256;
      im1[r * W + c] = 64;
    }
  }
  shmm_data_t d0 = to_shmm_data(im0, sizeof(im0), sizeof(uint16_t));
  shmm_data_t d1 = to_shmm_data(im1, sizeof(im1), sizeof(uint16_t));

  for (size_t n=0; n < 2; ++n)
    for (size_t i=0; i < 2; ++i)
      if (n == 0)
        err = shmm_add_buf<uint16_t, uint32_t>(&m, &d0, 0);
      else
        err = shmm_add_buf<uint16_t, uint32_t>(&m, &d1, 1);
  TEST_ASSERT(err == SHMM_ERR_OK);
  shmm_write_done(&m);

  uint32_t *res = (uint32_t *)m.payload;
  TEST_ASSERT(res[idx3(0,2,0,H,0,W)] == 512);
  TEST_ASSERT(res[idx3(1,2,0,H,0,W)] == 128);
  // CLEAN & RM
  shmm_close_buf(&m);
}

void test_shmm_add_buf_u16_to_u32_5d(void)
{
#ifdef _WIN32
  // TODO ?
#elif __APPLE__
  // TODO ?
#else
  remove("/dev/shm/test_shmm");
  remove("/dev/shm/sem.test_shmm.done");
#endif
  shmm_error_t err;
  shmm_t m;

  // 819_200B         nim d1    H   W   dtype;
  size_t n_bytes = 2 * 100 * H * W * sizeof(uint32_t);
  // might fail silently
  err = shmm_new_buf(&m, FN, n_bytes);
  // apply shape
  uint16_t n_dims        = 5;
  uint16_t dtype_code = 6;
  uint16_t dims[5]       = {2,100,1,H,W};
  shmm_set_buf_shape(&m, sizeof(uint32_t), dtype_code, n_dims, dims);
  // fill an image
  uint16_t im[H*W];
  for (size_t r=0; r < H; ++r)
  {
    for (size_t c=0; c < W; ++c)
    {
      im[r * W + c] = 128;
    }
  }
  shmm_data_t d = to_shmm_data(im, sizeof(im), sizeof(uint16_t));

  size_t idx;
  for (size_t n=0; n < 100; ++n)
  {
    for (size_t i=0; i < 10; ++i)
    {
      idx = idx2(i % 2, n, 100);
      err = shmm_add_buf<uint16_t, uint32_t>(&m, &d, idx);
    }
 }
  TEST_ASSERT(err == SHMM_ERR_OK);
  shmm_write_done(&m);

  uint32_t *res = (uint32_t *)m.payload;
  TEST_ASSERT(res[idx4(0,2,12,100,0,H,0,W)] == 640);
  // CLEAN & RM
  shmm_close_buf(&m);
}

void test_shmm_get_dst_ptr_f32(void)
{
#ifdef _WIN32
  // TODO ?
#elif __APPLE__
  // TODO ?
#else
  remove("/dev/shm/test_shmm");
  remove("/dev/shm/sem.test_shmm.done");
#endif
  shmm_error_t err;
  shmm_t m;

  size_t n_bytes = 18 * H * W * sizeof(float);
  err = shmm_new_buf(&m, FN, n_bytes);
  TEST_ASSERT(err == SHMM_ERR_OK);
  float *dst;
  err = shmm_get_dst_ptr(&m, &dst, 4, H*W);
  TEST_ASSERT(dst == (float*)m.payload + H * W * sizeof(float));
  /* overflowing index */
  size_t t_idx = 24;
  err = shmm_get_dst_ptr(&m, &dst, t_idx, H*W);
  TEST_ASSERT(err == SHMM_ERR_OOB);

  size_t t_idx_2 = 16;
  err = shmm_get_dst_ptr(&m, &dst, t_idx_2, H*W);
  TEST_ASSERT(H * W * t_idx_2 == dst-(float*)m.payload);
  // CLEAN & RM
  shmm_close_buf(&m);
}

void test_shmm_block_until_done_immediate(void)
{
#ifdef _WIN32
  // TODO ?
#elif __APPLE__
  // TODO ?
#else
  remove("/dev/shm/test_shmm");
  remove("/dev/shm/sem.test_shmm.done");
#endif
  shmm_error_t err;
  shmm_t m;

  size_t n_bytes = 4096;
  err = shmm_block_until_done(&m);
  TEST_ASSERT(err == SHMM_ERR_NO_INIT);
  err = shmm_new_buf(&m, FN, n_bytes);
  TEST_ASSERT(err == SHMM_ERR_OK);
  err = shmm_write_done(&m);
  TEST_ASSERT(err == SHMM_ERR_OK);
  err = shmm_block_until_done(&m, 200);
  TEST_ASSERT(err == SHMM_ERR_OK);
  // CLEAN & RM
  shmm_close_buf(&m);
}

void test_shmm_block_until_done(void)
{
#ifdef _WIN32
  // TODO ?
#elif __APPLE__
  // TODO ?
#else
  remove("/dev/shm/test_shmm");
  remove("/dev/shm/sem.test_shmm.done");
#endif
  shmm_error_t err;
  shmm_t m;

  size_t n_bytes = 4096;
  shmm_new_buf(&m, FN, n_bytes);

  std::thread worker([&]{
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // Mark as done
    err = shmm_write_done(&m);
  });
  err = shmm_block_until_done(&m, 500);
  worker.join();

  TEST_ASSERT(err == SHMM_ERR_OK);
  // CLEAN & RM
  shmm_close_buf(&m);
}

void test_poll_buf_created(void)
{
#ifdef _WIN32
  // TODO ?
#elif __APPLE__
  // TODO ?
#else
  remove("/dev/shm/test_shmm");
  remove("/dev/shm/sem.test_shmm.done");
#endif
  shmm_error_t err;
  shmm_t m;

  err = shmm_poll_is_created(FN, 100);
  TEST_ASSERT(err == SHMM_ERR_TIMEDOUT);

  std::thread worker([&]{
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // create buf
    size_t n_bytes = 4096;
    shmm_new_buf(&m, FN, n_bytes);
  });
  err = shmm_poll_is_created(FN, 800);
  worker.join();

  TEST_ASSERT(err == SHMM_ERR_OK);
  // CLEAN & RM
  shmm_close_buf(&m);
}
