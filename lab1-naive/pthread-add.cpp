#include <thread>
#include <vector>
#include "common.h"
void pthread_add(size_t size, double A[], double B[], double C[]) {
  std::vector<std::thread> threads;
  int thread_num = std::thread::hardware_concurrency();
  for (int t_id = 0; t_id < thread_num; ++t_id) {
    size_t beg = (t_id * size) / thread_num;
    size_t end = (t_id + 1) * size / thread_num;
    threads.emplace_back(
        [](size_t beg, size_t end, double A[], double B[], double C[]) {
          for (size_t i = beg; i < end; ++i) {
            C[i] = B[i] + A[i];
          }
        },
        beg, end, A, B, C);
  }
  for (auto& th : threads) {
    th.join();
  }
}

int main() {
  EXEC(pthread_add);
  return 0;
}
