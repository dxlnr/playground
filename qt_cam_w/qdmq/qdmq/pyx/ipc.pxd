# distutils: language = c++
#cython: language_level=3

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool


cdef extern from "ipc.h":
  cdef cppclass Context:
    @staticmethod
    Context * create()

  cdef cppclass Message:
    void init(size_t)
    void init(char *, size_t)
    void close()
    size_t getSize()
    char *getData()

  cdef cppclass SubSocket:
    @staticmethod
    SubSocket * create() nogil
    int connect(Context *, string, string, bool, bool, int) nogil
    Message * receive(bool) nogil
    void setTimeout(int) nogil
    string getEndpoint()

  cdef cppclass PubSocket:
    @staticmethod
    PubSocket * create()
    int connect(Context *, string, bool, int)
    int sendMessage(Message *)
    int send(char *, size_t)
    bool all_readers_updated()

  cdef cppclass Poller:
    @staticmethod
    Poller * create()
    void registerSocket(SubSocket *)
    vector[SubSocket*] poll(int) nogil
