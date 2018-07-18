#include <thread>
#include <vector>
#include "../common/opencv-bench.h"

void erode_pthread_workload(Mat& src, Mat& dst, Mat& kernel, int beg_r,
                            int end_r) {
  for (int base_row = beg_r; base_row < end_r; ++base_row) {
    for (int base_col = 0; base_col < src.cols; ++base_col) {
      int len_row = std::min(kernel.rows, src.rows - base_row);
      int len_col = std::min(kernel.cols, src.cols - base_col);
      uint8_t pixel = 255;
      auto anchor = src.data + base_row * src.cols + base_col;
#pragma unroll(10)
      for (int ki = 0; ki < len_row; ++ki) {
#pragma unroll(10)
        for (int kj = 0; kj < len_col; ++kj) {
          if (kernel.data[ki * base_col + kj]) {
            pixel = std::min(pixel, anchor[ki * src.cols + kj]);
          }
        }
      }
      dst.data[base_row * src.cols + base_col] = pixel;
    }
  }
}

void erode_pthread(Mat& src, Mat& dst, Mat& kernel) {
  std::vector<std::thread> threads;
  int thread_num = std::thread::hardware_concurrency();
  // int thread_num = 4;
  // split by rows
  int size = src.rows;
  
  for (int t_id = 0; t_id < thread_num; ++t_id) {
    size_t beg = (t_id * size) / thread_num;
    size_t end = (t_id + 1) * size / thread_num;
    if (t_id != thread_num - 1) {
      threads.emplace_back(
          [](Mat* src, Mat* dst, Mat* kernel, size_t beg, size_t end) { erode_pthread_workload(*src, *dst, *kernel, beg, end); }, 
            &src, &dst, &kernel, beg, end
          );
    } else {
      // the last thread is the main thread itself
      erode_pthread_workload(src, dst, kernel, beg, end);
    }
  }
  for (auto& th : threads) {
    th.join();
  }
}

void erode_ref(Mat& src, Mat& dst, Mat& kernel) { erode(src, dst, kernel); }

int main(int argc, char* argv[]) {
  EXEC_CV(erode_pthread);
  return 0;
}
