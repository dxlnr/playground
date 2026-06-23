# distutils: language = c++
#cython: language_level=3

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.set cimport set
from libc.stdint cimport uint32_t, uint64_t
from libcpp cimport bool, int

cdef extern from "vbuf.h":
  cdef enum VisionType:
    pass

  cdef cppclass VisionBuf:
    void * addr
    size_t len
    size_t width
    size_t height
    size_t idx
    void set_frame_id(uint64_t id)

cdef extern from "vipc.h":
  struct VisionIpcBufExtra:
    uint32_t frame_id
    uint64_t timestamp_sof
    uint64_t timestamp_eof
    bool valid

cdef extern from "vipc_server.h":
  string get_endpoint_name(string, VisionType)

  cdef cppclass VisionIpcServer:
    VisionIpcServer(string)
    void create_buffers(VisionType, size_t, size_t, size_t)
    void create_buffers_with_sizes(VisionType, size_t, size_t, size_t, size_t)
    void send(VisionBuf *, VisionIpcBufExtra *, bool)
    void start_listener()
    VisionBuf * get_buffer(VisionType)

cdef extern from "vipc_client.h":
  cdef cppclass VisionIpcClient:
    int num_buffers
    VisionType type
    VisionBuf buffers[1]
    VisionIpcClient(string, VisionType, bool)
    VisionBuf * recv(VisionIpcBufExtra *, int)
    bool connect(bool)
    bool is_connected()
    @staticmethod
    set[VisionType] getAvailableStreams(string, bool)
