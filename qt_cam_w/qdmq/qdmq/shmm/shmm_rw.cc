#include "shmm_rw.h"

/*
 * WRITER
 **/
shmm_error_t shmm_writer::init(
  const char * fn,
  uint16_t dtype_size,
  uint16_t dtype_code,
  uint16_t n_dims,
  const uint16_t* dims,
  uint32_t nonce
) {
  shmm_error_t err;
  /* clean (potential) leftovers */
  err = shmm_force_cleanup(fn);
  if (err != SHMM_ERR_OK) { return err; }

  size_t n_bytes  = (size_t)dtype_size;
  for (uint16_t i = 0; i < n_dims; ++i)
    n_bytes *= (size_t)dims[i];
  // create new buffer
  err = shmm_new_buf(&_m, fn, n_bytes, nonce);
  if (err != SHMM_ERR_OK) { return err; }
  // if shape is invalid opt out and clean up buffer
  err = shmm_set_buf_shape(&_m, dtype_size, dtype_code, n_dims, dims);
  if (err != SHMM_ERR_OK)
  {
    shmm_close_buf(&_m); return err;
  }
  return SHMM_ERR_OK;
}

shmm_error_t shmm_writer::add_u16_im_to_u32_buf(
  size_t idx,
  uint16_t *im,
  size_t n_bytes_im)
{
  shmm_data_t d = to_shmm_data(im, n_bytes_im, sizeof(uint16_t));
  return shmm_add_buf<uint16_t, uint32_t>(&_m, &d, idx);
}

shmm_error_t shmm_writer::get_data_ptr_f32(float **d_ptr)
{
  return shmm_get_dst_ptr(&_m, d_ptr, 0, 0);
}

shmm_error_t shmm_writer::write_success() { return shmm_set_write_result(&_m, 1); }
shmm_error_t shmm_writer::write_done()    { return shmm_write_done(&_m); }
shmm_error_t shmm_writer::close()         { return shmm_close_buf(&_m); }
/*
 * READER
 **/
shmm_error_t shmm_reader::poll_open(const char *fn, uint32_t nonce, int timeout_ms)
{
  shmm_error_t err;
  err = shmm_poll_is_created(fn, timeout_ms, nonce);
  if (err != SHMM_ERR_OK) return err;
  err = shmm_open_buf(&_m, fn);
  if (err != SHMM_ERR_OK) return err;
  return SHMM_ERR_OK;
}
const void* shmm_reader::data() const  { return static_cast<const void*>(_m.payload); }
size_t shmm_reader::size_bytes() const { return _m.payload_size;  }

shmm_error_t shmm_reader::block_until_done()
{
  return shmm_block_until_done(&_m);
}

uint8_t shmm_reader::get_write_success()
{
  uint8_t ok_value;
  shmm_error_t err = shmm_read_write_ok_value(&_m, ok_value);
  if (err != SHMM_ERR_OK) return 0;
  else                    return 1;
}

shmm_error_t shmm_reader::close() { return shmm_close_buf(&_m); }

shmm_shape_t shmm_reader::get_shape()
{
  shmm_shape_t s;
  s.dtype_size = _m.h->dtype_size;
  s.dtype_code = _m.h->dtype_code;
  s.n_dims     = _m.h->n_dims;
  for (uint16_t i = 0; i < _m.h->n_dims; ++i)
    s.dims[i] = _m.h->dims[i];
  return s;
}
