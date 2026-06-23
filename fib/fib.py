import time
from numba import njit
import functools


@njit(cache=True)
def fib(n):
    if n <= 1:
        return n
    else:
        return fib(n - 1) + fib(n - 2)

# @functools.lru_cache
@njit(cache=True)
def fibc(n):
    if n <= 1:
        return n
    else:
        return fibc(n - 1) + fibc(n - 2)


st = time.time_ns()
n = fib(25)
et = time.time_ns()
print(f"(naive) {n} | rt: {et - st} ns")
st = time.time_ns()
n = fibc(25)
et = time.time_ns()
print(f"(meta)  {n} | rt: {et - st} ns")
