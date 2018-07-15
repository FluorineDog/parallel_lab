#include <thread>
#include <vector>
#include "../common/opencv-bench.h"

__constant__ uint8_t ck[100];

struct Config {
    int rows;
    int cols;
    int kernel_rows;
    int kernel_cols;
};

void erode_cuda(Mat& src, Mat& dst, Mat& kernel) {
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

void erode_cuda(Mat& src, Mat& dst, Mat& kernel) {
  texture<uint8_t, cudatTextureType2D> tex8u; 
  cudaCreateTextureObject()
  cudaCreateTextureObject()
}


int main() {
  EXEC_CV(erode_cuda);
  return 0;
}
