"""Create numpy array"""
import os
import argparse
import numpy as np


if __name__ == "__main__":
    PARSER = argparse.ArgumentParser(description="GEMM")
    PARSER.add_argument("--W", type=int, default=64, help="Width")
    PARSER.add_argument("--H", type=int, default=64, help="Height")
    PARSER.add_argument("--F", type=int, default=401, help="Dynamic steps")

    args = PARSER.parse_args()
    H = args.H
    W = args.W
    F = args.F
    # Initialize two random matrices
    A = np.random.rand(2, F, H, W).astype(np.uint32)

    if not os.path.exists("mat"):
        os.makedirs("mat")
    with open("mat/matA", "wb") as f:
        f.write(A.data)
