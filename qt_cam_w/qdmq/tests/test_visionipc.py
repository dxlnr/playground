#!/usr/bin/env python

import sys
from pathlib import Path
import unittest
import numpy as np

sys.path.insert(0, str(Path(__file__).parent.parent))

from qdmq import VisionIpcServer, VisionIpcClient, VisionType


class TestVisionIpc(unittest.TestCase):
    def setUp(self):
        pass

    def setup_vipc(
        self, name, *stream_types, num_buffers=5, width=100, height=200, conflate=False
    ):
        server = VisionIpcServer(name)
        for stream_type in stream_types:
            server.create_buffers(stream_type, num_buffers, width, height)
        server.start_listener()

        if len(stream_types):
            client = VisionIpcClient(name, stream_types[0], conflate)
            assert client.connect(True)
        else:
            client = None
        return server, client

    def test_connect(self):
        _, c = self.setup_vipc(b"camerad", VisionType.VISION_SENSOR_MONO16)
        assert c.is_connected

    def test_buffers(self):
        width, height, num_buffers = 100, 200, 5
        _, c = self.setup_vipc(
            b"camerad",
            VisionType.VISION_SENSOR_MONO16,
            num_buffers=num_buffers,
            width=width,
            height=height,
        )
        assert c.width == width
        assert c.height == height
        assert c.buffer_len > 0
        assert c.num_buffers == num_buffers

    def test_single_buffer_uint8(self):
        width, height, num_buffers, frame_id = 100, 200, 2, 7
        s, c = self.setup_vipc(
            b"testd",
            VisionType.VISION_SENSOR_MONO8,
            num_buffers=num_buffers,
            width=width,
            height=height,
        )
        buf = np.random.randint(0, 256, height * width, dtype=np.uint8)
        s.send(VisionType.VISION_SENSOR_MONO8, buf, frame_id=frame_id)
        recv_buf = c.recv()
        assert np.all(recv_buf.data.view("<u1") == buf)

    def test_send_single_buffer(self):
        width, height, num_buffers, testd1, testd2, frame_id = (
            100,
            200,
            5,
            333,
            40_000,
            99,
        )
        s, c = self.setup_vipc(
            b"camerad",
            VisionType.VISION_SENSOR_MONO16,
            num_buffers=num_buffers,
            width=width,
            height=height,
        )
        buf = np.zeros(height * width, dtype=np.uint16)
        buf[0] = testd1
        buf[1] = testd2
        s.send(VisionType.VISION_SENSOR_MONO16, buf.tobytes(), frame_id=frame_id)

        recv_buf = c.recv()
        assert recv_buf is not None
        assert recv_buf.data.view("<u2")[0] == testd1
        assert recv_buf.data.view("<u2")[1] == testd2
        assert c.frame_id == frame_id

    def test_whole_buffer(self):
        width, height, num_buffers, frame_id = 100, 200, 5, 99
        s, c = self.setup_vipc(
            b"camerad",
            VisionType.VISION_SENSOR_MONO16,
            num_buffers=num_buffers,
            width=width,
            height=height,
        )
        buf = np.random.randint(0, 65536, height * width, dtype=np.uint16)
        s.send(VisionType.VISION_SENSOR_MONO16, buf.tobytes(), frame_id=frame_id)
        recv_buf = c.recv()
        assert np.all(recv_buf.data.view("<u2") == buf)


if __name__ == "__main__":
    unittest.main()
