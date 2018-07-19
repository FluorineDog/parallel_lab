#include <thread>
#include <vector>
#include "../common/opencv-bench.h"

void erode_openmp(Mat& src, Mat& dst, Mat& kernel) {
  #pragma omp parallel for
  for (int base_row = 0; base_row < src.rows; ++base_row) {
    for (int base_col = 0; base_col < src.cols; ++base_col) {
      int len_row = std::min(kernel.rows, src.rows - base_row);
      int len_col = std::min(kernel.cols, src.cols - base_col);
      uint8_t pixel = 255;
      auto anchor = src.data + base_row * src.cols + base_col;
#pragma unroll(10)
      for (int ki = 0; ki < len_row; ++ki) {
#pragma unroll(10)
        for (int kj = 0; kj < len_col; ++kj) {
          if (kernel.data[ki * kernel.cols + kj]) {
            pixel = std::min(pixel, anchor[ki * src.cols + kj]);
          }
        }
      }
      dst.data[base_row * src.cols + base_col] = pixel;
    }
  }
}


void erode_ref(Mat& src, Mat& dst, Mat& kernel) {
  erode(src, dst, kernel);
}


int main(int argc, char* argv[]) {
  EXEC_CV(erode_openmp);
  return 0;
}
