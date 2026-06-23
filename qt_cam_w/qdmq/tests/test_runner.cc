#include <stdlib.h>

#include "test_helper.h"
#include "test_msgq.h"
#include "test_vision.h"
#include "test_shmm.h"
#include "test_shmm_rw.h"

int total = 0;
int fails = 0;

int main() 
{
  printf("UNITTEST (qdmq)\n");
  printf("-------------------------------------------------------------------\n");
  // MSGQ
  test_msgq_msg_init_size();
  test_msgq_msg_init_data();
  test_msgq_init_subscriber();
  test_msgq_msg_send();
  test_msgq_msg_send_wraparound();
  test_msgq_write_read_msg();
  test_msgq_pub_slow_sub();
  // VISION
  test_mq_vision_vbuf();
  test_mq_vision_connecting();
  test_mq_vision_check_buffers();
  // SHMM
  test_shmm_new_buf_ok();
  test_shmm_new_buf_once_only();
  test_shmm_set_buf_shape();
  test_shmm_open_buf_no_buf_created();
  test_shmm_open_buf();
  test_shmm_add_buf_u16_to_u32();
  test_shmm_add_buf_u16_to_u32_5d();
  test_shmm_get_dst_ptr_f32();
  test_shmm_block_until_done_immediate();
  test_shmm_block_until_done();
  test_poll_buf_created();
  // SHMM RW
  test_shmm_rw_add_to_buf();
  test_shmm_rw_get_data_ptr();

  printf("\n");
  if (fails > 0) {
    printf(ANSI_COLOR_RED   "[tests] " ANSI_COLOR_RESET "Done: %d test(s) failed!\n", fails);
    printf("\n");
    return EXIT_FAILURE;
  } else {
    printf(ANSI_COLOR_GREEN "[tests] " ANSI_COLOR_RESET "Done: %d test(s) passed!\n", total);
    printf("\n");
    return EXIT_SUCCESS;
  }
}
