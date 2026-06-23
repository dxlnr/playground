#include <unistd.h>

#include "../../qdmq/shmm/shmm.h"

#define W  64
#define H  64
#define F 401
#define N_FRAMES 10

inline size_t idx4(size_t a, size_t A,
                   size_t b, size_t B,
                   size_t c, size_t C,
                   size_t d, size_t D)
{
  return (((a * B + b) * C + c) * D + d);
}

int main() {
  shmm_error_t err;
  shmm_t m;

  err = shmm_poll_is_created("vbuf", -1);
  err = shmm_open_buf(&m, "vbuf");
  if (err != SHMM_ERR_OK) { printf("%d\n", err); }

  printf("(reader) Waiting for data\n");
  shmm_block_until_done(&m);
  printf("(reader) Received data\n---------------------\n");

  uint16_t *dbuf = (uint16_t *)m.payload;

  printf("(reader) [0,100, 0, 0] | %d vs (%d)\n", dbuf[idx4(0,2,100,F,0,H,0,W)], 5);
  printf("(reader) [1,100, 0, 0] | %d vs (%d)\n", dbuf[idx4(1,2,100,F,0,H,0,W)], 10);
  printf("(reader) [0,120,16,16] | %d vs (%d)\n", dbuf[idx4(0,2,120,F,16,H,16,W)], 5);
  printf("(reader) [1,120,16,16] | %d vs (%d)\n", dbuf[idx4(1,2,120,F,16,H,16,W)], 10);
  printf("(reader) [0, 92,16,30] | %d vs (%d)\n", dbuf[idx4(0,2,92,F,16,H,30,W)], 5);
  printf("(reader) [1, 92,16,30] | %d vs (%d)\n", dbuf[idx4(1,2,92,F,16,H,30,W)], 10);

  shmm_close_buf(&m);
  return 0;
}
