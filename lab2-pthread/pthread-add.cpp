#include <thread>
#include <vector>
#include "../common/opencv-bench.h"

void erode_pthread(Mat& src, Mat& dst, Mat& kernel) {
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
          if (kernel.data[ki * base_col + kj]) {
            pixel = std::min(pixel, anchor[ki * src.cols + kj]);
          }
        }
      }
      dst.data[base_row * src.cols + base_col] = pixel;
    }
  }
}


int main() {
  EXEC_CV(erode_pthread);
  return 0;
}
