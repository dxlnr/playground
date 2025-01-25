import time
import numpy as np

from msgq import VisionIpcServer, VisionType


if __name__ == '__main__':
    nbuff, w, h = 1, 480, 480
    server = VisionIpcServer("lived")
    server.create_buffers(VisionType.VISION_SENSOR_MONO8, nbuff, w, h)
    server.start_listener()

    fid = 0
    try:
        while True:
            buf = np.random.poisson(15_000, size=w * h).astype(np.uint8)
            server.send(VisionType.VISION_SENSOR_MONO8, buf.tobytes(), frame_id=fid)
            fid += 1
            print(f"sent : {fid} | buf[10] : {buf[10]}")
            time.sleep(.1)
    except KeyboardInterrupt:
        pass
