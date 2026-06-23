# distutils: language = c++
# cython: c_string_encoding=ascii, language_level=3

import sys

import numpy as np
cimport numpy as cnp
from cython.view cimport array

from libc.string cimport memcpy
from libc.stdint cimport uint32_t, uint64_t, uint16_t
from libcpp cimport bool
from libcpp.string cimport string

from qdmq.pyx.vipc cimport VisionIpcServer as cppVisionIpcServer
from qdmq.pyx.vipc cimport VisionIpcClient as cppVisionIpcClient
from qdmq.pyx.vipc cimport VisionBuf as cppVisionBuf
from qdmq.pyx.vipc cimport VisionIpcBufExtra
from qdmq.pyx.vipc cimport get_endpoint_name as cpp_get_endpoint_name


def get_endpoint_name(string name, VisionType stream):
    return cpp_get_endpoint_name(name, stream).decode('utf-8')

cpdef enum VisionType:
    VISION_SENSOR_MONO8
    VISION_SENSOR_MONO16
    #
    VISION_STREAM_MAX


cdef class VisionBuf:
    cdef cppVisionBuf * buf

    @staticmethod
    cdef create(cppVisionBuf * cbuf):
        buf = VisionBuf()
        buf.buf = cbuf
        return buf

    @property
    def data(self):
        return np.asarray(<cnp.uint8_t[:self.buf.len]> self.buf.addr)
    @property
    def width(self):
        return self.buf.width
    @property
    def height(self):
        return self.buf.height

cdef class VisionIpcServer:
    cdef cppVisionIpcServer * server

    def __init__(self, string name):
        self.server = new cppVisionIpcServer(name)

    def create_buffers(self, VisionType tp, size_t num_buffers, size_t width, size_t height):
        self.server.create_buffers(tp, num_buffers, width, height)

    def create_buffers_with_sizes(self, VisionType tp, size_t num_buffers, size_t width, size_t height, size_t size):
        self.server.create_buffers_with_sizes(tp, num_buffers, width, height, size)

    def send(self, VisionType tp, const unsigned char [:] data, uint32_t frame_id=0, uint64_t timestamp_sof=0, uint64_t timestamp_eof=0):
        cdef cppVisionBuf * buf = self.server.get_buffer(tp)

        assert buf.len == data.nbytes
        memcpy(buf.addr, &data[0], data.nbytes)
        buf.set_frame_id(frame_id)

        cdef VisionIpcBufExtra extra
        extra.frame_id = frame_id
        extra.timestamp_sof = timestamp_sof
        extra.timestamp_eof = timestamp_eof

        self.server.send(buf, &extra, False)

    def start_listener(self):
        self.server.start_listener()

    def __dealloc__(self):
        del self.server

cdef class VisionIpcClient:
    cdef cppVisionIpcClient * client
    cdef VisionIpcBufExtra extra

    def __cinit__(self, string name, VisionType stream, bool conflate):
        self.client = new cppVisionIpcClient(name, stream, conflate)

    def __dealloc__(self):
        del self.client

    @property
    def width(self):
        return self.client.buffers[0].width if self.client.num_buffers else None
    @property
    def height(self):
        return self.client.buffers[0].height if self.client.num_buffers else None
    @property
    def buffer_len(self):
        return self.client.buffers[0].len if self.client.num_buffers else None
    @property
    def num_buffers(self):
        return self.client.num_buffers
    @property
    def frame_id(self):
        return self.extra.frame_id
    @property
    def timestamp_sof(self):
        return self.extra.timestamp_sof
    @property
    def timestamp_eof(self):
        return self.extra.timestamp_eof
    @property
    def valid(self):
        return self.extra.valid
    @property
    def visiontype(self):
        return self.client.type

    def recv(self, int timeout_ms=100):
        buf = self.client.recv(&self.extra, timeout_ms)
        if not buf:
            return None
        return VisionBuf.create(buf)

    def connect(self, bool blocking):
        return self.client.connect(blocking)

    def is_connected(self):
        return self.client.is_connected()

    @staticmethod
    def available_streams(string name, bool block):
        return cppVisionIpcClient.getAvailableStreams(name, block)
