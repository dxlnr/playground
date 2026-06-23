#!/usr/bin/env python

import sys
import random
import unittest
import time
import string
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from qdmq import pub_sock, sub_sock


def random_sock():
    return "".join(random.choices(string.ascii_uppercase + string.digits, k=10))


def random_bytes(length=1000):
    return bytes([random.randrange(0xFF) for _ in range(length)])


class TestPubSub(unittest.TestCase):
    def test_pub_sub(self):
        sock = random_sock()
        p_sock = pub_sock(sock)
        s_sock = sub_sock(sock, conflate=False, timeout=None)
        time.sleep(0.8)

        for _ in range(200):
            m = random_bytes()
            p_sock.send(m)
            recvd = s_sock.receive()
            assert m == recvd


if __name__ == "__main__":
    unittest.main()
