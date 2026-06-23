import sys
import os
from pathlib import Path
import numpy as np

sys.path.append(os.path.dirname(Path(os.path.abspath(__file__)).parent.parent))

from qdmq import ShmmReader


if __name__ == '__main__':
    r = ShmmReader()
    err = r.poll_open("vbuf", 1234, 20_000)  # timeout 20s
    if err != 0:
        print("(error) poll_open failed: ", err)
        del r
        exit(1)
    else:
        print("Connection to data buffer established successfully")
    r.block_until_done()
    print("success?", r.get_success_value())

    arr = r.to_numpy()
    print(arr.shape)
    print("all correct?", np.all(arr == 1280))
    # This is a "double" free and should be possible
    # write a test for that
    r.close()
