# distutils: language = c++
# cython: c_string_encoding=ascii, language_level=3

import sys

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool
from libc cimport errno
from libc.string cimport strerror
from cython.operator import dereference

from qdmq.pyx.ipc cimport Context as cppContext
from qdmq.pyx.ipc cimport SubSocket as cppSubSocket
from qdmq.pyx.ipc cimport PubSocket as cppPubSocket
from qdmq.pyx.ipc cimport Poller as cppPoller
from qdmq.pyx.ipc cimport Message as cppMessage


class IpcError(Exception):
    def __init__(self, endpoint=None):
        suffix = f"with {endpoint.decode('utf-8')}" if endpoint else ""
        message = f"Messaging failure {suffix}: {strerror(errno.errno).decode('utf-8')}"
        super().__init__(message)

class MultiplePublishersError(IpcError):
    pass

cdef class Context:
    cdef cppContext * context

    def __cinit__(self):
        self.context = cppContext.create()

    def term(self):
        del self.context
        self.context = NULL

    def __dealloc__(self):
        # Deleting the context will hang if sockets are still active
        # TODO: Figure out a way to make sure the context is closed last
        # del self.context
        pass

cdef class Poller:
    cdef cppPoller * poller
    cdef list sub_sockets

    def __cinit__(self):
        self.sub_sockets = []
        self.poller = cppPoller.create()

    def __dealloc__(self):
        del self.poller

    def registerSocket(self, SubSocket socket):
        self.sub_sockets.append(socket)
        self.poller.registerSocket(socket.socket)

    def poll(self, timeout):
        sockets = []
        cdef int t = timeout

        with nogil:
            result = self.poller.poll(t)

        for s in result:
            socket = SubSocket()
            socket.setPtr(s)
            sockets.append(socket)

        return sockets

cdef class SubSocket:
    cdef cppSubSocket * socket
    cdef bool is_owner

    def __cinit__(self):
        with nogil:
            self.socket = cppSubSocket.create()
        self.is_owner = True
        if self.socket == NULL:
            raise IpcError

    def __dealloc__(self):
        if self.is_owner:
            with nogil:
                del self.socket

    cdef setPtr(self, cppSubSocket * ptr):
        if self.is_owner:
            with nogil:
                del self.socket

        self.is_owner = False
        self.socket = ptr

    def connect(self, Context context, string endpoint, string address=b"127.0.0.1", bool conflate=False, int buf_size=-1):
        cdef int r
        with nogil:
            r = self.socket.connect(context.context, endpoint, address, conflate, True, buf_size)
        if r != 0:
            if errno.errno == errno.EADDRINUSE:
                raise MultiplePublishersError(endpoint)
            else:
                raise IpcError(endpoint)

    def setTimeout(self, int timeout):
        with nogil:
            self.socket.setTimeout(timeout)

    def receive(self, bool non_blocking=False):
        cdef cppMessage *msg
        with nogil:
            msg = self.socket.receive(non_blocking)
        if msg == NULL:
            return None
        else:
            sz = msg.getSize()
            m = msg.getData()[:sz]
            with nogil:
                del msg
        return m

    def get_endpoint(self) -> str:
        return self.socket.getEndpoint().decode('utf-8')

cdef class PubSocket:
    cdef cppPubSocket * socket

    def __cinit__(self):
        self.socket = cppPubSocket.create()
        if self.socket == NULL:
            raise IpcError

    def __dealloc__(self):
        del self.socket

    def connect(self, Context context, string endpoint, int buf_size=-1):
        r = self.socket.connect(context.context, endpoint, True, buf_size)

        if r != 0:
            if errno.errno == errno.EADDRINUSE:
                raise MultiplePublishersError(endpoint)
            else:
                raise IpcError(endpoint)

    def send(self, bytes data):
        length = len(data)
        r = self.socket.send(<char*>data, length)

        if r != length:
            if errno.errno == errno.EADDRINUSE:
                raise MultiplePublishersError
            else:
                raise IpcError

    def all_readers_updated(self):
        return self.socket.all_readers_updated()
