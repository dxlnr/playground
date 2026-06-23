from typing import Optional, List

from .ipcx import (
    Context,
    Poller,
    SubSocket,
    PubSocket,
    MultiplePublishersError,
    IpcError,
)
from .vipcx import (
    VisionBuf,
    VisionIpcClient,
    VisionIpcServer,
    VisionType,
    get_endpoint_name,
)
from .shmm_rwx import ShmmReader

context = Context()


def pub_sock(endpoint: str, buf_size: Optional[int] = None) -> PubSocket:
    """Create a new publisher socket"""
    sock = PubSocket()
    buf_size = buf_size if buf_size is not None else -1
    sock.connect(context, endpoint, buf_size)
    return sock


def sub_sock(
    endpoint: str,
    poller: Optional[Poller] = None,
    addr: str = "127.0.0.1",
    conflate: bool = False,
    timeout: Optional[int] = None,
    buf_size: Optional[int] = None
) -> SubSocket:
    """Create a new subscriber socket"""
    sock = SubSocket()
    buf_size = buf_size if buf_size is not None else -1
    sock.connect(context, endpoint, addr.encode("utf8"), conflate, buf_size)

    if timeout is not None:
        sock.setTimeout(timeout)
    if poller is not None:
        poller.registerSocket(sock)
    return sock


def drain_sock_raw(sock: SubSocket, wait_for_one: bool = False) -> List[bytes]:
    """Receive all message currently available on the queue"""
    ret: List[bytes] = []
    while True:
        if wait_for_one and len(ret) == 0:
            dat = sock.receive()
        else:
            dat = sock.receive(non_blocking=True)
        if dat is None:
            break
        ret.append(dat)
    return ret


__all__ = [
    "Context",
    "Poller",
    "SubSocket",
    "PubSocket",
    "MultiplePublishersError",
    "IpcError",
    "pub_sock",
    "sub_sock",
    "drain_sock_raw",
    "context",
    # VIPC
    "VisionBuf",
    "VisionIpcClient",
    "VisionIpcServer",
    "VisionType",
    "get_endpoint_name",
    # SHMM_RW
    "ShmmReader"
]
