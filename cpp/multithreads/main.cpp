#include <iostream>
#include <omp.h>

int main() {
  #pragma omp parallel for
  for (int i = 0; i < 100; ++i) {
      std::cout << "Thread " << omp_get_thread_num() << " handling iteration " << i << std::endl;
  }

  return 0;
}
