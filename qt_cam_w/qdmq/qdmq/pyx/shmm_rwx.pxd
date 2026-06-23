# distutils: language = c++
# cython: language_level=3

from libc.stdint cimport uint8_t, uint16_t, uint32_t

cdef extern from "shmm/shmm.h":
    ctypedef int shmm_error_t

cdef extern from "shmm/shmm.h":
    ctypedef enum shmm_dtype_t:
        SHMM_DTYPE_UINT8   =  2
        SHMM_DTYPE_INT8    =  1
        SHMM_DTYPE_UINT16  =  4
        SHMM_DTYPE_INT16   =  3
        SHMM_DTYPE_UINT32  =  6
        SHMM_DTYPE_INT32   =  5
        SHMM_DTYPE_UINT64  =  8
        SHMM_DTYPE_INT64   =  7
        SHMM_DTYPE_FLOAT32 = 11
        SHMM_DTYPE_FLOAT64 = 12

cdef extern from "shmm/shmm_rw.h":
    ctypedef struct shmm_shape_t:
        uint16_t dtype_size
        uint16_t dtype_code
        uint16_t n_dims
        uint16_t dims[5]

    cdef cppclass shmm_reader_t:
        shmm_reader_t() except +
        shmm_error_t poll_open(const char *fn, uint32_t nonce, int timeout_ms) nogil
        shmm_error_t block_until_done() nogil
        shmm_error_t close() nogil
        uint8_t get_write_success() nogil

        const void* data() const
        size_t size_bytes() const

        shmm_shape_t get_shape()
