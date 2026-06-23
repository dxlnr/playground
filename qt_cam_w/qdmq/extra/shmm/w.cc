#include <cstring>

#include "../../qdmq/shmm/shmm.h"
#include "../../qdmq/shmm/shmm_rw.h"

#define W         960
#define H         960
#define F        401
#define N_FRAMES  10

inline size_t idx2(size_t r, size_t c, size_t C) { return (r * C + c); }

int main() {

  shmm_error_t err;

  uint16_t im[H*W];
  for (size_t r=0; r < H; ++r) {
    for (size_t c=0; c < W; ++c) {
      im[r * W + c] = 256;
    }
  }
  /* DTYPE `uint32_t` */
  shmm_writer_t sw;
  uint16_t dtype_code = 6;
  uint16_t dims[5]    = {2,F,1,H,W};
  err = sw.init("vbuf", sizeof(uint32_t), dtype_code, 5, dims, 1234);
  if (err != SHMM_ERR_OK)
  {
    printf("(error) Initializing ESR failed: %d\n", err);
    sw.close();
    return 1;
  } else { printf("ESR initialized. Data streaming will be performed\n"); }
  // WRITE
  size_t idx;
  for (size_t f_step=0; f_step < F; ++f_step) {
    for (size_t n=0; n < N_FRAMES; ++n) {
      idx = idx2(n % 2, f_step, F);
      err = sw.add_u16_im_to_u32_buf(idx, &im[0], sizeof(im));
      if (err != SHMM_ERR_OK) printf("found something: %d\n", err);
    }
  }
  sw.write_success();
  sw.write_done();
  sw.close();
  return 0;
}
