#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

uint64_t get_time()
{
  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);
  return (uint64_t)start.tv_sec*1e9 + (uint64_t)start.tv_nsec;
}

constexpr int fib(int n)
{
  if (n <= 1) return n;
  return fib(n-1) + fib(n-2);
}

constexpr int fibc(int n)
{
  if (n <= 1) return n;
  return fibc(n-1) + fibc(n-2);
}

int main(void)
{
  uint64_t      st, et, dur;
  constexpr int n = 25;
  /* native */
  st  = get_time();
  int r0 = fib(n);
  et  = get_time();
  dur = (et - st);
  printf("(naive)     %d | rt: %ld ns\n", r0, dur);

  st  = get_time();
  constexpr int r1 = fib(n);
  et  = get_time();
  dur = (et - st);
  printf("(constexpr) %d | rt: %ld ns\n", r1, dur);
  return 0;
}
