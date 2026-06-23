# distutils: language = c++
# cython: c_string_encoding=ascii, language_level=3

from qdmq.pyx.shmm_rwx cimport shmm_reader_t, shmm_shape_t, shmm_error_t
from qdmq.pyx.shmm_rwx cimport (
    SHMM_DTYPE_UINT8, SHMM_DTYPE_INT8,
    SHMM_DTYPE_UINT16, SHMM_DTYPE_INT16,
    SHMM_DTYPE_UINT32, SHMM_DTYPE_INT32,
    SHMM_DTYPE_UINT64, SHMM_DTYPE_INT64,
    SHMM_DTYPE_FLOAT32, SHMM_DTYPE_FLOAT64
)

import numpy as np
cimport numpy as cnp


cdef int _dtype_code_to_numpy(uint16_t dtype_code) except -1:
    """Map shmm_dtype_t to NumPy type number."""
    if dtype_code == SHMM_DTYPE_UINT8:
        return cnp.NPY_UINT8
    elif dtype_code == SHMM_DTYPE_INT8:
        return cnp.NPY_INT8
    elif dtype_code == SHMM_DTYPE_UINT16:
        return cnp.NPY_UINT16
    elif dtype_code == SHMM_DTYPE_INT16:
        return cnp.NPY_INT16
    elif dtype_code == SHMM_DTYPE_UINT32:
        return cnp.NPY_UINT32
    elif dtype_code == SHMM_DTYPE_INT32:
        return cnp.NPY_INT32
    elif dtype_code == SHMM_DTYPE_UINT64:
        return cnp.NPY_UINT64
    elif dtype_code == SHMM_DTYPE_INT64:
        return cnp.NPY_INT64
    elif dtype_code == SHMM_DTYPE_FLOAT32:
        return cnp.NPY_FLOAT32
    elif dtype_code == SHMM_DTYPE_FLOAT64:
        return cnp.NPY_FLOAT64
    else:
        raise RuntimeError(f"Unknown dtype_code: {dtype_code}")

cdef class ShmmReader:
    cdef shmm_reader_t _c

    def __cinit__(self):
        self._c = shmm_reader_t()

    def __dealloc__(self):
        # Ensure resources are freed exactly once
        with nogil:
            self._c.close()

    def poll_open(self, fn: str, uint32_t nonce = 0, int timeout_ms = -1) -> int:
        cdef bytes fn_bytes = fn.encode("utf-8")
        cdef const char* c_fn = fn_bytes
        cdef shmm_error_t err

        with nogil:
            err = self._c.poll_open(c_fn, nonce, timeout_ms)
        return <int>err

    def block_until_done(self) -> int:
        cdef shmm_error_t err
        with nogil:
             err = self._c.block_until_done()
        return <int>err

    def get_success_value(self) -> bool:
        cdef uint8_t ok_value
        with nogil:
            ok_value = self._c.get_write_success()
        return ok_value != 0

    def close(self) -> int:
        cdef shmm_error_t err
        with nogil:
            err = self._c.close()
        return <int>err

    def to_numpy(self) -> np.ndarray:
        # data
        cdef const void* ptr = self._c.data()
        # size
        cdef size_t n_bytes = self._c.size_bytes()
        # array shape
        cdef shmm_shape_t s = self._c.get_shape()

        if ptr == NULL or n_bytes == 0:
            raise RuntimeError("No payload mapped")

        cdef cnp.npy_intp dims[5]
        for idx, d in enumerate(s.dims):
            dims[idx] = d

        cdef int numpy_dtype = _dtype_code_to_numpy(s.dtype_code)

        cdef cnp.ndarray raw = cnp.PyArray_SimpleNewFromData(
            s.n_dims,       # number of dimensions
            dims,           # pointer to dims array
            numpy_dtype,    # dtype (dynamic based on writer)
            <void*>ptr      # raw data pointer
        )
        return raw
