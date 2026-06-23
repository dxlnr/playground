#!/usr/bin/env python

import sys
from pathlib import Path
import unittest
import subprocess
import numpy as np
import time
import threading

sys.path.insert(0, str(Path(__file__).parent.parent))

from qdmq import ShmmReader

class TestShmmReader(unittest.TestCase):
    def test_shmm_reader_double_free(self):
        r = ShmmReader()
        r.close()
        # noop assertion
        self.assertTrue(True)

    def test_to_numpy_dtype_from_writer(self):
        # Path to the C++ writer binary
        writer_path = Path(__file__).parent / "writer_t"
        self.assertTrue(
            writer_path.exists(),
            f"Writer binary not found at `{writer_path}`. Test failed.",
        )
        # reader thread results
        results = {}

        def reader_thread():
            r = ShmmReader()
            err = r.poll_open("vbuf", 0, 5_000)  # 5s timeout
            results["poll_err"] = err
            if err != 0:
                results["error"] = f"poll_open failed with error {err}"
                return
            r.block_until_done()
            results["success"] = r.get_success_value()
            if not results["success"]:
                results["error"] = "Writer did not succeed"
                return
            arr = r.to_numpy()
            results["dtype"] = arr.dtype
            results["shape"] = arr.shape
            results["values_correct"] = np.all(arr == 256)
            r.close()

        # start reader in thread
        t = threading.Thread(target=reader_thread)
        t.start()
        # delay a bit
        time.sleep(0.25)
        # Start writer in subprocess
        proc = subprocess.Popen([str(writer_path)], cwd=writer_path.parent)
        # Wait for thread to finish
        t.join(timeout=12)
        self.assertFalse(t.is_alive(), "Reader thread did not finish")
        # check results
        if "error" in results:
            self.fail(results["error"])
        self.assertEqual(results["poll_err"], 0)
        self.assertTrue(results["success"])
        self.assertEqual(
            results["dtype"], np.uint32, f"Expected uint32, got {results['dtype']}"
        )
        self.assertEqual(results["shape"], (2, 401, 1, 96, 96))
        self.assertTrue(results["values_correct"])
        # Wait for writer to finish
        proc.wait(timeout=5)
        self.assertEqual(proc.returncode, 0, "Writer process failed")


if __name__ == "__main__":
    unittest.main()
