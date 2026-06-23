#include <cstring>
#include <chrono>
#include <thread>

#include "../qdmq/shmm/shmm.h"
#include "../qdmq/shmm/shmm_rw.h"

#define W         96
#define H         96
#define F        401
#define N_FRAMES   2

inline size_t idx2(size_t r, size_t c, size_t C) { return (r * C + c); }

int main() {
  shmm_error_t err;

  uint16_t im[H*W];
  for (size_t r=0; r < H; ++r) {
    for (size_t c=0; c < W; ++c) {
      im[r * W + c] = 256;
    }
  }
  /* dtype `uint32_t` */
  shmm_writer_t sw;
  uint16_t dtype_code = SHMM_DTYPE_UINT32;
  uint16_t dims[5]    = {2,F,1,H,W};
  err = sw.init("vbuf", sizeof(uint32_t), dtype_code, 5, dims);
  if (err != SHMM_ERR_OK)
  {
    sw.close();
    return 1;
  }
  /* allow the reader to catch up */
  std::this_thread::sleep_for(std::chrono::milliseconds(400));
  size_t idx;
  for (size_t f_step=0; f_step < F; ++f_step) {
    for (size_t n=0; n < N_FRAMES; ++n) {
      idx = idx2(n % 2, f_step, F);
      err = sw.add_u16_im_to_u32_buf(idx, &im[0], sizeof(im));
      if (err != SHMM_ERR_OK)
        return 1;
    }
  }
  sw.write_success();
  sw.write_done();
  sw.close();
  /* success */
  return 0;
}
