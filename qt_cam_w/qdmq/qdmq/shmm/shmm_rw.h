#ifndef SHMM_RW_H
#define SHMM_RW_H

#include <cstdint>
#include <stdio.h>

#include "shmm.h"

typedef struct shmm_shape {
  uint16_t dtype_size;
  uint16_t dtype_code;  /* shmm_dtype_t - NumPy-compatible type code */
  uint16_t n_dims;
  uint16_t dims[5];
} shmm_shape_t;

/*
 * `server` wrapper over shmm_t for writing the data
 **/
typedef struct shmm_writer {

  shmm_error_t init(
    const char * fn,
    uint16_t dtype_size,
    uint16_t dtype_code,
    uint16_t n_dims,
    const uint16_t* dims,
    uint32_t nonce=0);
  shmm_error_t add_u16_im_to_u32_buf(size_t idx, uint16_t *im, size_t n_bytes_im);
  shmm_error_t get_data_ptr_f32     (float **d_ptr);
  /* indicates that writing was successful */
  shmm_error_t write_success();
  /* informs reader(s) when writing is done */
  shmm_error_t write_done();
  shmm_error_t close();

private:
  shmm_t _m;

  /* expose access when testing */
  friend class shmm_writer_test;
} shmm_writer_t;

/*
 * `client` wrapper over shmm_t for receiving the data
 **/
typedef struct shmm_reader {
  /* Poll until opening the buffer is possible and then open
   *
   * @param timeout_ms Default is -1 which will poll forever
   **/
  shmm_error_t poll_open(const char *fn, uint32_t nonce=0, int timeout_ms=-1);
  shmm_error_t block_until_done();
  shmm_error_t close();
  /* Check if writer indicates success */
  uint8_t      get_write_success();
  /* data access
   *
   * returns raw pointer to payload
   * */
  const void * data() const;
  /* Size (n bytes) of payload */
  size_t       size_bytes() const;
  /* Retrieve the target shape set by writer */
  shmm_shape_t get_shape();

private:
  shmm_t _m;

} shmm_reader_t;

#endif /* SHMM_RW_H */
